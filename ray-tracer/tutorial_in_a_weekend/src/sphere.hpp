#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.hpp"
#include "vec3.hpp"
#include "onb.hpp"
#include "pdf.hpp"

class Sphere : public Hittable {
public:
  Sphere();
  Sphere(Point3 cen, double r, Material *m);

  virtual bool hit(
    const Ray &r, double t_min, double t_max, HitRecord &rec
  ) const override;
  virtual bool bounding_box(
    double time0, double time1, Aabb &output_box
  ) const override;
  virtual double pdf_value(const Point3& o, const Vec3& v) const override;
  virtual Vec3 random(const Point3& o) const override;

public:
  Point3 center;
  double radius;
  Material *material;

private:
  static void get_sphere_uv(const Point3 &p, double &u, double &v);
};

#endif
