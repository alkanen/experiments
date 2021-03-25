#pragma "once"

#include <iostream>

#include "rtweekend.hpp"
#include "vec3.hpp"

using Color = Vec3;    // RGB color

Color clamp_color(const Color& color, double maxval);
bool is_nan(Color &color);
void color_adjust_nan(Color &color);
void write_color(std::ostream &out, Color pixel_color, int samples_per_pixel);
