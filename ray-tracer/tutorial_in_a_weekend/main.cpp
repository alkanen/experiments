// Based on this tutorial
// https://raytracing.github.io/books/RayTracingInOneWeekend.html

#include <cstdlib>
// #include "tqdm.hpp"

#include "rtweekend.hpp"
#include "color.hpp"
#include "hittable_list.hpp"
#include "sphere.hpp"

#include <iostream>

Color ray_color(const Ray &r, const Hittable &world)
{
  HitRecord rec;
  if(world.hit(r, 0, infinity, rec)) {
    return 0.5 * (rec.normal + Color(1, 1, 1));
  }
  Vec3 unit_direction = unit_vector(r.direction());
  auto t = 0.5 * (unit_direction.y() + 1.0);
  return (1.0 - t) * Color(1.0, 1.0, 1.0) + t * Color(0.5, 0.7, 1.0);
}

int main(void)
{
  // Image geometry
  const auto aspect_ratio = 16.0 / 9.0;
  const int width = 800;
  const int height = static_cast<int>(width / aspect_ratio);

  // World
  HittableList world;
  world.add(make_shared<Sphere>(Point3(0, 0, -1), 0.5));
  world.add(make_shared<Sphere>(Point3(0, -100.5, -1), 100));

  // Camera settings
  auto viewport_height = 2.0;
  auto viewport_width = aspect_ratio * viewport_height;
  auto focal_length = 1.0;

  auto origin = Point3(0, 0, 0);
  auto horizontal = Vec3(viewport_width, 0, 0);
  auto vertical = Vec3(0, viewport_height, 0);
  auto lower_left_corner = origin - horizontal/2 - vertical/2 - Vec3(0, 0, focal_length);

  // Render
  std::cerr << "Begin" << std::endl;
  std::cout << "P3\n" << width << ' ' << height << "\n255\n";
  //for(auto inv_j : tqdm::range(height)) {
  for(int j = height - 1; j >= 0; j--) {
    std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
    // auto j = height - 1 - inv_j;
    for (int i = 0; i < width; ++i) {
      auto u = double(i) / (width - 1);
      auto v = double(j) / (height - 1);

      //Ray r(origin, lower_left_corner + u * horizontal + v * vertical - origin);
      Ray r(origin, lower_left_corner + u * horizontal + v * vertical);

      Color pixel_color = ray_color(r, world);

      write_color(std::cout, pixel_color);
    }
  }
  std::cerr << "\nDone." << std::endl;

  return 0;
}
