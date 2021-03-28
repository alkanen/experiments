#include "bvh.hpp"

#include "hittable_list.hpp"
#include "hittable.hpp"
#include "aabb.hpp"
#include "ray.hpp"

BvhNode::BvhNode()
{
}

BvhNode::BvhNode(const HittableList &list, double time0, double time1)
  : BvhNode(list.objects, time0, time1)
{
}

BvhNode::BvhNode(Hittable *left_obj, Hittable *right_obj, double time0, double time1)
  : left(left_obj)
  , right(right_obj)
{
  Aabb box_left, box_right;

  left->bounding_box(time0, time1, box_left);
  right->bounding_box(time0, time1, box_right);

  box = surrounding_box(box_left, box_right);
}

BvhNode::BvhNode(
  const std::vector<Hittable*> &src_objects,
  double time0, double time1
)
{
  auto objects = src_objects; // Create a modifiable array of the source scene objects
  size_t num_objs = src_objects.size();
  double volumes[num_objs][num_objs];

  while(num_objs > 2) {
    double min_value = 1e20;
    size_t min_i = num_objs, min_j = num_objs;
    for(size_t i = 0; i < num_objs; i++) {
      volumes[i][i] = 0;

      for(size_t j = i+1; j < num_objs; j++) {
        Aabb i_box, j_box;
        objects[i]->bounding_box(time0, time1, i_box);
        objects[j]->bounding_box(time0, time1, j_box);
        volumes[i][j] = surrounding_box(i_box, j_box).volume();

        if(volumes[i][j] < min_value) {
          min_value = volumes[i][j];
          min_i = i;
          min_j = j;
        }
      }
    }

    auto obj_i = objects[min_i];
    auto obj_j = objects[min_j];

    objects.erase(objects.begin() + min_j);
    objects.erase(objects.begin() + min_i);

    objects.push_back(new BvhNode(obj_i, obj_j, time0, time1));

    num_objs = objects.size();
  }

  if(num_objs == 2) {
    left = objects[0];
    right = objects[1];
  } else {
    left = objects[0];
    right = objects[1];
  }

  Aabb box_left, box_right;
  if(
    !left->bounding_box(time0, time1, box_left)
    || !right->bounding_box(time0, time1, box_right)
  )
    std::cerr << "No bounding box in bvh_node constructor.\n";

  box = surrounding_box(box_left, box_right);
}

bool BvhNode::bounding_box(double time0, double time1, Aabb &output_box) const
{
  output_box = box;
  return true;
}

bool BvhNode::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const
{
  if(!box.hit(r, t_min, t_max))
    return false;

  bool hit_left = left->hit(r, t_min, t_max, rec);
  bool hit_right = right->hit(r, t_min, hit_left ? rec.t : t_max, rec);

  return hit_left || hit_right;
}

inline bool box_compare(const Hittable *a, const Hittable *b, int axis)
{
  Aabb box_a;
  Aabb box_b;

  if(!a->bounding_box(0,0, box_a) || !b->bounding_box(0,0, box_b))
    std::cerr << "No bounding box in bvh_node constructor.\n";

  return box_a.min().e[axis] < box_b.min().e[axis];
}


bool box_x_compare(const Hittable *a, const Hittable *b)
{
  return box_compare(a, b, 0);
}

bool box_y_compare(const Hittable *a, const Hittable *b)
{
  return box_compare(a, b, 1);
}

bool box_z_compare(const Hittable *a, const Hittable *b)
{
  return box_compare(a, b, 2);
}
