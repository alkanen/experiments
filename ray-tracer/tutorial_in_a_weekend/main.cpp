// Based on this tutorial
// https://raytracing.github.io/books/RayTracingInOneWeekend.html

#include <cstdlib>
// #include "tqdm.hpp"

#include "rtweekend.hpp"
#include "color.hpp"
#include "hittable_list.hpp"
#include "sphere.hpp"
#include "camera.hpp"
#include "material.hpp"

#include <iostream>

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
  const int width = 400;
  const int height = static_cast<int>(width / aspect_ratio);
  const int samples_per_pixel = 100;
  const int max_depth = 50;

  // World
  HittableList world;

  auto material_ground = make_shared<Lambertian>(Color(0.8, 0.8, 0.0));
  auto material_center = make_shared<Lambertian>(Color(0.7, 0.3, 0.3));
  auto material_left   = make_shared<Metal>(Color(0.8, 0.8, 0.8), 0.3);
  auto material_right  = make_shared<Metal>(Color(0.8, 0.6, 0.2), 1.0);

  world.add(make_shared<Sphere>(Point3( 0.0, -100.5, -1.0), 100.0, material_ground));
  world.add(make_shared<Sphere>(Point3( 0.0,    0.0, -1.0),   0.5, material_center));
  world.add(make_shared<Sphere>(Point3(-1.0,    0.0, -1.0),   0.5, material_left));
  world.add(make_shared<Sphere>(Point3( 1.0,    0.0, -1.0),   0.5, material_right));

  // Camera
  Camera cam;

  // Render
  std::cerr << "Begin" << std::endl;
  std::cout << "P3\n" << width << ' ' << height << "\n255\n";
  //for(auto inv_j : tqdm::range(height)) {
  for(int j = height - 1; j >= 0; j--) {
    std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
    // auto j = height - 1 - inv_j;
    for (int i = 0; i < width; ++i) {
      Color pixel_color(0, 0, 0);
      for(int s = 0; s < samples_per_pixel; s++) {
        auto u = (i + random_double()) / (width - 1);
        auto v = (j + random_double()) / (height - 1);

        Ray r = cam.get_ray(u, v);
        pixel_color += ray_color(r, world, max_depth);
      }

      write_color(std::cout, pixel_color, samples_per_pixel);
    }
  }
  std::cerr << "\nDone." << std::endl;

  return 0;
}
