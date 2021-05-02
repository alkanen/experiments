#ifndef BVH_HPP
#define BVH_HPP

#include <algorithm>

#include "hittable.hpp"
#include "hittable_list.hpp"


class BvhNode;

std::ostream& operator<<(std::ostream &out, const BvhNode &node);
std::ostream& operator<<(std::ostream &out, const BvhNode *node);

class Bvh : public Hittable {
public:
  Bvh(const HittableList &list, double time0, double time1);
  Bvh(
    const std::vector<Hittable*> &src_objects,
    double time0, double time1
  );

  virtual bool hit(
    const Ray &r, double t_min, double t_max, HitRecord &rec
  ) const override;

  virtual bool bounding_box(
    double time0, double time1, Aabb &output_box
  ) const override;

public:
  Aabb box;
  std::vector<BvhNode*> nodes;
  bool empty;
};

bool box_compare(const Hittable *a, const Hittable *b, int axis);
bool box_x_compare(const Hittable *a, const Hittable *b);
bool box_y_compare(const Hittable *a, const Hittable *b);
bool box_z_compare(const Hittable *a, const Hittable *b);


#endif
