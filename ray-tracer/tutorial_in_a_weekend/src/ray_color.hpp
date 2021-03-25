#ifndef RAY_COLOR_HPP
#define RAY_COLOR_HPP

#include "color.hpp"
#include "hittable.hpp"
#include "ray.hpp"

Color ray_color(const Ray &r, const Color &background, const Hittable &world, Hittable &lights, int depth);
bool is_nan(Color &color);
void color_adjust_nan(Color &color);
void write_color(std::ostream &out, Color pixel_color, int samples_per_pixel);

#endif
