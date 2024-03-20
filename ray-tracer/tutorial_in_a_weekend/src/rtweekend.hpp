#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <cmath>
#include <limits>
#include <memory>

// Usings

using std::sqrt;

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degrees)
{
  return degrees * pi / 180.0;
}

inline double random_double()
{
  // Returns a random real in [0,1).
  return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max)
{
  // Returns a random real in [min,max).
  return min + (max - min) * random_double();
}

inline int random_int(int min, int max)
{
    // Returns a random integer in [min,max].
    return static_cast<int>(random_double(min, max+1));
}

inline double clamp(double x, double min, double max)
{
  if (x < min)
    return min;
  if (x > max)
    return max;
  return x;
}

// Common Headers

#include "ray.hpp"
#include "vec3.hpp"
#include "color.hpp"

#endif