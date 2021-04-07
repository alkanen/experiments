#include "bvh.hpp"

#include <string>

#include "hittable_list.hpp"
#include "hittable.hpp"
#include "aabb.hpp"
#include "ray.hpp"

BvhNode::BvhNode()
{
}

BvhNode::BvhNode(Hittable *left_obj, Hittable *right_obj, double time0, double time1, bool leaf)
  : left(left_obj)
  , right(right_obj)
  , leaf(leaf)
{
  Aabb box_left, box_right;

  left->bounding_box(time0, time1, box_left);
  right->bounding_box(time0, time1, box_right);

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
  bool hit_right = false;
  if(left != right)
    hit_right = right->hit(r, t_min, hit_left ? rec.t : t_max, rec);

  return hit_left || hit_right;
}

std::ostream& operator<<(std::ostream &out, const BvhNode &node)
{
  out << &node;
  return out;
}

std::ostream& operator<<(std::ostream &out, const BvhNode *node)
{
  if(node->leaf) {
    out << "{\"box\": " << node->box << "}";
  } else {
    out << "{\"box\": "
	<< node->box
	<< ", \"left\": "
	<< dynamic_cast<BvhNode*>(node->left)
	<< ", \"right\": "
	<< dynamic_cast<BvhNode*>(node->right)
	<< "}";
  }

  return out;
}

Bvh::Bvh(const HittableList &list, double time0, double time1)
  : Bvh(list.objects, time0, time1)
{
}

Bvh::Bvh(
  const std::vector<Hittable*> &src_objects,
  double time0, double time1
)
{
  size_t num_objs = src_objects.size();
  auto objects = src_objects;
  std::vector<Hittable*> leaves;

  while(num_objs > 0) {
    size_t min_i = num_objs, min_j = num_objs;
    if(num_objs == 1) {
      min_i = 0;
      min_j = 0;
    } else {
      double min_value = 1e20;
      for(size_t i = 0; i < num_objs; i++) {
        for(size_t j = i+1; j < num_objs; j++) {
          Aabb i_box, j_box;
          objects[i]->bounding_box(time0, time1, i_box);
          objects[j]->bounding_box(time0, time1, j_box);

          double volume = surrounding_box(i_box, j_box).volume();

          if(volume < min_value) {
            min_value = volume;
            min_i = i;
            min_j = j;
          }
        }
      }
    }

    auto obj_i = objects[min_i];
    auto obj_j = objects[min_j];

    if(num_objs > 1) {
      objects.erase(objects.begin() + min_j);
      objects.erase(objects.begin() + min_i);
    } else {
      objects.clear();
    }

    leaves.push_back(new BvhNode(obj_i, obj_j, time0, time1, true));
    leaves.back()->setName(
      "{\"left\": \"" + obj_i->name + "\", \"right\": \"" + obj_j->name + "\"}"
    );

    num_objs = objects.size();
  }

  objects = leaves;
  leaves.clear();

  num_objs = objects.size();

  int ctr = 1;
  while(num_objs > 1) {
    while(num_objs > 0) {
      size_t min_i = num_objs, min_j = num_objs;
      if(num_objs == 1) {
        min_i = 0;
        min_j = 0;
      } else {
        double min_value = 1e20;
        for(size_t i = 0; i < num_objs; i++) {
          for(size_t j = i+1; j < num_objs; j++) {
            Aabb i_box, j_box;
            objects[i]->bounding_box(time0, time1, i_box);
            objects[j]->bounding_box(time0, time1, j_box);

            double volume = surrounding_box(i_box, j_box).volume();

            if(volume) {
              min_value = volume;
              min_i = i;
              min_j = j;
            }
          }
        }
      }

      auto obj_i = objects[min_i];
      auto obj_j = objects[min_j];

      if(num_objs > 1) {
        objects.erase(objects.begin() + min_j);
        objects.erase(objects.begin() + min_i);
      } else {
        objects.clear();
      }

      leaves.push_back(new BvhNode(obj_i, obj_j, time0, time1, false));
      leaves.back()->setName(
        "{\"left\":" + obj_i->name + ", \"right\":" + obj_j->name + "}"
      );

      num_objs = objects.size();
    }

    objects = leaves;
    leaves.clear();
    num_objs = objects.size();
  }

  // const size_t max_idx = 2 << ctr;
  objects[0]->bounding_box(time0, time1, box);
  //std::cerr << dynamic_cast<BvhNode*>(objects[0]) << std::endl;

  nodes.push_back(dynamic_cast<BvhNode*>(objects[0]));
}

bool Bvh::hit(
  const Ray &r, double t_min, double t_max, HitRecord &rec
) const
{
  if(!box.hit(r, t_min, t_max))
    return false;

  return nodes[0]->hit(r, t_min, t_max, rec);
}

bool Bvh::bounding_box(
  double time0, double time1, Aabb &output_box
) const
{
  output_box = box;
  return true;
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
