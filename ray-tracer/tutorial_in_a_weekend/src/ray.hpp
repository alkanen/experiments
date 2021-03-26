#ifndef RAY_H
#define RAY_H

#include <iostream>

#include "vec3.hpp"

using Point3 = Vec3;

class Ray {
public:
  Ray() {}
  Ray(const Point3& origin, const Vec3& direction, double time = 0.0)
    : orig(origin), dir(direction), tm(time)
  {}

  Point3 origin() const  { return orig; }
  Vec3 direction() const { return dir; }
  double time() const    { return tm; }

  Point3 at(double t) const {
    return orig + t * dir;
  }

public:
  Point3 orig;
  Vec3 dir;
  double tm;
};

inline std::ostream& operator<<(std::ostream &out, const Ray &r)
{
  return out << "Ray(" << r.origin() << ", " << r.direction() << ')';
}

#endif
