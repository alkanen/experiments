#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Config {
public:
  Config(json &conf);

public:
  std::string render_config;
  std::string camera_config;
  std::string world_config;

  int segment_height;
  int segment_width;
  int saves_per_render;

  std::string output_image;
  std::string output_folder;
};

#endif
