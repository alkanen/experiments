// Based on this tutorial
// https://raytracing.github.io/books/RayTracingInOneWeekend.html

#include <cstdlib>
#include <chrono>
#include <algorithm>
#include <execution>
#include <vector>
#include <random>
#include <iostream>
#include <fstream>
#include <mutex>

#include <nlohmann/json.hpp>

#include "camera.hpp"
#include "render_params.hpp"
#include "config.hpp"
#include "png.hpp"

#include "rtweekend.hpp"
#include "ray.hpp"
#include "vec3.hpp"
#include "color.hpp"
#include "hittable_list.hpp"
#include "pdf.hpp"
#include "world.hpp"
#include "ray_color.hpp"

#define SAMPLE_CLAMP 100
#undef SAMPLE_CLAMP

#define MAX_COLOR 2000000

using json = nlohmann::json;

using DataType = std::vector<Color>;
using CountType = std::vector<int64_t>;

int sample_pixel(
  DataType &data1,
  DataType &data2,
  CountType &counter,
  int pos,
  int i,
  int j,
  RenderParams &render_params,
  RenderSegment &render_segment,
  Camera &cam,
  World &world,
  int &skipped_nan,
  int &skipped_high
)
{
  // auto pos = ((int64_t)render_params.height - j - 1) * render_params.width + i;
  int count = 0;
  double grid_step = 1.0 / render_params.sample_grid_size;

  skipped_nan = 0;
  skipped_high = 0;

  for(int y = 0; y < render_params.sample_grid_size; y++ ) {
    for(int x = 0; x < render_params.sample_grid_size; x++ ) {
      auto x_offs = (x + random_double()) / render_params.sample_grid_size;
      auto y_offs = (y + random_double()) / render_params.sample_grid_size;
      auto u = (i + x_offs) / (double)render_params.width;
      auto v = (j + y_offs) / (double)render_params.height;
      Color tmpc1 = ray_color(
                      cam.get_ray(u, v),
                      world, render_params.max_depth
                    );

      if(is_nan(tmpc1)) {
        skipped_nan += 1;
        continue;
      }
      if(tmpc1.length() > MAX_COLOR) {
        skipped_high += 1;
        continue;
      }

      x_offs = (x + random_double()) / render_params.sample_grid_size;
      y_offs = (y + random_double()) / render_params.sample_grid_size;
      u = (i + x_offs) / (double)render_params.width;
      v = (j + y_offs) / (double)render_params.height;
      Color tmpc2 = ray_color(
                      cam.get_ray(u, v),
                      world, render_params.max_depth
                    );

      if(is_nan(tmpc2)) {
        skipped_nan += 1;
        continue;
      }
      if(tmpc2.length() > MAX_COLOR) {
        skipped_high += 1;
        continue;
      }

#ifdef SAMPLE_CLAMP
      data1[pos] += clamp_color(tmpc1, SAMPLE_CLAMP);
      data2[pos] += clamp_color(tmpc2, SAMPLE_CLAMP);
#else
      data1[pos] += tmpc1;
      data2[pos] += tmpc2;
#endif
      count += 2;
    }
  }

  counter[pos] += count;
  return count;
}

int main(int argc, char *argv[])
{
  std::string config_filename;
  if(argc >= 2) {
    config_filename = argv[1];
  } else {
    config_filename = "../config.json";
  }

  // Config parameters
  json config_conf;
  try {
    std::ifstream config_file(config_filename, std::ifstream::in);
    config_file >> config_conf;
  } catch(nlohmann::detail::parse_error &e) {
    std::cerr << "No config file found (" << e.what() << ")" << std::endl;
    return -1;
  }
  Config config(config_conf);

  // Render parameters
  json render_conf;
  try {
    std::ifstream render_file(config.render_config, std::ifstream::in);
    render_file >> render_conf;
  } catch(nlohmann::detail::parse_error &e) {
    std::cout << "No render file found (" << e.what() << ")" << std::endl;
    return -1;
  }
  RenderParams render_params(render_conf);

  // Camera
  json camera_conf;
  try {
    std::ifstream camera_file(config.camera_config, std::ifstream::in);
    camera_file >> camera_conf;
  } catch(nlohmann::detail::parse_error &e) {
    std::cout << "No camera file found (" << e.what() << ")" << std::endl;
    return -1;
  }
  Camera cam(camera_conf, render_params.aspect_ratio);

  // World
  json world_conf;
  try {
    std::ifstream world_file(config.world_config, std::ifstream::in);
    world_file >> world_conf;
  } catch(nlohmann::detail::parse_error &e) {
    std::cerr << "No world file found (" << e.what() << ")" << std::endl;
    return -1;
  }
  World world(world_conf, cam);

  // Image data
  // End result
  DataType data(render_params.height * render_params.width);
  // First comparison buffer
  DataType data1(data.size());
  // Second comparison buffer
  DataType data2(data.size());
  // Sample counter
  CountType counter(data.size());
  std::vector<RenderSegment> segments;
  int segment_height = config.segment_height;
  int segment_width = config.segment_width;
  for (int y = 0; y < render_params.height; y += segment_height) {
    for (int x = 0; x < render_params.width; x += segment_width) {
      segments.push_back(
        RenderSegment(
          x, y,
          std::min(x + segment_width - 1, render_params.width - 1),
          std::min(y + segment_height - 1, render_params.height - 1)
        )
      );
    }
  }
  size_t num_segments = segments.size();
  // Render
  std::cerr << "Begin sampling " << num_segments << " segments" << std::endl;
  int64_t segment_count = 0;
  int64_t sample_count = 0;
  int64_t samples_reached_max = 0;
  int64_t pixel_count = 0;
  int save_divider = config.saves_per_render;
  int64_t next_save = 0;
  if(save_divider)
    next_save = num_segments / save_divider;
  std::mutex mutex;
  auto t1 = std::chrono::high_resolution_clock::now();
  for_each(
    std::execution::par_unseq,
    segments.begin(),
    segments.end(),
    [
      &render_params,
      &config,
      &num_segments,
      &world,
      &data,
      &data1,
      &data2,
      &counter,
      &cam,
      &segment_count,
      &sample_count,
      &pixel_count,
      &next_save,
      &save_divider,
      &mutex,
      &samples_reached_max
    ] (auto &&segment) {
      int64_t segment_sample_count = 0;
      int64_t local_reached_max = 0;
      for (int j = segment.y1; j <= segment.y2; ++j) {
        for (int i = segment.x1; i <= segment.x2; ++i) {
          // Pre-emptively increase max counter
          local_reached_max++;
          int pos = ((int)render_params.height - j - 1) * render_params.width + i;

          int pixel_skipped_nan = 0;
          int pixel_skipped_high = 0;

          for (int s = 0; s < render_params.max_samples_per_pixel; ) {
            int skipped_nan = 0;
            int skipped_high = 0;
            int num_samples = sample_pixel(
              data1, data2,
              counter,
              pos,
              i, j,
              render_params, segment,
              cam, world,
              skipped_nan,
              skipped_high
            );
            s += num_samples;

            if (render_params.min_samples_per_pixel < s) {
              // Check if the variance between the two sample buffers are good enough
              const auto& c1 = data1[pos];
              const auto& c2 = data2[pos];
              auto dist = fabs((c1.x() - c2.x()) + fabs(c1.y() - c2.y()) + fabs(c1.z() - c2.z()));
              auto total = (c1.x() + c2.x()) + (c1.y() + c2.y()) + (c1.z() + c2.z());
              if (dist / total < render_params.pincer_limit) {
                // Restore max counter
                local_reached_max--;
                break;
              }
            }

            pixel_skipped_nan += skipped_nan;
            pixel_skipped_high += skipped_high;

            if(pixel_skipped_nan > render_params.max_samples_per_pixel) {
              std::cerr << "Had to bail on (" << j << ", " << i << ") due to too many NaN pixels" << std::endl;
              break;
            }
            if(pixel_skipped_high > render_params.max_samples_per_pixel) {
              std::cerr << "Had to bail on (" << j << ", " << i << ") due to too many high pixels" << std::endl;
              break;
            }
          }

          segment_sample_count += counter[pos];
        }
      }

      // Critical section
      {
        const std::lock_guard<std::mutex> lock(mutex);
        ++segment_count;
        sample_count += segment_sample_count;
	    pixel_count += segment.pixels;
        std::cerr << "       \rSegment " << segment_count << "/" << num_segments << std::flush;
        samples_reached_max += local_reached_max;
        std::cerr << "       \rSegment "
                  << segment_count
                  << "/"
                  << num_segments
                  << " "
                  << samples_reached_max
                  << " ceilings hit and "
                  << (sample_count / (double)(pixel_count))
                  << " samples per pixel"
                  << std::flush;
        if (next_save && !config.output_image.empty() && segment_count > next_save) {
          for(size_t i = 0; i < data.size(); i++) {
            if(counter[i])
              data[i] = (data1[i] + data2[i]) / static_cast<double>(counter[i]);
            else
              data[i] = Color();
          }
          save_png(data, render_params.width, render_params.height, config.output_image);
          next_save += num_segments / save_divider;
        }
      }
    }
  );
  auto t2 = std::chrono::high_resolution_clock::now();
  std::cerr << "\nRender complete." << std::endl;
  std::cerr
    << "Average "
    << ((double)sample_count / ((double)render_params.width * render_params.height))
    << " samples per pixel"
    << std::endl;

  int ms = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count());
  int minutes = int(ms / 60000);
  ms -= minutes * 60000;
  int seconds = int(ms / 1000);
  ms -= seconds * 1000;
  std::cerr << "Total time: " << minutes << ":" << seconds << "." << ms << std::endl;

  // Dump image
  if(!config.output_image.empty()) {
    std::cerr << "Saving image to '" << config.output_image << "'" << std::endl;
    for(size_t i = 0; i < data.size(); i++) {
      if(counter[i])
	data[i] = (data1[i] + data2[i]) / static_cast<double>(counter[i]);
      else
	data[i] = Color();
    }
    save_png(data, render_params.width, render_params.height, config.output_image);
  }

  std::cerr << "\nDone." << std::endl;

  return 0;
}
