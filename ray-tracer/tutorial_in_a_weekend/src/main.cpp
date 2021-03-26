// Based on this tutorial
// https://raytracing.github.io/books/RayTracingInOneWeekend.html

#include <cstdlib>
#include <algorithm>
#include <execution>
#include <array>
#include <vector>
#include <random>
#include <iostream>
#include <fstream>
#include <mutex>

#include <nlohmann/json.hpp>

#include "stb_image.h"
#include "stb_image_write.h"

#include "camera.hpp"
#include "render.hpp"

#include "rtweekend.hpp"
#include "ray.hpp"
#include "vec3.hpp"
#include "color.hpp"
#include "hittable_list.hpp"
#include "pdf.hpp"
#include "world.hpp"

#define SAMPLE_CLAMP 100
#undef SAMPLE_CLAMP

#define MAX_COLOR 200

using json = nlohmann::json;

void save_png(std::vector<double> &data, const int width, const int height, const char *filename)
{
  int y, x;
  uint8_t *pixels;
  const int64_t pitch = width * 3;
  pixels = new uint8_t[data.size()];

  // Change from RGB to BGR because PNG sucks.
  for(y=0; y<height; y++) {
    for(x=0; x < pitch - 3; x += 3) {
      auto r = data[(int64_t)y * pitch + x + 0LL];
      auto g = data[(int64_t)y * pitch + x + 1LL];
      auto b = data[(int64_t)y * pitch + x + 2LL];

      pixels[(int64_t)y * pitch + x + 0LL] = static_cast<int>(256 * clamp(sqrt(r), 0.0, 0.999));
      pixels[(int64_t)y * pitch + x + 1LL] = static_cast<int>(256 * clamp(sqrt(g), 0.0, 0.999));
      pixels[(int64_t)y * pitch + x + 2LL] = static_cast<int>(256 * clamp(sqrt(b), 0.0, 0.999));
    }
  }

  stbi_write_png( filename, width, height, STBI_rgb, pixels, (int)pitch );
  delete[] pixels;
}


Color ray_color(const Ray &r, World &world, int depth)
{
  HitRecord rec;
  if(depth <= 0)
    return Color(0, 0, 0);

  if(!world.objects.hit(r, 0.001, infinity, rec))
    return world.background;

  ScatterRecord srec;
  //std::cerr << "ray_color before rec.material->emitted()" << std::endl;
  Color emitted = rec.material->emitted(r, rec, rec.u, rec.v, rec.p);

  //std::cerr << "ray_color before !rec.material->scatter()" << std::endl;
  if(!rec.material->scatter(r, rec, srec))
    return emitted;

  if(srec.is_specular) {
    //std::cerr << "ray_color before srec.attenuation * ray_color()" << std::endl;
    return srec.attenuation
      * ray_color(srec.specular_ray, world, depth-1);
  }

  Vec3 scatter_direction;
  Ray scattered;
  double pdf;
  if(dynamic_cast<HittableList*>(&world.lights)->size()) {
    //std::cerr << "ray_color before MixturePdf" << std::endl;
    auto p0 = std::make_shared<HittablePdf>(&world.lights, rec.p);
    MixturePdf mixed_pdf = MixturePdf(p0, srec.pdf);
    //std::cerr << "ray_color before mixed_pdf.generate()" << std::endl;
    scatter_direction = mixed_pdf.generate();
    scattered = Ray(rec.p, scatter_direction, r.time());
    //std::cerr << "ray_color before mixed_pdf.value()" << std::endl;
    pdf = mixed_pdf.value(scattered.direction());
  } else {
    //std::cerr << "ray_color before srec.pdf->generat()" << std::endl;
    scatter_direction = srec.pdf->generate();
    scattered = Ray(rec.p, scatter_direction, r.time());
    //std::cerr << "ray_color before srec.pdf->value()" << std::endl;
    pdf = srec.pdf->value(scattered.direction());
  }

  //std::cerr << "ray_color return" << std::endl;
  return emitted
    + srec.attenuation
    * rec.material->scattering_pdf(r, rec, scattered) / pdf
    * ray_color(scattered, world, depth-1);
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
  std::vector<double> data(3LL * render_params.height * render_params.width);
  std::vector<int> scanlines;
  for(int i = 0; i < render_params.height; ++i) {
    scanlines.push_back(i);
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
    scanlines.begin(),
    scanlines.end(),
    [
      &render_params,
      &world,
      &data,
      &cam,
      &line_count,
      &sample_count,
      &next_save,
      &mutex,
      &samples_reached_max,
      &filename
    ] (auto &&j) {
      int64_t line_sample_count = 0;
      int64_t local_reached_max = 0;
      for (int i = 0; i < render_params.width; ++i) {
        // Pre-emptively increase max counter
        local_reached_max++;
        Color c1(0, 0, 0), c2(0, 0, 0);
        int64_t count = 0;
        for(int s = 0; s < render_params.min_samples_per_pixel / 2; s++) {
          auto u = (i + random_double()) / ((double)render_params.width - 1);
          auto v = (j + random_double()) / ((double)render_params.height - 1);

          // Ray calculation contains some randomness
          auto tmpc1 = ray_color(cam.get_ray(u, v), world, render_params.max_depth);
          auto tmpc2 = ray_color(cam.get_ray(u, v), world, render_params.max_depth);

	  if(
	     is_nan(tmpc1) || is_nan(tmpc2)
	     || tmpc1.length() > MAX_COLOR || tmpc2.length() > MAX_COLOR
	     ) {
	    s -= 2;
	    continue;
	  }

#ifdef SAMPLE_CLAMP
          c1 += clamp_color(tmpc1, SAMPLE_CLAMP);
          c2 += clamp_color(tmpc2, SAMPLE_CLAMP);
#else
          c1 += tmpc1;
          c2 += tmpc2;
#endif
          count += 2;
        }

        // Adaptive loop
        for(
            int s = render_params.min_samples_per_pixel / 2;
            s < render_params.max_samples_per_pixel / 2;
            s++
        ) {
          auto u = (i + random_double()) / ((double)render_params.width - 1);
          auto v = (j + random_double()) / ((double)render_params.height - 1);

          auto tmpc1 = ray_color(cam.get_ray(u, v), world, render_params.max_depth);
          auto tmpc2 = ray_color(cam.get_ray(u, v), world, render_params.max_depth);

	  if(
	     is_nan(tmpc1) || is_nan(tmpc2)
	     || tmpc1.length() > MAX_COLOR || tmpc2.length() > MAX_COLOR
             ) {
	    s -= 2;
	    continue;
	  }

#ifdef SAMPLE_CLAMP
          c1 += clamp_color(tmpc1, SAMPLE_CLAMP);
          c2 += clamp_color(tmpc2, SAMPLE_CLAMP);
#else
          c1 += tmpc1;
          c2 += tmpc2;
#endif
          count += 2;

          auto dist = fabs((c1.x() - c2.x()) + (c1.y() - c2.y()) + (c1.z() - c2.z()));
          auto total = (c1.x() + c2.x()) + (c1.y() + c2.y()) + (c1.z() + c2.z());
          if(dist / total < render_params.pincer_limit) {
            // Restore max counter
            local_reached_max--;
            break;
          }
        }

        line_sample_count += count;
        data[((int64_t)render_params.height - j - 1) * render_params.width * 3 + i * 3 + 0] = (c1 + c2).x() / count;
        data[((int64_t)render_params.height - j - 1) * render_params.width * 3 + i * 3 + 1] = (c1 + c2).y() / count;
        data[((int64_t)render_params.height - j - 1) * render_params.width * 3 + i * 3 + 2] = (c1 + c2).z() / count;
      }

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
          save_png(data, render_params.width, render_params.height, filename);
          next_save += render_params.height / 5;
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
  save_png(data, render_params.width, render_params.height, filename);

  std::cerr << "\nDone." << std::endl;

  return 0;
}
