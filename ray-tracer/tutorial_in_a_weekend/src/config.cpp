#include "config.hpp"

#include <iostream>

#include <nlohmann/json.hpp>

Config::Config(json &conf)
{
  try {
    render_config = conf["render_params"].get<std::string>();
    camera_config = conf["camera"].get<std::string>();
    world_config = conf["world"].get<std::string>();
    segment_height = conf["segment_height"].get<int>();
    segment_width = conf["segment_width"].get<int>();
    saves_per_render = conf["saves_per_render"].get<int>();
    output_image = conf["output_image"].get<std::string>();
    output_folder = conf["output_folder"].get<std::string>();
  } catch(nlohmann::detail::parse_error &e) {
    std::cerr << "Unable to parse config: " << e.what() << std::endl;
    throw(e);
  }
}
