#ifndef WORLD_HPP
#define WORLD_HPP

#include <map>
#include <string>

#include <nlohmann/json.hpp>

#include "color.hpp"
#include "hittable.hpp"
#include "hittable_list.hpp"
#include "texture.hpp"
#include "material.hpp"

using json = nlohmann::json;

typedef struct {
  Color background;
  std::map<std::string, Texture*> texture_list;
  std::map<std::string, Material*> material_list;
  std::map<std::string, Hittable*> object_list;
  HittableList objects;
  HittableList lights;
} World;

World build_world(json &conf);

#endif
