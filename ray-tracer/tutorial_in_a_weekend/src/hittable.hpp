#ifndef HITTABLE_H
#define HITTABLE_H

#include <iostream>

#include "rtweekend.hpp"

#include "aabb.hpp"

class Material;

class HitRecord {
public:
  HitRecord() : u(-1), v(-1), material(nullptr) {}

  inline void set_face_normal(const Ray &r, const Vec3 &outward_normal)
  {
    front_face = dot(r.direction(), outward_normal) < 0;
    normal = front_face ? outward_normal : -outward_normal;
  }

public:
  Point3 p;
  Vec3 normal;
  double t;
  bool front_face;
  // For texture mapping
  double u, v;
  Material *material;
};

inline std::ostream& operator<<(std::ostream &out, const HitRecord &rec)
{
  return out << "HitRecord(" << rec.p << ", " << rec.normal << ", " << rec.material << ")";
}


class Hittable {
public:
  virtual bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const = 0;
  virtual bool bounding_box(double time0, double time1, Aabb &output_box) const = 0;
};

class Translate : public Hittable
{
public:
  Translate(Hittable *p, const Vec3 &displacement)
    : ptr(p), offset(displacement) {}

  virtual bool hit(
    const Ray &ray, double t_min, double t_max, HitRecord &rec
  ) const override;

  virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override;

public:
  Hittable *ptr;
  Vec3 offset;
};

bool Translate::hit(const Ray &ray, double t_min, double t_max, HitRecord &rec) const
{
  Ray moved_ray(ray.origin() - offset, ray.direction(), ray.time());

  if(!ptr->hit(moved_ray, t_min, t_max, rec))
    return false;

  rec.p += offset;
  rec.set_face_normal(moved_ray, rec.normal);

  return true;
}

bool Translate::bounding_box(double time0, double time1, Aabb &output_box) const
{
  if(ptr->bounding_box(time0, time1, output_box))
    return false;

  output_box = Aabb(
    output_box.min() + offset,
    output_box.max() + offset
  );

  return true;
}

class RotateY : public Hittable
{
public:
  RotateY(Hittable *p, double angle);

  virtual bool hit(
    const Ray &ray, double t_min, double t_max, HitRecord &rec
  ) const override;

  virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override
  {
    output_box = bbox;
    return hasbox;
  }

public:
  Hittable *ptr;
  double sin_theta;
  double cos_theta;
  bool hasbox;
  Aabb bbox;
};

RotateY::RotateY(Hittable *p, double angle) : ptr(p)
{
  auto radians = degrees_to_radians(angle);
  sin_theta = sin(radians);
  cos_theta = cos(radians);
  hasbox = ptr->bounding_box(0, 1, bbox);

  Point3 min( infinity,  infinity,  infinity);
  Point3 max(-infinity, -infinity, -infinity);

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 2; k++) {
        auto x = i * bbox.max().x() + (1-i)*bbox.min().x();
        auto y = j * bbox.max().y() + (1-j)*bbox.min().y();
        auto z = k * bbox.max().z() + (1-k)*bbox.min().z();

        auto newx =  cos_theta*x + sin_theta*z;
        auto newz = -sin_theta*x + cos_theta*z;

        Vec3 tester(newx, y, newz);

        for (int c = 0; c < 3; c++) {
          min[c] = fmin(min[c], tester[c]);
          max[c] = fmax(max[c], tester[c]);
        }
      }
    }
  }

  bbox = Aabb(min, max);
}

bool RotateY::hit(const Ray &ray, double t_min, double t_max, HitRecord &rec) const {
  auto origin = ray.origin();
  auto direction = ray.direction();

  origin[0] = cos_theta * ray.origin()[0] - sin_theta * ray.origin()[2];
  origin[2] = sin_theta * ray.origin()[0] + cos_theta * ray.origin()[2];

  direction[0] = cos_theta * ray.direction()[0] - sin_theta * ray.direction()[2];
  direction[2] = sin_theta * ray.direction()[0] + cos_theta * ray.direction()[2];

  Ray rotated_ray(origin, direction, ray.time());

  if(!ptr->hit(rotated_ray, t_min, t_max, rec))
    return false;

  auto p = rec.p;
  auto normal = rec.normal;

  p[0] =  cos_theta * rec.p[0] + sin_theta * rec.p[2];
  p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];

  normal[0] =  cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
  normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];

  rec.p = p;
  rec.set_face_normal(rotated_ray, normal);

  return true;
}

class FlipFace: public Hittable {
public:
  FlipFace(Hittable *p) : ptr(p) {}

  virtual bool hit(
    const Ray &r, double t_min, double t_max, HitRecord &rec
  ) const override {
    if (!ptr->hit(r, t_min, t_max, rec))
      return false;

    rec.front_face = !rec.front_face;
    return true;
  }

  virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override {
    return ptr->bounding_box(time0, time1, output_box);
  }

public:
  Hittable *ptr;
};

#endif
