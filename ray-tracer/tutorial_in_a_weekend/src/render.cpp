#include "render.hpp"

#include <iostream>

#include <nlohmann/json.hpp>

RenderParams::RenderParams(json &conf)
{
  try {
    std::cerr << "Json: " << conf << std::endl;
    std::cerr << "Load width: " << conf["width"] << std::endl;
    width = conf["width"].get<int>();
    std::cerr << "Load height" << std::endl;
    height = conf["height"].get<int>();
    std::cerr << "Load min per pix" << std::endl;
    min_samples_per_pixel = conf["min_samples_per_pixel"].get<int>();
    std::cerr << "Load max per pix" << std::endl;
    max_samples_per_pixel = conf["max_samples_per_pixel"].get<int>();
    std::cerr << "Load depth" << std::endl;
    max_depth = conf["max_depth"].get<int>();
    std::cerr << "Load pincer" << std::endl;
    pincer_limit = conf["pincer_limit"].get<double>();
    std::cerr << "Done" << std::endl;
  } catch(nlohmann::detail::parse_error &e) {
    std::cerr << "Unable to parse render parameters: " << e.what() << std::endl;
    throw(e);
  }
}
