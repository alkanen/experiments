#include "png.hpp"

#include <vector>
#include <cstdint>

#include "stb_image.h"
#include "stb_image_write.h"

#include "rtweekend.hpp"

void save_png(std::vector<double> &data, const int width, const int height, const char *filename)
{
  int y, x;
  uint8_t *pixels;
  const int64_t pitch = width * 3;
  pixels = new uint8_t[data.size()];

  // Change from RGB to BGR because PNG sucks.
  for(y=0; y<height; y++) {
    for(x=0; x < pitch - 3; x += 3) {
      auto r = data[(int64_t)y * pitch + x + 0LL];
      auto g = data[(int64_t)y * pitch + x + 1LL];
      auto b = data[(int64_t)y * pitch + x + 2LL];

      pixels[(int64_t)y * pitch + x + 0LL] = static_cast<int>(256 * clamp(sqrt(r), 0.0, 0.999));
      pixels[(int64_t)y * pitch + x + 1LL] = static_cast<int>(256 * clamp(sqrt(g), 0.0, 0.999));
      pixels[(int64_t)y * pitch + x + 2LL] = static_cast<int>(256 * clamp(sqrt(b), 0.0, 0.999));
    }
  }

  stbi_write_png( filename, width, height, STBI_rgb, pixels, (int)pitch );
  delete[] pixels;
}
