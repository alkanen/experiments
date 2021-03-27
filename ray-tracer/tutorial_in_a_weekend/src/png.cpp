#include "png.hpp"

#include <vector>
#include <cstdint>

#include "stb_image.h"
#include "stb_image_write.h"

#include "rtweekend.hpp"
#include "color.hpp"

void save_png(std::vector<Color> &data, const int width, const int height, const char *filename)
{
  const int64_t pitch = width * 3;
  uint8_t *pixels;
  pixels = new uint8_t[data.size() * 3];

  // Change from RGB to BGR because PNG sucks.
  size_t index = 0;
  for (auto const &c : data) {
    auto r = c.x();
    auto g = c.y();
    auto b = c.z();

    index++[pixels] = static_cast<int>(256 * clamp(sqrt(r), 0.0, 0.999));
    index++[pixels] = static_cast<int>(256 * clamp(sqrt(g), 0.0, 0.999));
    index++[pixels] = static_cast<int>(256 * clamp(sqrt(b), 0.0, 0.999));
  }

  stbi_write_png( filename, width, height, STBI_rgb, pixels, (int)pitch );
  delete[] pixels;
}
