#ifndef AARECT_HPP
#define AARECT_HPP

#include "rtweekend.hpp"

#include "hittable.hpp"

class XyRect : public Hittable {
public:
  XyRect() {}

  XyRect(
    double x0, double x1, double y0, double y1, double k, Material *mat
  )
    : x0(x0), x1(x1), y0(y0), y1(y1), k(k), material(mat) {};

  virtual bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;

  virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override
  {
    // The bounding box must have non-zero width in each dimension, so pad the Z
    // dimension a small amount.
    output_box = Aabb(Point3(x0, y0, k-0.0001), Point3(x1, y1, k+0.0001));
    return true;
  }

public:
  double x0, x1, y0, y1, k;
  Material *material;
};

bool XyRect::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const
{
  auto t = (k-r.origin().z()) / r.direction().z();
  if(t < t_min || t > t_max)
    return false;

  auto x = r.origin().x() + t*r.direction().x();
  auto y = r.origin().y() + t*r.direction().y();
  if(x < x0 || x > x1 || y < y0 || y > y1)
    return false;

  rec.u = (x - x0) / (x1 - x0);
  rec.v = (y - y0) / (y1 - y0);
  rec.t = t;

  auto outward_normal = Vec3(0, 0, 1);
  rec.set_face_normal(r, outward_normal);
  rec.material = material;
  rec.p = r.at(t);

  return true;
}

class XzRect : public Hittable {
public:
  XzRect() {}

  XzRect(
    double x0, double x1, double z0, double z1, double k, Material *mat
  )
    : x0(x0), x1(x1), z0(z0), z1(z1), k(k), material(mat) {};

  virtual bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;

  virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override
  {
    // The bounding box must have non-zero width in each dimension, so pad the Z
    // dimension a small amount.
    output_box = Aabb(Point3(x0, k-0.0001, z0), Point3(x1, k+0.0001, z1));
    return true;
  }

  virtual double pdf_value(const Point3 &o, const Vec3 &v) const
  {
    HitRecord rec;
    if(!this->hit(Ray(o, v), 0.001, infinity, rec)) {
      // std::cout << "XzRect::pdf_value(" << o << ", " << v << ") -> 0 (not hit)" << std::endl;
      return 0;
    }

    auto area = (x1 - x0) * (z1 - z0);
    auto distance_squared = rec.t * rec.t * v.length_squared();
    auto cosine = fabs(dot(v, rec.normal) / v.length());
    auto retval = distance_squared / (cosine * area);
    // std::cout << "XzRect::pdf_value(" << o << ", " << v << ") -> " << retval << std::endl;
    return retval;
  }

  virtual Vec3 random(const Vec3 &o) const {
    auto random_point = Point3(random_double(x0, x1), k, random_double(z0, z1));
    return random_point - o;
  }

public:
  double x0, x1, z0, z1, k;
  Material *material;
};

bool XzRect::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const
{
  auto t = (k-r.origin().y()) / r.direction().y();
  if(t < t_min || t > t_max)
    return false;

  auto x = r.origin().x() + t*r.direction().x();
  auto z = r.origin().z() + t*r.direction().z();
  if(x < x0 || x > x1 || z < z0 || z > z1)
    return false;

  rec.u = (x - x0) / (x1 - x0);
  rec.v = (z - z0) / (z1 - z0);
  rec.t = t;

  auto outward_normal = Vec3(0, 1, 0);
  rec.set_face_normal(r, outward_normal);
  rec.material = material;
  rec.p = r.at(t);

  return true;
}

class YzRect : public Hittable {
public:
  YzRect() {}

  YzRect(
    double y0, double y1, double z0, double z1, double k, Material *mat
  )
    : y0(y0), y1(y1), z0(z0), z1(z1), k(k), material(mat) {};

  virtual bool hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const override;

  virtual bool bounding_box(double time0, double time1, Aabb &output_box) const override
  {
    // The bounding box must have non-zero width in each dimension, so pad the Z
    // dimension a small amount.
    output_box = Aabb(Point3(k-0.0001, y0, z0), Point3(k+0.0001, y1, z1));
    return true;
  }

public:
  double y0, y1, z0, z1, k;
  Material *material;
};

bool YzRect::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const
{
  auto t = (k-r.origin().x()) / r.direction().x();
  if(t < t_min || t > t_max)
    return false;

  auto y = r.origin().y() + t*r.direction().y();
  auto z = r.origin().z() + t*r.direction().z();
  if(y < y0 || y > y1 || z < z0 || z > z1)
    return false;

  rec.u = (y - y0) / (y1 - y0);
  rec.v = (z - z0) / (z1 - z0);
  rec.t = t;

  auto outward_normal = Vec3(1, 0, 0);
  rec.set_face_normal(r, outward_normal);
  rec.material = material;
  rec.p = r.at(t);

  return true;
}

#endif
