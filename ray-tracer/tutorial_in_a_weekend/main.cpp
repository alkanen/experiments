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

// #include "tqdm.hpp"

#include "rtweekend.hpp"
#include "color.hpp"
#include "hittable_list.hpp"
#include "sphere.hpp"
#include "camera.hpp"
#include "material.hpp"

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

int main(void)
{
  // Image geometry
  const auto aspect_ratio = 16.0 / 9.0;
  const int width = 1920;
  const int height = static_cast<int>(width / aspect_ratio);
  const int samples_per_pixel = 10000;
  const int max_depth = 25;

  // World
  HittableList world;

  auto material_ground = make_shared<Lambertian>(Color(0.8, 0.8, 0.0));
  auto material_center = make_shared<Lambertian>(Color(0.1, 0.2, 0.5));
  auto material_left   = make_shared<Dielectric>(1.5);
  auto material_inner  = make_shared<Dielectric>(2.5);
  auto material_right  = make_shared<Metal>(Color(0.8, 0.6, 0.2), 0.0);

  world.add(make_shared<Sphere>(Point3( 0.0, -100.5, -1.0), 100.0, material_ground));
  world.add(make_shared<Sphere>(Point3( 0.0,    0.0, -1.0),   0.5, material_center));
  world.add(make_shared<Sphere>(Point3(-1.0,    0.0, -1.0),   0.5, material_left));
  world.add(make_shared<Sphere>(Point3(-1.0,    0.0, -1.0), -0.45, material_left));
  //world.add(make_shared<Sphere>(Point3(-1.0,    0.0, -1.0),  0.45, material_inner));
  //world.add(make_shared<Sphere>(Point3(-1.0,    0.0, -1.0), -0.40, material_inner));
  world.add(make_shared<Sphere>(Point3( 1.0,    0.0, -1.0),   0.5, material_right));

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
  std::mutex mutex;
  for_each(
    std::execution::par_unseq,
    scanlines.begin(),
    scanlines.end(),
    [
     &data,
     &cam,
     &world,
     &max_depth,
     &line_count,
     &mutex
    ] (auto &&j) {
      for (int i = 0; i < width; ++i) {
        Color pixel_color(0, 0, 0);
        for(int s = 0; s < samples_per_pixel; s++) {
          auto u = (i + random_double()) / (width - 1);
          auto v = (j + random_double()) / (height - 1);

          Ray r = cam.get_ray(u, v);
          pixel_color += ray_color(r, world, max_depth);
        }

        data[(height - j - 1) * width * 3 + i * 3 + 0] = pixel_color.x();
        data[(height - j - 1) * width * 3 + i * 3 + 1] = pixel_color.y();
        data[(height - j - 1) * width * 3 + i * 3 + 2] = pixel_color.z();
      }

      {
        const std::lock_guard<std::mutex> lock(mutex);
        ++line_count;
        std::cerr << "\rScanline " << line_count << "/" << height << std::flush;
      }
    }
  );
  std::cerr << "\nRender complete." << std::endl;

  // Dump PPM file
  std::cout << "P3\n" << width << ' ' << height << "\n255\n";
  for(auto raw : data) {
    auto col = static_cast<int>(256 * clamp(sqrt(raw / samples_per_pixel), 0.0, 0.999));
    std::cout << col << " ";
  }

  std::cerr << "\nDone." << std::endl;

  return 0;
}
