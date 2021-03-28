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
  , cached_t0(-1)
  , cached_t1(-1)
{
  bounding_box(time0, time1, cached_bb);;
  cached_t0 = time0;
  cached_t1 = time1;
}

Point3 MovingSphere::center(double time) const
{
  return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0);
}

bool MovingSphere::hit(const Ray& ray, double t_min, double t_max, HitRecord& rec) const
{
  //std::cerr << "MovingSphere::hit(" << ray << ", " << t_min << ", " << t_max << ", " << rec << ")" << std::endl;
  Aabb box;
  bounding_box(ray.time(), ray.time(), box);
  if(!box.hit(ray, t_min, t_max))
    return false;
  //std::cerr << "  hit bounding box" << std::endl;

  Vec3 oc = ray.origin() - center(ray.time());
  auto a = ray.direction().length_squared();
  auto half_b = dot(oc, ray.direction());
  auto c = oc.length_squared() - radius * radius;

  auto discriminant = half_b * half_b - a * c;
  if(discriminant < 0)
    return false;
  auto sqrtd = sqrt(discriminant);
  //std::cerr << "  hit discriminant" << std::endl;

  // Find the nearest root that lies in the acceptable range.
  auto root = (-half_b - sqrtd) / a;
  if (root < t_min || t_max < root) {
    root = (-half_b + sqrtd) / a;
    if (root < t_min || t_max < root)
      return false;
  }
  //std::cerr << "  root found" << std::endl;
  rec.t = root;
  rec.p = ray.at(rec.t);
  auto outward_normal = (rec.p - center(ray.time())) / radius;
  rec.set_face_normal(ray, outward_normal);
  rec.material = material;

  return true;
}

bool MovingSphere::bounding_box(double time0, double time1, Aabb &output_box) const
{
  //std::cerr << "MovingSphere::bounding_box(" << time0 << ", " << time1 << ")" << std::endl;
  if(
     time0 >= cached_t0 and time0 <= cached_t1
     and
     time1 >= cached_t0 and time1 <= cached_t1
  ) {
  // if(cached_t0 <= time0 && cached_t1 <= time1) {
    //std::cerr << "  cache hit: " << cached_bb << std::endl;
    output_box = cached_bb;
    return true;
  }

  Aabb box0(
    center(time0) - Vec3(radius, radius, radius),
    center(time0) + Vec3(radius, radius, radius)
  );
  //std::cerr << "  box0: " << box0 << std::endl;
  Aabb box1(
    center(time1) - Vec3(radius, radius, radius),
    center(time1) + Vec3(radius, radius, radius)
  );
  //std::cerr << "  box1: " << box1 << std::endl;
  output_box = surrounding_box(box0, box1);
  //std::cerr << "    -> " << output_box << std::endl;
  return true;
}
