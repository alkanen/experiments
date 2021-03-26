#include "ray.hpp"

#include <iostream>

Ray::Ray()
{
}

Ray::Ray(const Point3& origin, const Vec3& direction, double time)
  : orig(origin), dir(direction), tm(time)
{
}

Point3 Ray::origin() const
{
  return orig;
}

Vec3 Ray::direction() const
{
  return dir;
}

double Ray::time() const
{
  return tm;
}

Point3 Ray::at(double t) const
{
  return orig + t * dir;
}

std::ostream& operator<<(std::ostream &out, const Ray &r)
{
  return out << "Ray(" << r.origin() << ", " << r.direction() << ')';
}
