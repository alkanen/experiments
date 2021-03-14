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
#include "bvh.hpp"
#include "box.hpp"
#include "sphere.hpp"
#include "moving_sphere.hpp"
#include "aarect.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "constant_medium.hpp"
#include "pdf.hpp"

#undef SAMPLE_CLAMP

void save_png(std::vector<double> &data, const int width, const int height, const char *filename)
{
  int y, x;
  uint8_t *pixels;
  const int pitch = width * 3;
  // Change from RGB to BGR because PNG sucks.
  pixels = new uint8_t[data.size()];
  for(y=0; y<height; y++) {
    for(x=0; x<pitch-3; x+=3) {
      pixels[y * pitch + x + 0] = static_cast<int>(256 * clamp(sqrt(data[y * pitch + x + 0]), 0.0, 0.999));
      pixels[y * pitch + x + 1] = static_cast<int>(256 * clamp(sqrt(data[y * pitch + x + 1]), 0.0, 0.999));
      pixels[y * pitch + x + 2] = static_cast<int>(256 * clamp(sqrt(data[y * pitch + x + 2]), 0.0, 0.999));
    }
  }

  stbi_write_png( filename, width, height, STBI_rgb, pixels, width*3 );
  delete[] pixels;
}


Color ray_color(const Ray &r, const Color &background, const Hittable &world, Hittable &lights, int depth)
{
  HitRecord rec;

  if(depth <= 0)
    return Color(0, 0, 0);

  if(!world.hit(r, 0.001, infinity, rec))
    return background;

  ScatterRecord srec;
  Color emitted = rec.material->emitted(r, rec, rec.u, rec.v, rec.p);

  if(!rec.material->scatter(r, rec, srec))
    return emitted;

  if(srec.is_specular) {
    return srec.attenuation
      * ray_color(srec.specular_ray, background, world, lights, depth-1);
  }

  auto p0 = HittablePdf(&lights, rec.p);
  MixturePdf mixed_pdf(&p0, srec.pdf);

  Ray scattered = Ray(rec.p, mixed_pdf.generate(), r.time());
  auto pdf = mixed_pdf.value(scattered.direction());

  return emitted
    + srec.attenuation
    * rec.material->scattering_pdf(r, rec, scattered) / pdf
    * ray_color(scattered, background, world, lights, depth-1);
}

HittableList random_scene()
{
  HittableList world;

  auto checker = new CheckerTexture(Color(0.2, 0.3, 0.1), Color(0.9, 0.9, 0.9));
  auto ground_material = new Lambertian(checker);
  auto sphere = new Sphere(Point3(0,-1000,0), 1000, ground_material);
  world.add(sphere);

  for (int a = -11; a < 11; a++) {
    for (int b = -11; b < 11; b++) {
      auto choose_mat = random_double();
      Point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

      if ((center - Point3(4, 0.2, 0)).length() > 0.9) {
        Material *sphere_material;
        Hittable *sphere_tmp;
        if (choose_mat < 0.8) {
          // diffuse
          auto albedo = Color::random() * Color::random();
          sphere_material = new Lambertian(albedo);
          Point3 center2 = center + Vec3(0, random_double(0,.5), 0);
          sphere_tmp = new MovingSphere(
            center,
            center2,
            0.0,
            1.0,
            0.2,
            sphere_material
          );
          world.add(sphere_tmp);
        } else if (choose_mat < 0.95) {
          // metal
          auto albedo = Color::random(0.5, 1);
          auto fuzz = random_double(0, 0.5);
          sphere_material = new Metal(albedo, fuzz);
          sphere_tmp = new Sphere(center, 0.2, sphere_material);
          world.add(sphere_tmp);
        } else {
          // glass
          sphere_material = new Dielectric(1.5);
          sphere_tmp = new Sphere(center, 0.2, sphere_material);
          world.add(sphere_tmp);
        }
      }
    }
  }

  // Middle
  Material *material1 = new Dielectric(1.5);
  Sphere *sphere1 = new Sphere(Point3(0, 1, 0), 1.0, material1);
  world.add(sphere1);

  // Farthest
  Material *material2 = new Lambertian(Color(0.4, 0.2, 0.1));
  Sphere *sphere2 = new Sphere(Point3(-4, 1, 0), 1.0, material2);
  world.add(sphere2);

  // Closest
  Material * material3 = new Metal(Color(0.7, 0.6, 0.5), 0.0);
  Sphere *sphere3 = new Sphere(Point3(4, 1, 0), 1.0, material3);
  world.add(sphere3);

  return world;
}

HittableList two_spheres()
{
  HittableList objects;

  auto checker = new CheckerTexture(Color(0.2, 0.3, 0.1), Color(0.9, 0.9, 0.9));

  objects.add(new Sphere(Point3(0,-10, 0), 10, new Lambertian(checker)));
  objects.add(new Sphere(Point3(0, 10, 0), 10, new Lambertian(checker)));

  return objects;
}

HittableList two_perlin_spheres() {
  HittableList objects;

  auto pertext = new NoiseTexture(4);
  objects.add(new Sphere(Point3(0, -1000, 0), 1000, new Lambertian(pertext)));
  objects.add(new Sphere(Point3(0, 2, 0), 2, new Lambertian(pertext)));

  return objects;
}

HittableList earth()
{
  auto earth_texture = new ImageTexture("earthmap.jpg");
  auto earth_surface = new Lambertian(earth_texture);
  auto globe = new Sphere(Point3(0,0,0), 2, earth_surface);

  return HittableList(globe);
}

HittableList simple_light()
{
  HittableList objects;

  auto pertext = new NoiseTexture(4);
  objects.add(new Sphere(Point3(0, -1000, 0), 1000, new Lambertian(pertext)));
  objects.add(new Sphere(Point3(0, 2, 0), 2, new Lambertian(pertext)));

  auto difflight = new DiffuseLight(Color(4, 4, 4));
  objects.add(new XyRect(3, 5, 1, 3, -2, difflight));

  return objects;
}

typedef struct {
  HittableList objects;
  HittableList lights;
} World;

World cornell_box() {
  World world;

  auto red   = new Lambertian(Color(.65, .05, .05));
  auto white = new Lambertian(Color(.73, .73, .73));
  auto green = new Lambertian(Color(.12, .45, .15));
  auto light = new DiffuseLight(Color(15, 15, 15));

  std::cout << "red:   " << red << std::endl;
  std::cout << "white: " << white << std::endl;
  std::cout << "green: " << green << std::endl;
  std::cout << "light: " << light << std::endl;

  world.objects.add(new YzRect(0, 555, 0, 555, 555, green));
  world.objects.add(new YzRect(0, 555, 0, 555, 0, red));
  world.lights.add(new XzRect(213, 343, 227, 332, 554, new Material()));
  world.lights.add(new Sphere(Point3(190, 90, 190), 90, new Material()));
  world.objects.add(new FlipFace(new XzRect(213, 343, 227, 332, 554, light)));
  world.objects.add(new XzRect(0, 555, 0, 555, 0, white));
  world.objects.add(new XzRect(0, 555, 0, 555, 555, white));
  world.objects.add(new XyRect(0, 555, 0, 555, 555, white));

  auto aluminium = new Metal(Color(0.8, 0.85, 0.88), 0.0);
  Hittable *box1 = new Box(Point3(0, 0, 0), Point3(165, 330, 165), aluminium);
  box1 = new RotateY(box1, 15);
  box1 = new Translate(box1, Vec3(265, 0, 295));
  world.objects.add(box1);

  /*
  Hittable *box2 = new Box(Point3(0, 0, 0), Point3(165, 165, 165), white);
  box2 = new RotateY(box2, -18);
  box2 = new Translate(box2, Vec3(130, 0, 65));
  world.objects.add(box2);
  */
  auto glass = new Dielectric(1.5);
  world.objects.add(new Sphere(Point3(190, 90, 190), 90, glass));

  return world;
}

HittableList cornell_smoke() {
  HittableList objects;

  auto red   = new Lambertian(Color(.65, .05, .05));
  auto white = new Lambertian(Color(.73, .73, .73));
  auto green = new Lambertian(Color(.12, .45, .15));
  auto light = new DiffuseLight(Color(7, 7, 7));

  objects.add(new YzRect(0, 555, 0, 555, 555, green));
  objects.add(new YzRect(0, 555, 0, 555, 0, red));
  objects.add(new XzRect(113, 443, 127, 432, 554, light));
  objects.add(new XzRect(0, 555, 0, 555, 555, white));
  objects.add(new XzRect(0, 555, 0, 555, 0, white));
  objects.add(new XyRect(0, 555, 0, 555, 555, white));

  Hittable *box1 = new Box(Point3(0, 0, 0), Point3(165, 330, 165), white);
  box1 = new RotateY(box1, 15);
  box1 = new Translate(box1, Vec3(265, 0, 295));

  Hittable *box2 = new Box(Point3(0, 0, 0), Point3(165, 165, 165), white);
  box2 = new RotateY(box2, -18);
  box2 = new Translate(box2, Vec3(130, 0, 65));

  objects.add(new ConstantMedium(box1, 0.01, Color(0, 0, 0)));
  objects.add(new ConstantMedium(box2, 0.01, Color(1, 1, 1)));

  return objects;
}

HittableList final_scene() {
  HittableList boxes1;
  auto ground = new Lambertian(Color(0.48, 0.83, 0.53));

  const int boxes_per_side = 20;
  for (int i = 0; i < boxes_per_side; i++) {
    for (int j = 0; j < boxes_per_side; j++) {
      auto w = 100.0;
      auto x0 = -1000.0 + i*w;
      auto z0 = -1000.0 + j*w;
      auto y0 = 0.0;
      auto x1 = x0 + w;
      auto y1 = random_double(1,101);
      auto z1 = z0 + w;

      boxes1.add(new Box(Point3(x0, y0, z0), Point3(x1, y1, z1), ground));
    }
  }

  HittableList objects;

  objects.add(new BvhNode(boxes1, 0, 1));

  auto light = new DiffuseLight(Color(7, 7, 7));
  objects.add(new XzRect(123, 423, 147, 412, 554, light));

  auto center1 = Point3(400, 400, 200);
  auto center2 = center1 + Vec3(30,0,0);
  auto moving_sphere_material = new Lambertian(Color(0.7, 0.3, 0.1));
  objects.add(new MovingSphere(center1, center2, 0, 1, 50, moving_sphere_material));

  objects.add(new Sphere(Point3(260, 150, 45), 50, new Dielectric(1.5)));
  objects.add(new Sphere(
    Point3(0, 150, 145), 50, new Metal(Color(0.8, 0.8, 0.9), 1.0)
  ));

  auto boundary = new Sphere(Point3(360,150,145), 70, new Dielectric(1.5));
  objects.add(boundary);
  objects.add(new ConstantMedium(boundary, 0.2, Color(0.2, 0.4, 0.9)));
  boundary = new Sphere(Point3(0, 0, 0), 5000, new Dielectric(1.5));
  objects.add(new ConstantMedium(boundary, .0001, Color(1, 1, 1)));

  auto emat = new Lambertian(new ImageTexture("earthmap.jpg"));
  objects.add(new Sphere(Point3(400, 200, 400), 100, emat));
  auto pertext = new NoiseTexture(0.1);
  objects.add(new Sphere(Point3(220, 280, 300), 80, new Lambertian(pertext)));

  HittableList boxes2;
  auto white = new Lambertian(Color(.73, .73, .73));
  int ns = 1000;
  for (int j = 0; j < ns; j++) {
    boxes2.add(new Sphere(Point3::random(0,165), 10, white));
  }

  objects.add(
    new Translate(
      new RotateY(
        new BvhNode(boxes2, 0.0, 1.0),
        15
      ),
      Vec3(-100,270,395)
    )
  );

  return objects;
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
  auto aspect_ratio = 16.0 / 9.0;
  int width = 1920/5;
  int min_samples_per_pixel = 100;
  int max_samples_per_pixel = 100000000;
  int max_depth = 25;
  double pincer_limit = 0.0001; // 0.000005;
  Color background(0, 0, 0);

  // Camera settings
  auto look_from = Point3(13, 2, 3);
  auto look_at = Point3(0, 0, 0);
  auto vup = Vec3(0, 1, 0);
  auto vfov = 20.0;
  auto dist_to_focus = 10.0;
  auto aperture = .1;
  auto time0 = 0.0;
  auto time1 = 1.0;

  // World
  World world;
  HittableList objects;
  HittableList lights;

  switch(0) {
  case 1:
    objects = random_scene();
    background = Color(0.70, 0.80, 1.00);
    look_from = Point3(13, 2, 3);
    look_at = Point3(0, 0, 0);
    vfov = 20.0;
    aperture = 0.1;
    break;

  case 2:
    objects = two_spheres();
    background = Color(0.70, 0.80, 1.00);
    look_from = Point3(13, 2, 3);
    look_at = Point3(0, 0, 0);
    vfov = 20.0;
    break;

  case 3:
    objects = two_perlin_spheres();
    background = Color(0.70, 0.80, 1.00);
    look_from = Point3(13, 2, 3);
    look_at = Point3(0,0,0);
    vfov = 20.0;
    break;

  case 4:
    objects = earth();
    look_from = Point3(13, 2, 3);
    background = Color(0.70, 0.80, 1.00);
    look_at = Point3(0, 0, 0);
    vfov = 20.0;
    break;

  case 5:
    objects = simple_light();
    background = Color(0, 0, 0);
    look_from = Point3(26, 3, 6);
    look_at = Point3(0, 2, 0);
    vfov = 20.0;
    break;

  case 6:
    world = cornell_box();
    objects = world.objects;
    lights = world.lights;
    aspect_ratio = 1.0;
    width = 600;
    max_depth = 3;
    background = Color(0, 0, 0);
    look_from = Point3(278, 278, -800);
    look_at = Point3(278, 278, 0);
    vfov = 40.0;
    dist_to_focus = 400;
    break;

  case 7:
    objects = cornell_smoke();
    aspect_ratio = 1.0;
    width = 600;
    max_samples_per_pixel = 200;
    look_from = Point3(278, 278, -800);
    look_at = Point3(278, 278, 0);
    vfov = 40.0;
    break;

  case 8:
    objects = final_scene();
    aspect_ratio = 1.0;
    width = 80;
    max_samples_per_pixel = 10000;
    background = Color(0, 0, 0);
    look_from = Point3(478, 278, -600);
    look_at = Point3(278, 278, 0);
    dist_to_focus = (look_at - look_from).length();
    vfov = 40.0;
    break;

  default:
  case 9:
    world = cornell_box();
    objects = world.objects;
    lights = world.lights;

    aspect_ratio = 1.0;
    width = 600;
    max_depth = 50;
    min_samples_per_pixel = 50;
    max_samples_per_pixel = 1000;
    background = Color(0, 0, 0);
    look_from = Point3(278, 278, -800);
    look_at = Point3(278, 278, 0);
    vfov = 40.0;
    dist_to_focus = 10.0; //(look_at - look_from).length();
    time0 = 0;
    time1 = 1;
    aperture = 0.0;
    break;

  }

  // Camera
  int height = static_cast<int>(width / aspect_ratio);
  Camera cam(look_from, look_at, vup, vfov, aspect_ratio, aperture, dist_to_focus, time0, time1);

  // Image data
  std::vector<double> data(3 * height * width);
  std::vector<int> scanlines;
  for(int i = 0; i < height; ++i) {
    scanlines.push_back(i);
  }

  // Render
  std::cerr << "Begin" << std::endl;
  int64_t line_count = 0;
  int64_t sample_count = 0;
  int64_t samples_reached_max = 0;
  int64_t next_save = height / 10;
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
      &objects,
      &lights,
      &max_depth,
      &pincer_limit,
      &min_samples_per_pixel,
      &max_samples_per_pixel,
      &line_count,
      &sample_count,
      &next_save,
      &mutex,
      &background,
      &samples_reached_max,
      &filename
    ] (auto &&j) {
      int64_t line_sample_count = 0;
      int64_t local_reached_max = 0;
      for (int i = 0; i < width; ++i) {
        // Pre-emptively increase max counter
        local_reached_max++;
        Color c1(0, 0, 0), c2(0, 0, 0);
        int64_t count = 0;
        for(int s = 0; s < min_samples_per_pixel / 2; s++) {
          auto u = (i + random_double()) / (width - 1);
          auto v = (j + random_double()) / (height - 1);

          // Ray calculation contains some randomness
#ifdef SAMPLE_CLAMP
          c1 += clamp_color(ray_color(cam.get_ray(u, v), background, objects, lights, max_depth), SAMPLE_CLAMP);
          c2 += clamp_color(ray_color(cam.get_ray(u, v), background, objects, lights, max_depth), SAMPLE_CLAMP);
#else
          c1 += ray_color(cam.get_ray(u, v), background, objects, lights, max_depth);
          c2 += ray_color(cam.get_ray(u, v), background, objects, lights, max_depth);
#endif
          count += 2;
        }

        // Adaptive loop
        for(int s = min_samples_per_pixel / 2; s < max_samples_per_pixel / 2; s++) {
          auto u = (i + random_double()) / (width - 1);
          auto v = (j + random_double()) / (height - 1);

          // Ray r = cam.get_ray(u, v);
          // pixel_color += ray_color(r, objects, lights, max_depth);
          // Ray calculation contains some randomness
#ifdef SAMPLE_CLAMP
          c1 += clamp_color(ray_color(cam.get_ray(u, v), background, objects, lights, max_depth), SAMPLE_CLAMP);
          c2 += clamp_color(ray_color(cam.get_ray(u, v), background, objects, lights, max_depth), SAMPLE_CLAMP);
#else
          c1 += ray_color(cam.get_ray(u, v), background, objects, lights, max_depth);
          c2 += ray_color(cam.get_ray(u, v), background, objects, lights, max_depth);
#endif
          count += 2;

          auto dist = fabs((c1.x() - c2.x()) + (c1.y() - c2.y()) + (c1.z() - c2.z()));
          auto total = (c1.x() + c2.x()) + (c1.y() + c2.y()) + (c1.z() + c2.z());
          if(dist / total < pincer_limit) {
            // Restore max counter
            local_reached_max--;
            break;
          }
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
        samples_reached_max += local_reached_max;
        std::cerr << "\rScanline "
                  << line_count
                  << "/"
                  << height
                  << " "
                  << samples_reached_max
                  << " ceilings hit and "
                  << ((double)sample_count / (width * line_count))
                  << " samples per pixel"
                  << std::flush;
        if (line_count > next_save) {
          save_png(data, width, height, filename);
          next_save += height / 10;
        }
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
