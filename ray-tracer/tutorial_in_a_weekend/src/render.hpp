#ifndef RENDER_HPP
#define RENDER_HPP

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class  RenderParams {
public:
  RenderParams(json &conf);

public:
  int width;
  int height;
  double aspect_ratio;
  int min_samples_per_pixel;
  int max_samples_per_pixel;
  int max_depth;
  double pincer_limit;
};

#endif
