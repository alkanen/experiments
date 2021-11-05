#ifndef FIXED_POINT
#define FIXED_POINT

#include <stdint.h>

typedef int64_t Ordinate;
typedef uint64_t Distance;

typedef struct Vec3D_s {
  Ordinate x;
  Ordinate y;
  Ordinate z;
} Vec3D;

typedef struct Vec2D_s {
  Ordinate x;
  Ordinate y;
} Vec2D;

typedef Vec3D Coordinate;

// Calculates the distance between two coordinates in an overflow safe manner
Distance distance(Coordinate p1, Coordinate p2);

#endif
