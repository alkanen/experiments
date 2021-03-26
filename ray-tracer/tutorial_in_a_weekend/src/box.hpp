#ifndef BOX_HPP
#define BOX_HPP

#include "vec3.hpp"
#include "material.hpp"
#include "ray.hpp"
#include "aarect.hpp"
#include "hittable.hpp"
#include "hittable_list.hpp"

class Box : public Hittable
{
public:
  Box();
  Box(const Point3 &p0, const Point3 &p1, Material *material);

  virtual bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;
  virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override;

public:
  Point3 box_min;
  Point3 box_max;
  HittableList sides;
};

#endif
