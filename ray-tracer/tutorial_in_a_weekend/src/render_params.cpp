#include "render_params.hpp"

#include <iostream>

#include <nlohmann/json.hpp>

RenderParams::RenderParams(json &conf)
{
  try {
    width = conf["width"].get<int>();
    height = conf["height"].get<int>();
    aspect_ratio = (double)width / height;
    sample_grid_size = conf["sample_grid_size"].get<int>();
    min_samples_per_pixel = conf["min_samples_per_pixel"].get<int>();
    max_samples_per_pixel = conf["max_samples_per_pixel"].get<int>();
    max_depth = conf["max_depth"].get<int>();
    pincer_limit = conf["pincer_limit"].get<double>();
  } catch(nlohmann::detail::parse_error &e) {
    std::cerr << "Unable to parse render parameters: " << e.what() << std::endl;
    throw(e);
  }
}

RenderSegment::RenderSegment(json &conf)
{
  try {
    x1 = conf["x1"].get<int>();
    y1 = conf["y1"].get<int>();
    x2 = conf["x2"].get<int>();
    y2 = conf["y2"].get<int>();
  } catch(nlohmann::detail::parse_error &e) {
    std::cerr << "Unable to parse render parameters: " << e.what() << std::endl;
    throw(e);
  }

  sort();
}

RenderSegment::RenderSegment(int x1, int y1, int x2, int y2)
  : x1(x1)
  , y1(y1)
  , x2(x2)
  , y2(y2)
{
  sort();
}

void RenderSegment::sort(void)
{
  if(x1 > x2) {
    auto tmp = x1;
    x1 = x2;
    x2 = tmp;
  }

  if(y1 > y2) {
    auto tmp = y1;
    y1 = y2;
    y2 = tmp;
  }
}
