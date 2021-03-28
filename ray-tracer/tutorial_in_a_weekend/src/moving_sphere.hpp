#ifndef MOVING_SPHERE_HPP
#define MOVING_SPHERE_HPP

#include "hittable.hpp"
#include "material.hpp"
#include "vec3.hpp"
#include "ray.hpp"
#include "aabb.hpp"

class MovingSphere : public Hittable {
public:
  MovingSphere();
  MovingSphere(
    Point3 center0,
    Point3 center1,
    double time0,
    double time1,
    double radius,
    Material *material
  );
  virtual bool hit(
    const Ray &ray, double t_min, double t_max, HitRecord &rec
  ) const override;
  virtual bool bounding_box(
    double time0, double time1, Aabb &output_box
  ) const override;
  Point3 center(double time) const;

public:
  Point3 center0;
  Point3 center1;
  double time0;
  double time1;
  double radius;
  Material *material;

  Aabb cached_bb;
  double cached_t0;
  double cached_t1;
};

#endif
