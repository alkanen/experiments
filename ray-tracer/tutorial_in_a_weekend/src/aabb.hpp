#ifndef AABB_HPP
#define AABB_HPP

#include "rtweekend.hpp"

class Aabb {
public:
  Aabb() {}
  Aabb(const Point3 &a, const Point3 &b)
  {
    minimum = a;
    maximum = b;
  }

  Point3 min() const { return minimum; }
  Point3 max() const { return maximum; }

  bool hit(const Ray& r, double t_min, double t_max) const;
  /*
  {
    for (int a = 0; a < 3; a++) {
      auto t0 = fmin(
        (minimum[a] - r.origin()[a]) / r.direction()[a],
        (maximum[a] - r.origin()[a]) / r.direction()[a]
      );
      auto t1 = fmax(
        (minimum[a] - r.origin()[a]) / r.direction()[a],
        (maximum[a] - r.origin()[a]) / r.direction()[a]
      );
      t_min = fmax(t0, t_min);
      t_max = fmin(t1, t_max);
      if(t_max <= t_min)
        return false;
    }
    return true;
  }
  */

public:
  Point3 minimum;
  Point3 maximum;
};


inline bool Aabb::hit(const Ray &r, double t_min, double t_max) const {
  for (int a = 0; a < 3; a++) {
    auto invD = 1.0f / r.direction()[a];
    auto t0 = (min()[a] - r.origin()[a]) * invD;
    auto t1 = (max()[a] - r.origin()[a]) * invD;
    if (invD < 0.0f)
      std::swap(t0, t1);
    t_min = t0 > t_min ? t0 : t_min;
    t_max = t1 < t_max ? t1 : t_max;
    if (t_max <= t_min)
      return false;
  }
  return true;
}

Aabb surrounding_box(Aabb box0, Aabb box1)
{
  Point3 small(
    fmin(box0.min().x(), box1.min().x()),
    fmin(box0.min().y(), box1.min().y()),
    fmin(box0.min().z(), box1.min().z())
  );

  Point3 big(
    fmax(box0.max().x(), box1.max().x()),
    fmax(box0.max().y(), box1.max().y()),
    fmax(box0.max().z(), box1.max().z())
  );

  return Aabb(small, big);
}

#endif
