// Based on this tutorial
// https://raytracing.github.io/books/RayTracingInOneWeekend.html

#include <cstdlib>
#include <algorithm>
#include <execution>
#include <array>
#include <vector>
#include <random>
#include <iostream>
#include <mutex>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include "rtweekend.hpp"
#include "color.hpp"
#include "hittable_list.hpp"
#include "sphere.hpp"
#include "camera.hpp"
#include "material.hpp"

void save_png(std::vector<double> &data, const int width, const int height, const char *filename)
{
  int y, x;
  uint8_t *pixels;
  const int pitch = width * 3;
  // Change from RGB to BGR because PNG sucks.
  pixels = new uint8_t[data.size()];
  for(y=0; y<height; y++) {
    for(x=0; x<pitch-3; x+=3) {
      pixels[y * pitch + x + 0] = static_cast<int>(256 * clamp(sqrt(data[y * pitch + x + 0]), 0.0, 0.999));;
      pixels[y * pitch + x + 1] = static_cast<int>(256 * clamp(sqrt(data[y * pitch + x + 1]), 0.0, 0.999));;
      pixels[y * pitch + x + 2] = static_cast<int>(256 * clamp(sqrt(data[y * pitch + x + 2]), 0.0, 0.999));;
    }
  }

  stbi_write_png( filename, width, height, STBI_rgb, pixels, width*3 );
  delete[] pixels;
}


Color ray_color(const Ray &r, const Hittable &world, int depth)
{
  HitRecord rec;

  if(depth <= 0)
    return Color(0, 0, 0);

  if(world.hit(r, 0.001, infinity, rec)) {
    Ray scattered;
    Color attenuation;
    if(rec.material->scatter(r, rec, attenuation, scattered))
      return attenuation * ray_color(scattered, world, depth-1);

    return Color(0, 0, 0);
  }
  Vec3 unit_direction = unit_vector(r.direction());
  auto t = 0.5 * (unit_direction.y() + 1.0);
  return (1.0 - t) * Color(1.0, 1.0, 1.0) + t * Color(0.5, 0.7, 1.0);
}

int main(int argc, char *argv[])
{
  char *filename;
  if(argc >= 2) {
    filename = argv[1];
  } else {
    filename = const_cast<char*>("test.png");
  }
  // Image properties
  const auto aspect_ratio = 16.0 / 9.0;
  const int width = 1920*2;
  const int height = static_cast<int>(width / aspect_ratio);
  const int min_samples_per_pixel = 10;
  const int max_samples_per_pixel = 10000000;
  const int max_depth = 25;
  const double pincer_limit = 0.00001;

  // World
  HittableList world;

  auto material_ground = Lambertian(Color(0.8, 0.8, 0.0));
  auto material_center = Lambertian(Color(0.1, 0.2, 0.5));
  auto material_left   = Dielectric(1.5);
  auto material_inner  = Dielectric(2.5);
  auto material_right  = Metal(Color(0.8, 0.6, 0.2), 0.0);

  auto sphere1 = Sphere(Point3( 0.0, -100.5, -1.0), 100.0, &material_ground);
  auto sphere2 = Sphere(Point3( 0.0,    0.0, -1.0),   0.5, &material_center);
  auto sphere3 = Sphere(Point3(-1.0,    0.0, -1.0),   0.5, &material_left);
  auto sphere4 = Sphere(Point3(-1.0,    0.0, -1.0), -0.45, &material_left);
  auto sphere5 = Sphere(Point3( 1.0,    0.0, -1.0),   0.5, &material_right);
  world.add(&sphere1);
  world.add(&sphere2);
  world.add(&sphere3);
  world.add(&sphere4);
  //world.add(make_shared<Sphere>(Point3(-1.0,    0.0, -1.0),  0.45, material_inner));
  //world.add(make_shared<Sphere>(Point3(-1.0,    0.0, -1.0), -0.40, material_inner));
  world.add(&sphere5);

  // Camera
  auto look_from = Point3(-2, 2, 1);
  auto look_at = Point3(0, 0, -1);
  auto vup = Vec3(0, 1, 0);
  auto fov = 90.0;
  auto dist_to_focus = (look_at - look_from).length();
  auto aperture = .20;

  Camera cam(look_from, look_at, vup, fov, aspect_ratio, aperture, dist_to_focus);

  // Image data
  std::vector<double> data(3 * height * width);
  std::vector<int> scanlines;
  for(int i = 0; i < height; ++i) {
    scanlines.push_back(i);
  }

  // Render
  std::cerr << "Begin" << std::endl;
  auto line_count = 0;
  auto sample_count = 0;
  std::mutex mutex;
  for_each(
    std::execution::par_unseq,
    scanlines.begin(),
    scanlines.end(),
    [
      &width,
      &height,
      &data,
      &cam,
      &world,
      &max_depth,
      &pincer_limit,
      &min_samples_per_pixel,
      &max_samples_per_pixel,
      &line_count,
      &sample_count,
      &mutex
    ] (auto &&j) {
      auto line_sample_count = 0;
      for (int i = 0; i < width; ++i) {
        Color c1(0, 0, 0), c2(0, 0, 0);
        auto count = 0;
        for(int s = 0; s < min_samples_per_pixel / 2; s++) {
          auto u = (i + random_double()) / (width - 1);
          auto v = (j + random_double()) / (height - 1);

          // Ray calculation contains some randomness
          c1 += ray_color(cam.get_ray(u, v), world, max_depth);
          c2 += ray_color(cam.get_ray(u, v), world, max_depth);
          count += 2;
        }

        // Adaptive loop
        for(int s = min_samples_per_pixel / 2; s < max_samples_per_pixel / 2; s++) {
          auto u = (i + random_double()) / (width - 1);
          auto v = (j + random_double()) / (height - 1);

          // Ray r = cam.get_ray(u, v);
          // pixel_color += ray_color(r, world, max_depth);
          // Ray calculation contains some randomness
          c1 += ray_color(cam.get_ray(u, v), world, max_depth);
          c2 += ray_color(cam.get_ray(u, v), world, max_depth);
          count += 2;

          //if((c1 - c2).length() / (c1+c2).length() < pincer_limit)
          auto dist = fabs((c1.x() - c2.x()) + (c1.y() - c2.y()) + (c1.z() - c2.z()));
          auto total = (c1.x() + c2.x()) + (c1.y() + c2.y()) + (c1.z() + c2.z());
          if(dist / total < pincer_limit)
            break;
        }

        line_sample_count += count;
        data[(height - j - 1) * width * 3 + i * 3 + 0] = (c1 + c2).x() / count;
        data[(height - j - 1) * width * 3 + i * 3 + 1] = (c1 + c2).y() / count;
        data[(height - j - 1) * width * 3 + i * 3 + 2] = (c1 + c2).z() / count;
      }

      {
        const std::lock_guard<std::mutex> lock(mutex);
        ++line_count;
        sample_count += line_sample_count;
        std::cerr << "\rScanline " << line_count << "/" << height << std::flush;
      }
    }
  );
  std::cerr << "\nRender complete." << std::endl;
  std::cerr << "Average " << ((double)sample_count / (width * height)) << " samples per pixel" << std::endl;

  // Dump image
  std::cerr << "Saving image to '" << filename << "'" << std::endl;
  save_png(data, width, height, filename);

  std::cerr << "\nDone." << std::endl;

  return 0;
}
