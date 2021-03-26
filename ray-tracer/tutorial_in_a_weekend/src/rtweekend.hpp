#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include "constants.hpp"

// Utility Functions

double degrees_to_radians(double degrees);
double random_double();
double random_double(double min, double max);
int random_int(int min, int max);
double clamp(double x, double min, double max);

// Common Headers

#include "ray.hpp"
#include "vec3.hpp"
#include "color.hpp"

#endif
