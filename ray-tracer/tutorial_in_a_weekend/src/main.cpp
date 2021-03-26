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

#include "rtweekend.hpp"
#include "ray.hpp"
#include "vec3.hpp"
#include "color.hpp"
#include "hittable_list.hpp"
#include "sphere.hpp"
#include "moving_sphere.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "pdf.hpp"
#include "texture.hpp"

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


Color ray_color(const Ray &r, const Color &background, const Hittable &world, Hittable &lights, int depth)
{
  HitRecord rec;

  if(depth <= 0)
    return Color(0, 0, 0);

  if(!world.hit(r, 0.001, infinity, rec))
    return background;

  ScatterRecord srec;
  //std::cerr << "ray_color before rec.material->emitted()" << std::endl;
  Color emitted = rec.material->emitted(r, rec, rec.u, rec.v, rec.p);

  //std::cerr << "ray_color before !rec.material->scatter()" << std::endl;
  if(!rec.material->scatter(r, rec, srec))
    return emitted;

  if(srec.is_specular) {
    //std::cerr << "ray_color before srec.attenuation * ray_color()" << std::endl;
    return srec.attenuation
      * ray_color(srec.specular_ray, background, world, lights, depth-1);
  }

  Vec3 scatter_direction;
  Ray scattered;
  double pdf;
  if(dynamic_cast<HittableList*>(&lights)->size()) {
    //std::cerr << "ray_color before MixturePdf" << std::endl;
    auto p0 = std::make_shared<HittablePdf>(&lights, rec.p);
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
    * ray_color(scattered, background, world, lights, depth-1);
}

typedef struct {
  Color background;
  std::map<std::string, Texture*> texture_list;
  std::map<std::string, Material*> material_list;
  std::map<std::string, Hittable*> object_list;
  HittableList objects;
  HittableList lights;
} World;


World build_world(json &conf)
{
  World world;
  std::cerr << "Reading background color" << std::endl;
  auto bg = conf["background"];
  world.background = Color(
    bg["red"].get<double>(),
    bg["green"].get<double>(),
    bg["blue"].get<double>()
  );
  std::cerr << "  " << world.background << std::endl;

  std::cerr << "Reading textures" << std::endl;
  for(auto tx : conf["textures"]) {
    try {
      std::string key = tx["name"].get<std::string>();
      std::string texture_type = tx["type"].get<std::string>();

      Texture *new_texture = NULL;
      if( world.texture_list.find(key) != world.texture_list.end() ) {
        throw("The same texture name can't be used twice: '" + key + "'");
      }
      if( texture_type.find("__") != std::string::npos ) {
        throw("Double underscores are not allowed in names: '" + key + "'");
      }

      if( texture_type == "CheckerTexture" ) {
        auto col1 = tx["color1"].get<std::string>();
        auto col2 = tx["color2"].get<std::string>();

        new_texture = new CheckerTexture(
          world.texture_list[col1], world.texture_list[col2]
        );
      } else if( texture_type == "SolidColor" ) {
        auto red = tx["red"].get<double>();
        auto green = tx["green"].get<double>();
        auto blue = tx["blue"].get<double>();

        new_texture = new SolidColor(Color(red, green, blue));
      } else {
        throw("Unknown texture type: '" + texture_type + "'");
      }

      world.texture_list[key] = new_texture;

      world.texture_list[key]->setName(key);
      // std::cerr << "  " + key << std::endl;
    } catch(nlohmann::detail::type_error &e) {
      std::cerr << "Texture failed" << std::endl;
      throw(e);
    }
  }

  std::cerr << "Reading materials" << std::endl;
  for(auto mtl : conf["materials"]) {
    try {
      std::string key = mtl["name"];
      std::string material_type = mtl["type"];

      if( world.material_list.find(key) != world.material_list.end() ) {
        throw("The same material name can't be used twice: '" + key + "'");
      }
      if( material_type.find("__") != std::string::npos ) {
        throw("Double underscores are not allowed in names: '" + key + "'");
      }

      if( material_type == "Lambertian" ) {
        auto tex = mtl["texture"].get<std::string>();

        world.material_list[key] = new Lambertian(world.texture_list[tex]);
      } else if( material_type == "DiffuseLight" ) {
        auto tex = mtl["texture"].get<std::string>();

        world.material_list[key] = new DiffuseLight(world.texture_list[tex]);
      } else if( material_type == "Metal" ) {
        auto red = mtl["red"].get<double>();
        auto green = mtl["green"].get<double>();
        auto blue = mtl["blue"].get<double>();

        auto fuzz = mtl["fuzz"].get<double>();

        world.material_list[key] = new Metal(Color(red, green, blue), fuzz);
      } else if( material_type == "Dielectric" ) {
        auto refraction = mtl["refraction"].get<double>();

        world.material_list[key] = new Dielectric(refraction);
      } else {
        throw("Unknown material type: '" + material_type + "'");
      }

      world.material_list[key]->setName(key);
      // std::cerr << "  " + key << std::endl;
    } catch(nlohmann::detail::type_error &e) {
      std::cerr << "Material failed" << std::endl;
      throw(e);
    }
  }

  std::cerr << "Reading objects" << std::endl;
  for(auto obj : conf["objects"]) {
    try {
      std::string key = obj["name"];
      std::string object_type = obj["type"];

      if( world.object_list.find(key) != world.object_list.end() ) {
        throw("The same object name can't be used twice: '" + key + "'");
      }
      if( object_type.find("__") != std::string::npos ) {
        throw("Double underscores are not allowed in names: '" + key + "'");
      }

      if( object_type == "Sphere" ) {
        auto c = obj["center"];
        auto x = c[0].get<double>();
        auto y = c[1].get<double>();
        auto z = c[2].get<double>();
        auto center = Point3(x, y, z);
        auto radius = obj["radius"].get<double>();
        auto material = world.material_list[obj["material"].get<std::string>()];
        world.object_list[key] = new Sphere(
          center, radius, material
        );
      } else if( object_type == "MovingSphere" ) {
        auto c = obj["center0"];
        auto x = c[0].get<double>();
        auto y = c[1].get<double>();
        auto z = c[2].get<double>();
        auto center0 = Point3(x, y, z);

        c = obj["center1"];
        x = c[0].get<double>();
        y = c[1].get<double>();
        z = c[2].get<double>();
        auto center1 = Point3(x, y, z);

        auto radius = obj["radius"].get<double>();

        auto time0 = obj["time0"].get<double>();
        auto time1 = obj["time1"].get<double>();

        auto material = world.material_list[obj["material"].get<std::string>()];
        world.object_list[key] = new MovingSphere(
          center0, center1, time0, time1, radius, material
        );
      } else {
        throw("Unknown object type: '" + object_type + "'");
      }

      world.object_list[key]->setName(key);
      world.objects.add(world.object_list[key]);
      // std::cerr << "  " + key << std::endl;
    } catch(nlohmann::detail::type_error &e) {
      std::cerr << "Objects failed" << std::endl;
      throw(e);
    }
  }

  std::cerr << "Reading lights" << std::endl;
  for(auto light : conf["lights"]) {
    try {
      std::string key = light.get<std::string>();
      // std::cerr << "Adding light referense to object '" << key << "'" << std::endl;

      world.lights.add(world.object_list[key]);

      // std::cerr << "  " + key << std::endl;
    } catch(nlohmann::detail::type_error &e) {
      std::cerr << "Lights failed" << std::endl;
      throw(e);
    }
  }

  return world;
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
  double aspect_ratio;
  int width;
  int height;
  int min_samples_per_pixel;
  int max_samples_per_pixel;
  int max_depth;
  double pincer_limit;
  Color background(0, 0, 0);

  // Camera settings
  Point3 look_from;
  Point3 look_at;
  Vec3 vup;
  double vfov;
  double dist_to_focus;
  double aperture;
  double time0;
  double time1;

  // World
  World world;
  HittableList objects;
  HittableList lights;

  try {
    std::ifstream camera_file("../camera.json", std::ifstream::in);
    json camera_conf;
    camera_file >> camera_conf;

    look_from = Point3(camera_conf["look_from"][0], camera_conf["look_from"][1], camera_conf["look_from"][2]);
    look_at = Point3(camera_conf["look_at"][0], camera_conf["look_at"][1], camera_conf["look_at"][2]);
    vup = Vec3(camera_conf["up"][0], camera_conf["up"][1], camera_conf["up"][2]);
    vfov = camera_conf["vertical_fov"];
    dist_to_focus = camera_conf["dist_to_focus"];
    aperture = camera_conf["aperture"];
    time0 = camera_conf["time_start"];
    time1 = camera_conf["time_end"];
  } catch(nlohmann::detail::parse_error &e) {
    std::cout << "No camera file found (" << e.what() << ")" << std::endl;
    return -1;
  }

  try {
    std::ifstream render_file("../render.json", std::ifstream::in);
    json render_conf;
    render_file >> render_conf;

    width = render_conf["width"].get<int>();
    height = render_conf["height"].get<int>();
    min_samples_per_pixel = render_conf["min_samples_per_pixel"].get<int>();
    max_samples_per_pixel = render_conf["max_samples_per_pixel"].get<int>();
    max_depth = render_conf["max_depth"].get<int>();
    pincer_limit = render_conf["pincer_limit"].get<double>();


  } catch(nlohmann::detail::parse_error &e) {
    std::cout << "No render file found (" << e.what() << ")" << std::endl;
    return -1;
  }

  aspect_ratio = (double)width / height;
  try {
    std::ifstream world_file("../world.json", std::ifstream::in);
    json world_conf;
    world_file >> world_conf;
    world = build_world(world_conf);
  } catch(nlohmann::detail::parse_error &e) {
    std::cerr << "No world file found (" << e.what() << ")" << std::endl;
    return -1;
  }

  objects = world.objects;
  lights = world.lights;
  background = world.background;

  // Camera
  Camera cam(look_from, look_at, vup, vfov, aspect_ratio, aperture, dist_to_focus, time0, time1);

  // Image data
  std::vector<double> data(3LL * height * width);
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
          auto u = (i + random_double()) / ((double)width - 1);
          auto v = (j + random_double()) / ((double)height - 1);

          // Ray calculation contains some randomness
          auto tmpc1 = ray_color(cam.get_ray(u, v), background, objects, lights, max_depth);
          auto tmpc2 = ray_color(cam.get_ray(u, v), background, objects, lights, max_depth);

	  if(
	     is_nan(tmpc1) || is_nan(tmpc2)
	     || tmpc1.length() > MAX_COLOR || tmpc2.length() > MAX_COLOR
	     ) {
	    // std::cerr << "Color either NaN or too large: " << tmpc1 << ", " << tmpc2 << std::endl;
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
        for(int s = min_samples_per_pixel / 2; s < max_samples_per_pixel / 2; s++) {
          auto u = (i + random_double()) / ((double)width - 1);
          auto v = (j + random_double()) / ((double)height - 1);

          auto tmpc1 = ray_color(cam.get_ray(u, v), background, objects, lights, max_depth);
          auto tmpc2 = ray_color(cam.get_ray(u, v), background, objects, lights, max_depth);

	  if(
	     is_nan(tmpc1) || is_nan(tmpc2)
	     || tmpc1.length() > MAX_COLOR || tmpc2.length() > MAX_COLOR
             ) {
	    // std::cerr << "Color either NaN or too large: " << tmpc1 << ", " << tmpc2 << std::endl;
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
          if(dist / total < pincer_limit) {
            // Restore max counter
            local_reached_max--;
            break;
          }
        }

        line_sample_count += count;
        data[((int64_t)height - j - 1) * width * 3 + i * 3 + 0] = (c1 + c2).x() / count;
        data[((int64_t)height - j - 1) * width * 3 + i * 3 + 1] = (c1 + c2).y() / count;
        data[((int64_t)height - j - 1) * width * 3 + i * 3 + 2] = (c1 + c2).z() / count;
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
  std::cerr << "Average " << ((double)sample_count / ((double)width * height)) << " samples per pixel" << std::endl;

  // Dump image
  std::cerr << "Saving image to '" << filename << "'" << std::endl;
  save_png(data, width, height, filename);

  std::cerr << "\nDone." << std::endl;

  return 0;
}
