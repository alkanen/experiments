#ifndef IMAGE_PLANE_H
#define IMAGE_PLANE_H

#include <iostream>

#include "hittable.hpp"
#include "vec3.hpp"

/*
Create a bounded plane defined by a point on the plane and two vectors
lying on it.  The point is the center of the rectangle and the two
vectors point to two orthogonal midpoints:

0,0               0,4
 +-----------------+
 |                 |
 |                 |
 |       p   v1    |
 |        x<------>|
 |        ^        |
 |     v2 |        |
 |        v        |
 +-----------------+
3,0               3,4

p  = (1.5, 2, 0)
v1 = (0, 2, 0)
v2 = (1.5, 0, 0)

The normal of the plane is in the direction of v1 x v2.
*/

double epsilon = 1e-6;

class ImagePlane : public Hittable {
public:
  ImagePlane() {}
  ImagePlane(Point3 center, Vec3 semi_a, Vec3 semi_b, Material *material) :
    center(center), material(material) {
    auto dot_prod = dot(semi_a, semi_b);
    if(fabs(dot_prod) >= epsilon) {
      throw "semi_a and semi_b must be orthogonal";
    }

    normal = cross(semi_a, semi_b);
    tl = center + semi_a - semi_b;
    tr = center + semi_a + semi_b;
    br = center - semi_a + semi_b;
    bl = center - semi_a - semi_b;

    std::cout << "From " << tl << " to " << tr << std::endl;
    std::cout << " and " << bl << " to " << br << std::endl;
  };

  virtual bool hit(
    const Ray &r, double t_min, double t_max, HitRecord &rec
  ) const override;

public:
  Point3 center;
  Material *material;
  Vec3 tl, tr, br, bl;
  Vec3 normal;

private:
  double triangle_ray_intersect(
    const Ray &ray, const Point3 &v0, const Point3 &v1, const Point3 &v2
  ) const;
};

bool ImagePlane::hit(const Ray &ray, double t_min, double t_max, HitRecord &rec) const
{
  //auto vec = center - ray.origin();
  //auto tmp = dot(vec, ray.direction());
  auto tmp = dot(normal, ray.direction());
  if(fabs(tmp) < epsilon) {
    std::cout << "tmp = " << normal << ".dot(" << ray.direction() << ")" << std::endl;
    std::cout << "tmp (" << tmp << ") is less than " << epsilon << std::endl;
    return false;
  }

  int sign = static_cast<int>(round(tmp / fabs(tmp)));
  double t = triangle_ray_intersect(ray, tl, tr, br);
  if(t == 0)
    t = triangle_ray_intersect(ray, tl, br, bl);

  t *= sign;

  if(t > t_min && t < t_max) {
    rec.t = t;
    rec.p = ray.at(t);
    rec.set_face_normal(ray, normal);
    rec.material = material;
    return true;
  }

  std::cout << "t is outside valid range (" << t_min << ", " << t_max << ")" << std::endl;
  return false;
}

double ImagePlane::triangle_ray_intersect(const Ray &ray, const Point3 &v0, const Point3 &v1, const Point3 &v2) const
{
  Vec3 edge1 = v1 - v0;
  Vec3 edge2 = v2 - v0;

  Vec3 h = cross(edge2, ray.origin());
  double a = dot(edge1, h);

  if(fabs(a) < epsilon) {
    // Line is parallell with plane of triangle
    // std::cout << "Line is parallell with plane" << std::endl;
    return 0.0;
  }

  double inv_a = 1.0 / a;
  Vec3 s = ray.origin() - v0;

  double u = inv_a * dot(s, h);
  if(u < 0 || u > 1) {
    // Out of bounds
    std::cout << "u is out of bounds" << std::endl;
    return 0.0;
  }

  Vec3 q = cross(s, edge1);
  double v = inv_a * dot(ray.direction(), q);
  if(v < 0 || u + v > 1) {
    // Out of bounds
    std::cout << "v is out of bounds" << std::endl;
    return 0.0;
  }

  auto retval = inv_a * dot(edge2, q);
  if(retval <= 0) {
    std::cout << "dot product is 0 or less" << std::endl;
  }
  return retval;
}

#endif
