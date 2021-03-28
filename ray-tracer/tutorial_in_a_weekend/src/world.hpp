#ifndef WORLD_HPP
#define WORLD_HPP

#include <map>
#include <string>

#include <nlohmann/json.hpp>

#include "camera.hpp"
#include "color.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "hittable.hpp"
#include "hittable_list.hpp"

using json = nlohmann::json;

class World {
public:
  World(json &conf, Camera &camera);

public:
  Color background;
  HittableList objects;
  HittableList lights;

private:
  std::map<std::string, Texture*> texture_list;
  std::map<std::string, Material*> material_list;
  std::map<std::string, Hittable*> object_list;
};

#endif
