#ifndef HITTABLE_H
#define HITTABLE_H

#include "rtweekend.hpp"

class Material;

class HitRecord {
public:
  HitRecord() : u(-1), v(-1) {}

  inline void set_face_normal(const Ray &r, const Vec3 &outward_normal)
  {
    front_face = dot(r.direction(), outward_normal) < 0;
    normal = front_face ? outward_normal : -outward_normal;
  }

public:
  Point3 p;
  Vec3 normal;
  Material *material;
  double t;
  bool front_face;
  // For texture mapping
  double u, v;
};

class Hittable {
public:
  virtual bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const = 0;
};

#endif
