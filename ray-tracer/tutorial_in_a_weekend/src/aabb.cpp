#include "aabb.hpp"

#include <iostream>

Aabb::Aabb()
{
}

Aabb::Aabb(const Point3 &a, const Point3 &b)
{
  minimum = a;
  maximum = b;
}

Point3 Aabb::min() const
{
  return minimum;
}

Point3 Aabb::max() const
{
  return maximum;
}

bool Aabb::hit(const Ray &r, double t_min, double t_max) const
{
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

double Aabb::volume(void) const
{
  auto dx = fabs(minimum.x() - maximum.x());
  auto dy = fabs(minimum.y() - maximum.y());
  auto dz = fabs(minimum.z() - maximum.z());

  return dx * dy * dz;
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

Aabb intersection_box(const Aabb &box0, const Aabb &box1)
{
  Point3 small(
    fmax(box0.min().x(), box1.min().x()),
    fmax(box0.min().y(), box1.min().y()),
    fmax(box0.min().z(), box1.min().z())
  );

  Point3 big(
    fmin(box0.max().x(), box1.max().x()),
    fmin(box0.max().y(), box1.max().y()),
    fmin(box0.max().z(), box1.max().z())
  );

  // Any length < 0 -> no intersection
  for(int i = 0; i < 3; i++ ) {
    if(big[i] < small[i])
      return Aabb(Point3(0, 0, 0), Point3(0, 0, 0));
  }

  return Aabb(small, big);
}

std::ostream& operator<<(std::ostream &out, const Aabb &box)
{
  out << "Aabb(" << box.minimum << ", " << box.maximum << ")" << std::endl;
  return out;
}
