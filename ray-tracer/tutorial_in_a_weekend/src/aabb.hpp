#ifndef AABB_HPP
#define AABB_HPP

#include <iostream>

#include "vec3.hpp"
#include "ray.hpp"

class Aabb {
public:
  Aabb();
  Aabb(const Point3 &a, const Point3 &b);
  Point3 min() const;
  Point3 max() const;

  bool hit(const Ray& r, double t_min, double t_max) const;
  double volume(void) const;

public:
  Point3 minimum;
  Point3 maximum;
};

Aabb surrounding_box(Aabb box0, Aabb box1);
Aabb intersection_box(const Aabb &box0, const Aabb &box1);
std::ostream& operator<<(std::ostream &out, const Aabb &box);

#endif
