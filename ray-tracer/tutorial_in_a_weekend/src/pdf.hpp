#ifndef PDF_HPP
#define PDF_HPP

#include <memory>

#include "vec3.hpp"
#include "onb.hpp"
#include "hittable.hpp"

class Pdf {
public:
  virtual ~Pdf();
  virtual double value(const Vec3 &direction) const = 0;
  virtual Vec3 generate() const = 0;
};

class CosinePdf: public Pdf {
public:
  CosinePdf(const Vec3& w);

  virtual double value(const Vec3& direction) const override;
  virtual Vec3 generate() const override;

public:
  Onb uvw;
};

class HittablePdf: public Pdf {
public:
  HittablePdf(Hittable *p, const Point3 &origin);

  virtual double value(const Vec3& direction) const override;
  virtual Vec3 generate() const override;

public:
  Hittable *ptr;
  Point3 o;
};

class MixturePdf : public Pdf {
public:
  MixturePdf(std::shared_ptr<Pdf> p0, std::shared_ptr<Pdf> p1);

  virtual double value(const Vec3 &direction) const override;
  virtual Vec3 generate() const override;

public:
  std::shared_ptr<Pdf> p[2];
};

Vec3 random_to_sphere(double radius, double distance_squared);
#endif
