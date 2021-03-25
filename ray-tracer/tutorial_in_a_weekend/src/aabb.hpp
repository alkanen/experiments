#ifndef AABB_HPP
#define AABB_HPP

#include "rtweekend.hpp"

class Aabb {
public:
  Aabb() {}
  Aabb(const Point3 &a, const Point3 &b);
  Point3 min() const;
  Point3 max() const;
  bool hit(const Ray& r, double t_min, double t_max) const;

public:
  Point3 minimum;
  Point3 maximum;
};

Aabb surrounding_box(Aabb box0, Aabb box1);

#endif
