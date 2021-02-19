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

  virtual bool hit(
    const Ray &r, double t_min, double t_max, HitRecord &rec
  ) const override;

public:
  std::vector<Hittable*> objects;
};

bool HittableList::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const {
  HitRecord temp_rec;
  bool hit_anything = false;
  auto closest_so_far = t_max;

  for (const auto& object : objects) {
    if (object->hit(r, t_min, closest_so_far, temp_rec)) {
      hit_anything = true;
      closest_so_far = temp_rec.t;
      rec = temp_rec;
    }
  }

  return hit_anything;
}

#endif
