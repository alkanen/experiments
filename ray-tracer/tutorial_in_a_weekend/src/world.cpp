#include "world.hpp"

#include <map>
#include <string>

#include <nlohmann/json.hpp>

#include "camera.hpp"
#include "color.hpp"
#include "hittable_list.hpp"
#include "hittable.hpp"
#include "bvh.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "sphere.hpp"
#include "moving_sphere.hpp"

World::World(json &conf, Camera &camera)
{
  std::cerr << "Reading background color" << std::endl;
  auto bg = conf["background"];
  background = Color(
    bg["red"].get<double>(),
    bg["green"].get<double>(),
    bg["blue"].get<double>()
  );
  std::cerr << "  " << background << std::endl;

  std::cerr << "Reading textures" << std::endl;
  for(auto tx : conf["textures"]) {
    try {
      std::string key = tx["name"].get<std::string>();
      std::string texture_type = tx["type"].get<std::string>();

      Texture *new_texture = NULL;
      if( texture_list.find(key) != texture_list.end() ) {
        throw("The same texture name can't be used twice: '" + key + "'");
      }
      if( texture_type.find("__") != std::string::npos ) {
        throw("Double underscores are not allowed in names: '" + key + "'");
      }

      if( texture_type == "CheckerTexture" ) {
        auto col1 = tx["color1"].get<std::string>();
        auto col2 = tx["color2"].get<std::string>();

        new_texture = new CheckerTexture(
          texture_list[col1], texture_list[col2]
        );
      } else if( texture_type == "SolidColor" ) {
        auto red = tx["red"].get<double>();
        auto green = tx["green"].get<double>();
        auto blue = tx["blue"].get<double>();

        new_texture = new SolidColor(Color(red, green, blue));
      } else {
        throw("Unknown texture type: '" + texture_type + "'");
      }

      texture_list[key] = new_texture;

      texture_list[key]->setName(key);
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

      if( material_list.find(key) != material_list.end() ) {
        throw("The same material name can't be used twice: '" + key + "'");
      }
      if( material_type.find("__") != std::string::npos ) {
        throw("Double underscores are not allowed in names: '" + key + "'");
      }

      if( material_type == "Lambertian" ) {
        auto tex = mtl["texture"].get<std::string>();

        material_list[key] = new Lambertian(texture_list[tex]);
      } else if( material_type == "DiffuseLight" ) {
        auto tex = mtl["texture"].get<std::string>();

        material_list[key] = new DiffuseLight(texture_list[tex]);
      } else if( material_type == "Metal" ) {
        auto red = mtl["red"].get<double>();
        auto green = mtl["green"].get<double>();
        auto blue = mtl["blue"].get<double>();

        auto fuzz = mtl["fuzz"].get<double>();

        material_list[key] = new Metal(Color(red, green, blue), fuzz);
      } else if( material_type == "Dielectric" ) {
        auto refraction = mtl["refraction"].get<double>();

        material_list[key] = new Dielectric(refraction);
      } else {
        throw("Unknown material type: '" + material_type + "'");
      }

      material_list[key]->setName(key);
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

      if( object_list.find(key) != object_list.end() ) {
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
        auto material = material_list[obj["material"].get<std::string>()];
        object_list[key] = new Sphere(
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

        auto material = material_list[obj["material"].get<std::string>()];
        object_list[key] = new MovingSphere(
          center0, center1, time0, time1, radius, material
        );
      } else {
        throw("Unknown object type: '" + object_type + "'");
      }

      object_list[key]->setName(key);
      // objects.add(object_list[key]);
      // std::cerr << "  " + key << std::endl;
    } catch(nlohmann::detail::type_error &e) {
      std::cerr << "Objects failed" << std::endl;
      throw(e);
    }
  }

  std::vector<Hittable*> tmp_objects;
  for( auto it = object_list.begin(); it != object_list.end(); ++it ) {
    tmp_objects.push_back( it->second );
  }
  // What the hell should the time values be set to here?
  object_list["global_bvh"] = new BvhNode(
    tmp_objects, camera.start_time(), camera.end_time()
  );
  objects.add(object_list["global_bvh"]);

  std::cerr << "Reading lights" << std::endl;
  for(auto light : conf["lights"]) {
    try {
      std::string key = light.get<std::string>();
      // std::cerr << "Adding light referense to object '" << key << "'" << std::endl;

      lights.add(object_list[key]);

      // std::cerr << "  " + key << std::endl;
    } catch(nlohmann::detail::type_error &e) {
      std::cerr << "Lights failed" << std::endl;
      throw(e);
    }
  }
}
