#ifndef HITTABLE_H
#define HITTABLE_H

#include <iostream>

#include "rtweekend.hpp"
#include "vec3.hpp"
#include "ray.hpp"
#include "aabb.hpp"

class Material;

class HitRecord {
public:
  HitRecord();
  void set_face_normal(const Ray &r, const Vec3 &outward_normal);

public:
  Point3 p;
  Vec3 normal;
  double t;
  bool front_face;
  // For texture mapping
  double u, v;
  Material *material;
};

std::ostream& operator<<(std::ostream &out, const HitRecord &rec);


class Hittable {
public:
  void setName(std::string n);
  virtual bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const = 0;
  virtual bool bounding_box(double time0, double time1, Aabb &output_box) const = 0;
  virtual double pdf_value(const Point3 &o, const Vec3 &v) const;
  virtual Vec3 random(const Vec3 &o) const;

public:
  std::string name;
};

class Translate : public Hittable
{
public:
  Translate(Hittable *p, const Vec3 &displacement);

  virtual bool hit(
    const Ray &ray, double t_min, double t_max, HitRecord &rec
  ) const override;

  virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override;

public:
  Hittable *ptr;
  Vec3 offset;
};

class RotateY : public Hittable
{
public:
  RotateY(Hittable *p, double angle);

  virtual bool hit(
    const Ray &ray, double t_min, double t_max, HitRecord &rec
  ) const override;

  virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override;

public:
  Hittable *ptr;
  double sin_theta;
  double cos_theta;
  bool hasbox;
  Aabb bbox;
};

class FlipFace: public Hittable {
public:
  FlipFace(Hittable *p);

  virtual bool hit(
    const Ray &r, double t_min, double t_max, HitRecord &rec
  ) const override;

  virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override;

public:
  Hittable *ptr;
};

#endif
