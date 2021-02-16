#ifndef COLOR_H
#define COLOR_H

#include "vec3.hpp"

#include <iostream>

using Color = Vec3;    // RGB color

void write_color(std::ostream &out, Color pixel_color) {
    // Write the translated [0,255] value of each color component.
    out << static_cast<int>(255.999 * pixel_color.x()) << ' '
        << static_cast<int>(255.999 * pixel_color.y()) << ' '
        << static_cast<int>(255.999 * pixel_color.z()) << '\n';
}

#endif
