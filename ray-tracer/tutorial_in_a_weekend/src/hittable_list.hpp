#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include <memory>
#include <vector>

#include "hittable.hpp"

class HittableList : public Hittable {
public:
  HittableList() {}
  HittableList(Hittable *object) { add(object); }

  void clear() { objects.clear(); }
  void add(Hittable *object) { objects.push_back(object); }
  size_t size(void) { return objects.size(); }

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

bool HittableList::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const {
  HitRecord temp_rec;
  bool hit_anything = false;
  auto closest_so_far = t_max;

  for(const auto &object : objects) {
    if(object->hit(r, t_min, closest_so_far, temp_rec)) {
      hit_anything = true;
      closest_so_far = temp_rec.t;
      rec = temp_rec;
    }
  }

  return hit_anything;
}

bool HittableList::bounding_box(double time0, double time1, Aabb &output_box) const {
  if (objects.empty()) return false;

  Aabb temp_box;
  bool first_box = true;

  for(const auto& object : objects) {
    if(!object->bounding_box(time0, time1, temp_box))
      return false;
    output_box = first_box ? temp_box : surrounding_box(output_box, temp_box);
    first_box = false;
  }

  return true;
}

double HittableList::pdf_value(const Point3 &o, const Vec3 &v) const {
  auto weight = 1.0/objects.size();
  auto sum = 0.0;

  for (const auto& object : objects)
    sum += weight * object->pdf_value(o, v);

  return sum;
}

Vec3 HittableList::random(const Vec3 &o) const {
    auto int_size = static_cast<int>(objects.size());
    return objects[random_int(0, int_size-1)]->random(o);
}
#endif
