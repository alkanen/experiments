#ifndef RAY_COLOR_HPP
#define RAY_COLOR_HPP

#include "color.hpp"
#include "ray.hpp"
#include "world.hpp"

Color ray_color(const Ray &r, World &world, int depth);

#endif
