#include "rtweekend.hpp"

#include <cmath>
#include <limits>

using std::sqrt;

double degrees_to_radians(double degrees)
{
  return degrees * pi / 180.0;
}

double random_double()
{
  // Returns a random real in [0,1).
  return rand() / (RAND_MAX + 1.0);
}

double random_double(double min, double max)
{
  // Returns a random real in [min,max).
  return min + (max - min) * random_double();
}

int random_int(int min, int max)
{
    // Returns a random integer in [min,max].
    return static_cast<int>(random_double(min, max+1));
}

double clamp(double x, double min, double max)
{
  if (x < min)
    return min;
  if (x > max)
    return max;
  return x;
}
