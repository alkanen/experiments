#include "moving_sphere.hpp"

MovingSphere::MovingSphere()
{
}

MovingSphere::MovingSphere(
  Point3 center0,
  Point3 center1,
  double time0,
  double time1,
  double radius,
  Material *material
)
  : center0(center0)
  , center1(center1)
  , time0(time0)
  , time1(time1)
  , radius(radius)
  , material(material)
{
}

Point3 MovingSphere::center(double time) const
{
  return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
}

bool MovingSphere::hit(const Ray& ray, double t_min, double t_max, HitRecord& rec) const
{
  Vec3 oc = ray.origin() - center(ray.time());
  auto a = ray.direction().length_squared();
  auto half_b = dot(oc, ray.direction());
  auto c = oc.length_squared() - radius * radius;

  auto discriminant = half_b * half_b - a * c;
  if(discriminant < 0)
    return false;
  auto sqrtd = sqrt(discriminant);

  // Find the nearest root that lies in the acceptable range.
  auto root = (-half_b - sqrtd) / a;
  if (root < t_min || t_max < root) {
    root = (-half_b + sqrtd) / a;
    if (root < t_min || t_max < root)
      return false;
  }

  rec.t = root;
  rec.p = ray.at(rec.t);
  auto outward_normal = (rec.p - center(ray.time())) / radius;
  rec.set_face_normal(ray, outward_normal);
  rec.material = material;

  return true;
}

bool MovingSphere::bounding_box(double time0, double time1, Aabb &output_box) const
{
  Aabb box0(
    center(time0) - Vec3(radius, radius, radius),
    center(time1) + Vec3(radius, radius, radius)
  );
  Aabb box1(
    center(time1) - Vec3(radius, radius, radius),
    center(time1) + Vec3(radius, radius, radius)
  );
  output_box = surrounding_box(box0, box1);
  return true;
}
