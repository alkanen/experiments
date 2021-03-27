#ifndef PNG_HPP
#define PNG_HPP

#include <vector>

#include "color.hpp"

void save_png(std::vector<Color> &data, const int width, const int height, const char *filename);

#endif
