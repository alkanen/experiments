#include "render.hpp"

#include <iostream>

#include <nlohmann/json.hpp>

RenderParams::RenderParams(json &conf)
{
  try {
    width = conf["width"].get<int>();
    height = conf["height"].get<int>();
    aspect_ratio = (double)width / height;
    min_samples_per_pixel = conf["min_samples_per_pixel"].get<int>();
    max_samples_per_pixel = conf["max_samples_per_pixel"].get<int>();
    max_depth = conf["max_depth"].get<int>();
    pincer_limit = conf["pincer_limit"].get<double>();
  } catch(nlohmann::detail::parse_error &e) {
    std::cerr << "Unable to parse render parameters: " << e.what() << std::endl;
    throw(e);
  }
}
