#ifndef CONSTANT_MEDIUM_HPP
#define CONSTANT_MEDIUM_HPP

#include "rtweekend.hpp"

#include "hittable.hpp"
#include "material.hpp"
#include "texture.hpp"

class ConstantMedium : public Hittable
{
public:
  ConstantMedium(Hittable *b, double d, Texture *a)
    : boundary(b),
      neg_inv_density(-1/d),
      phase_function(new Isotropic(a))
  {}

  ConstantMedium(Hittable *b, double d, Color c)
    : boundary(b),
      neg_inv_density(-1/d),
      phase_function(new Isotropic(c))
  {}

  virtual bool hit(
    const Ray &ray, double t_min, double t_max, HitRecord &rec
  ) const override;

  virtual bool bounding_box(double time0, double time1, Aabb& output_box) const override
  {
    return boundary->bounding_box(time0, time1, output_box);
  }

public:
  Hittable *boundary;
  double neg_inv_density;
  Material *phase_function;
};

bool ConstantMedium::hit(const Ray &ray, double t_min, double t_max, HitRecord &rec) const
{
  // Print occasional samples when debugging. To enable, set enableDebug true.
  const bool enableDebug = false;
  const bool debugging = enableDebug && random_double() < 0.00001;

  HitRecord rec1, rec2;

  if(!boundary->hit(ray, -infinity, infinity, rec1))
    return false;

  if(!boundary->hit(ray, rec1.t+0.0001, infinity, rec2))
    return false;

  if(debugging) std::cerr << "\nt_min=" << rec1.t << ", t_max=" << rec2.t << '\n';

  if(rec1.t < t_min) rec1.t = t_min;
  if(rec2.t > t_max) rec2.t = t_max;

  if(rec1.t >= rec2.t)
    return false;

  if(rec1.t < 0)
    rec1.t = 0;

  const auto ray_length = ray.direction().length();
  const auto distance_inside_boundary = (rec2.t - rec1.t) * ray_length;
  const auto hit_distance = neg_inv_density * log(random_double());

  if(hit_distance > distance_inside_boundary)
    return false;

  rec.t = rec1.t + hit_distance / ray_length;
  rec.p = ray.at(rec.t);

  if(debugging) {
    std::cerr << "hit_distance = " <<  hit_distance << '\n'
              << "rec.t = " <<  rec.t << '\n'
              << "rec.p = " <<  rec.p << '\n';
  }

  rec.normal = Vec3(1, 0, 0);  // arbitrary
  rec.front_face = true;       // also arbitrary
  rec.material = phase_function;

  return true;
}

#endif
