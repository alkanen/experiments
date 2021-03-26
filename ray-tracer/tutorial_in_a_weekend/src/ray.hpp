#ifndef RAY_H
#define RAY_H

#include <iostream>

#include "vec3.hpp"

class Ray {
public:
  Ray();
  Ray(const Point3& origin, const Vec3& direction, double time = 0.0);

  Point3 origin() const;
  Vec3 direction() const;
  double time() const;

  Point3 at(double t) const;

public:
  Point3 orig;
  Vec3 dir;
  double tm;
};

std::ostream& operator<<(std::ostream &out, const Ray &r);

#endif
