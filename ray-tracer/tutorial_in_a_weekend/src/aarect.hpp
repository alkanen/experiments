#ifndef AARECT_HPP
#define AARECT_HPP

#include "vec3.hpp"
#include "ray.hpp"
#include "hittable.hpp"

class XyRect : public Hittable {
public:
  XyRect();
  XyRect(double x0, double x1, double y0, double y1, double k, Material *mat);

  virtual bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;

  virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override;

public:
  double x0, x1, y0, y1, k;
  Material *material;
};


class XzRect : public Hittable {
public:
  XzRect();
  XzRect(double x0, double x1, double z0, double z1, double k, Material *mat);

  virtual bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;

  virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override;

  virtual double pdf_value(const Point3 &o, const Vec3 &v) const;

  virtual Vec3 random(const Vec3 &o) const;

public:
  double x0, x1, z0, z1, k;
  Material *material;
};


class YzRect : public Hittable {
public:
  YzRect();

  YzRect(double y0, double y1, double z0, double z1, double k, Material *mat);

  virtual bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;

  virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override;

public:
  double y0, y1, z0, z1, k;
  Material *material;
};

#endif
