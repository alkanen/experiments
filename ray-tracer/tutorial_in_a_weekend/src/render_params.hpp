#ifndef RENDER_HPP
#define RENDER_HPP

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class RenderParams {
public:
  RenderParams(json &conf);

public:
  int width;
  int height;
  double aspect_ratio;
  // Width of the grid used to get samples over a pixel
  int sample_grid_size;
  int min_samples_per_pixel;
  int max_samples_per_pixel;
  int max_depth;
  double pincer_limit;
};

class RenderSegment {
public:
  RenderSegment(json &conf);
  RenderSegment(int x1, int y1, int x2, int y2);

private:
  void sort(void);

public:
  int x1;
  int y1;
  int x2;
  int y2;
};

#endif
