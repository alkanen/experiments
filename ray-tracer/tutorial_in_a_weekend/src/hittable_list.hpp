#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include <memory>
#include <vector>

#include "hittable.hpp"

#include "aabb.hpp"

class HittableList : public Hittable {
public:
  HittableList();
  HittableList(Hittable *object);

  void clear();
  void add(Hittable *object);
  size_t size(void);

  virtual bool hit(
    const Ray &r, double t_min, double t_max, HitRecord &rec
  ) const override;
  virtual bool bounding_box(
    double time0, double time1, Aabb &output_box
  ) const override;
  virtual double pdf_value(const Point3 &o, const Vec3 &v) const override;
  virtual Vec3 random(const Vec3 &o) const override;

public:
  std::vector<Hittable*> objects;
};

#endif
