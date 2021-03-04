#ifndef BOX_HPP
#define BOX_HPP

#include "rtweekend.hpp"

#include "aarect.hpp"
#include "hittable_list.hpp"

class Box : public Hittable
{
public:
  Box() {}
  Box(const Point3 &p0, const Point3 &p1, Material *material);

  virtual bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;

  virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override {
    output_box = Aabb(box_min, box_max);
    return true;
  }

public:
  Point3 box_min;
  Point3 box_max;
  HittableList sides;
};

Box::Box(const Point3 &p0, const Point3 &p1, Material *material)
{
  box_min = p0;
  box_max = p1;

  sides.add(new XyRect(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), material));
  sides.add(new XyRect(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), material));

  sides.add(new XzRect(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), material));
  sides.add(new XzRect(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), material));

  sides.add(new YzRect(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), material));
  sides.add(new YzRect(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), material));
}

bool Box::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const {
  return sides.hit(r, t_min, t_max, rec);
}

#endif
