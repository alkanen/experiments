#ifndef CONSTANT_MEDIUM_HPP
#define CONSTANT_MEDIUM_HPP

#include "hittable.hpp"
#include "material.hpp"
#include "texture.hpp"

class ConstantMedium : public Hittable
{
public:
  ConstantMedium(Hittable *b, double d, Texture *a);
  ConstantMedium(Hittable *b, double d, Color c);

  virtual bool hit(
    const Ray &ray, double t_min, double t_max, HitRecord &rec
  ) const override;

  virtual bool bounding_box(double time0, double time1, Aabb& output_box) const override;

public:
  Hittable *boundary;
  double neg_inv_density;
  Material *phase_function;
};

#endif
