#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.hpp"
#include "vec3.hpp"
#include "onb.hpp"
#include "pdf.hpp"

class Sphere : public Hittable {
public:
  Sphere() {}
  Sphere(Point3 cen, double r, Material *m) :
    center(cen), radius(r), material(m) {};

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
  static void get_sphere_uv(const Point3 &p, double &u, double &v)
  {
    // p: a given point on the sphere of radius one, centered at the origin.
    // u: returned value [0,1] of angle around the Y axis from X=-1.
    // v: returned value [0,1] of angle from Y=-1 to Y=+1.
    //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
    //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
    //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>

    auto theta = acos(-p.y());
    auto phi = atan2(-p.z(), p.x()) + pi;

    u = phi / (2*pi);
    v = theta / pi;
  }
};

bool Sphere::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const
{
  Vec3 oc = r.origin() - center;
  auto a = r.direction().length_squared();
  auto half_b = dot(oc, r.direction());
  auto c = oc.length_squared() - radius*radius;

  auto discriminant = half_b*half_b - a*c;
  if (discriminant < 0)
    return false;
  auto sqrtd = sqrt(discriminant);

  // Find the nearest root that lies in the acceptable range.
  auto root = (-half_b - sqrtd) / a;
  if(root < t_min || t_max < root) {
    root = (-half_b + sqrtd) / a;
    if(root < t_min || t_max < root)
      return false;
  }

  rec.t = root;
  rec.p = r.at(rec.t);
  Vec3 outward_normal = (rec.p - center) / radius;
  rec.set_face_normal(r, outward_normal);
  get_sphere_uv(outward_normal, rec.u, rec.v);
  rec.material = material;

  return true;
}

bool Sphere::bounding_box(double time0, double time1, Aabb &output_box) const
{
  output_box = Aabb(
    center - Vec3(radius, radius, radius),
    center + Vec3(radius, radius, radius)
  );
  return true;
}

double Sphere::pdf_value(const Point3& o, const Vec3& v) const {
  HitRecord rec;
  if(!this->hit(Ray(o, v), 0.001, infinity, rec))
    return 0;

  auto cos_theta_max = sqrt(1 - radius * radius / (center - o).length_squared());
  auto solid_angle = 2 * pi * (1 - cos_theta_max);

  return  1 / solid_angle;
}

Vec3 Sphere::random(const Point3& o) const {
    Vec3 direction = center - o;
    auto distance_squared = direction.length_squared();
    Onb uvw;
    uvw.build_from_w(direction);
    return uvw.local(random_to_sphere(radius, distance_squared));
}

#endif
