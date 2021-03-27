// Based on this tutorial
// https://raytracing.github.io/books/RayTracingInOneWeekend.html

#include <cstdlib>
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

#define MAX_COLOR 200

using json = nlohmann::json;

using DataType = std::vector<Color>;
using CountType = std::vector<int64_t>;

int sample_pixel(
  DataType &data1,
  DataType &data2,
  CountType &counter,
  int i,
  int j,
  RenderParams &render_params,
  RenderSegment &render_segment,
  Camera &cam,
  World &world
)
{
  auto pos = ((int64_t)render_params.height - j - 1) * render_params.width + i;
  int count = 0;
  double grid_step = 1.0 / render_params.sample_grid_size;

  for(int y = 0; y < render_params.sample_grid_size; y++ ) {
    for(int x = 0; x < render_params.sample_grid_size; x++ ) {
      auto x_offs = x + random_double() * grid_step;
      auto y_offs = y + random_double() * grid_step;
      auto u = (i + x_offs) / ((double)render_params.width - 1);
      auto v = (j + y_offs) / ((double)render_params.height - 1);
      Color tmpc1 = ray_color(
                      cam.get_ray(u, v),
                      world, render_params.max_depth
                    );

      x_offs = x + random_double() * grid_step;
      y_offs = y + random_double() * grid_step;
      u = (i + x_offs) / ((double)render_params.width - 1);
      v = (j + y_offs) / ((double)render_params.height - 1);
      Color tmpc2 = ray_color(
                      cam.get_ray(u, v),
                      world, render_params.max_depth
                    );

      if(
       is_nan(tmpc1 + tmpc2)
       || tmpc1.length() > MAX_COLOR
       || tmpc2.length() > MAX_COLOR
      )
        continue;

#ifdef SAMPLE_CLAMP
      data1[pos] += clamp_color(tmpc1, SAMPLE_CLAMP);
      data2[pos] += clamp_color(tmpc2, SAMPLE_CLAMP);
#else
      data1[pos] += tmpc1;
      data2[pos] += tmpc2;
#endif
      counter[pos] += 2;
      count += 2;
    }
  }
  return count;
}

int main(int argc, char *argv[])
{
  char *filename;
  if(argc >= 2) {
    filename = argv[1];
  } else {
    filename = const_cast<char*>("test.png");
  }

  // Render parameters
  json render_conf;
  try {
    std::ifstream render_file("../render.json", std::ifstream::in);
    render_file >> render_conf;
  } catch(nlohmann::detail::parse_error &e) {
    std::cout << "No render file found (" << e.what() << ")" << std::endl;
    return -1;
  }
  RenderParams render_params(render_conf);

  // Camera
  json camera_conf;
  try {
    std::ifstream camera_file("../camera.json", std::ifstream::in);
    camera_file >> camera_conf;
  } catch(nlohmann::detail::parse_error &e) {
    std::cout << "No camera file found (" << e.what() << ")" << std::endl;
    return -1;
  }
  Camera cam(camera_conf, render_params.aspect_ratio);

  // World
  json world_conf;
  try {
    std::ifstream world_file("../world.json", std::ifstream::in);
    world_file >> world_conf;
  } catch(nlohmann::detail::parse_error &e) {
    std::cerr << "No world file found (" << e.what() << ")" << std::endl;
    return -1;
  }
  World world(world_conf);

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
  for(int i = 0; i < render_params.height; ++i) {
    segments.push_back(
      RenderSegment(
        0, i, render_params.width-1, i
      )
    );
  }

  // Render
  std::cerr << "Begin" << std::endl;
  int64_t line_count = 0;
  int64_t sample_count = 0;
  int64_t samples_reached_max = 0;
  int64_t next_save = render_params.height / 5;
  std::mutex mutex;
  for_each(
    std::execution::par_unseq,
    segments.begin(),
    segments.end(),
    [
      &render_params,
      &world,
      &data,
      &data1,
      &data2,
      &counter,
      &cam,
      &line_count,
      &sample_count,
      &next_save,
      &mutex,
      &samples_reached_max,
      &filename
    ] (auto &&segment) {
      for (int j = segment.y1; j <= segment.y2; ++j ) {
        int64_t line_sample_count = 0;
        int64_t local_reached_max = 0;
        for (int i = segment.x1; i <= segment.x2; ++i) {
          // Pre-emptively increase max counter
          local_reached_max++;
          auto pos = ((int64_t)render_params.height - j - 1) * render_params.width + i;

          for(int s = 0; s < render_params.max_samples_per_pixel; ) {
            int num_samples = sample_pixel(
              data1, data2,
              counter,
              i, j,
              render_params, segment,
              cam, world
            );

            if( s > render_params.min_samples_per_pixel ) {
              // Check if the variance between the two sample buffers are good enough
              auto c1 = data1[pos];
              auto c2 = data2[pos];
              auto dist = fabs((c1.x() - c2.x()) + (c1.y() - c2.y()) + (c1.z() - c2.z()));
              auto total = (c1.x() + c2.x()) + (c1.y() + c2.y()) + (c1.z() + c2.z());
              if(dist / total < render_params.pincer_limit) {
                // Restore max counter
                local_reached_max--;
                break;
              }
            }

            s += num_samples;
          }

          line_sample_count += counter[pos];
        }

        // Critical section
        {
          const std::lock_guard<std::mutex> lock(mutex);
          ++line_count;
          sample_count += line_sample_count;
          std::cerr << "\rScanline " << line_count << "/" << render_params.height << std::flush;
          samples_reached_max += local_reached_max;
          std::cerr << "\rScanline "
                    << line_count
                    << "/"
                    << render_params.height
                    << " "
                    << samples_reached_max
                    << " ceilings hit and "
                    << ((double)sample_count / (render_params.width * line_count))
                    << " samples per pixel"
                    << std::flush;
          if (line_count > next_save) {
            for(size_t i = 0; i < data.size(); i++) {
              if(counter[i])
                data[i] = (data1[i] + data2[i]) / counter[i];
              else
                data[i] = Color();
            }
            save_png(data, render_params.width, render_params.height, filename);
            next_save += render_params.height / 5;
          }
        }
      }
    }
  );
  std::cerr << "\nRender complete." << std::endl;
  std::cerr
    << "Average "
    << ((double)sample_count / ((double)render_params.width * render_params.height))
    << " samples per pixel"
    << std::endl;

  // Dump image
  std::cerr << "Saving image to '" << filename << "'" << std::endl;
  for(size_t i = 0; i < data.size(); i++) {
    if(counter[i])
      data[i] = (data1[i] + data2[i]) / counter[i];
    else
      data[i] = Color();
  }
  save_png(data, render_params.width, render_params.height, filename);

  std::cerr << "\nDone." << std::endl;

  return 0;
}
