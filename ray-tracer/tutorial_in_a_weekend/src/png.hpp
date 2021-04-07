#ifndef PNG_HPP
#define PNG_HPP

#include <string>
#include <vector>

#include "color.hpp"

void save_png(std::vector<Color> &data, const int width, const int height, const std::string &filename);

#endif
