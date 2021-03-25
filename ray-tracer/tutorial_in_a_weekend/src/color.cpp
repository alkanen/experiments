#include "color.hpp"
#include <iostream>

Color clamp_color(const Color& color, double maxval)
{
  auto ratio = maxval / color.length();
  if (ratio >= 1)
    return color;

  return color * ratio;
}

bool is_nan(Color &color)
{
  return (color[0] != color[0]) || (color[1] != color[1]) || (color[2] != color[2]);
}

void color_adjust_nan(Color &color)
{
  if (color[0] != color[0])
    color[0] = 0;
  if (color[1] != color[1])
    color[1] = 0;
  if (color[2] != color[2])
    color[2] = 0;
}

void write_color(std::ostream &out, Color pixel_color, int samples_per_pixel)
{
  auto r = pixel_color.x();
  auto g = pixel_color.y();
  auto b = pixel_color.z();

  auto scale = 1.0 / samples_per_pixel;
  r = sqrt(scale * r);
  g = sqrt(scale * g);
  b = sqrt(scale * b);

  // Write the translated [0,255] value of each color component.
  out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
    << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
    << static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
}
