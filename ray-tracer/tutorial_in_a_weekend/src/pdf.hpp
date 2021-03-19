#ifndef PDF_HPP
#define PDF_HPP

#include "vec3.hpp"
#include "onb.hpp"
#include "hittable.hpp"

class Pdf {
public:
  virtual ~Pdf() {}

  virtual double value(const Vec3 &direction) const = 0;
  virtual Vec3 generate() const = 0;
};

class CosinePdf: public Pdf {
public:
  CosinePdf(const Vec3& w) { uvw.build_from_w(w); }

  virtual double value(const Vec3& direction) const override {
    auto cosine = dot(unit_vector(direction), uvw.w());
    return (cosine <= 0) ? 0 : cosine / pi;
  }

  virtual Vec3 generate() const override {
    return uvw.local(random_cosine_direction());
  }

public:
  Onb uvw;
};

class HittablePdf: public Pdf {
public:
  HittablePdf(Hittable *p, const Point3 &origin): ptr(p), o(origin) {}

  virtual double value(const Vec3& direction) const override {
    auto retval = ptr->pdf_value(o, direction);
    // std::cout << "HittablePdf::value(" << direction << ") -> " << retval << std::endl;
    return retval;
  }

  virtual Vec3 generate() const override {
    auto retval =  ptr->random(o);
    // std::cout << "HittablePdf::generate() -> " << retval << std::endl;
    return retval;
  }

public:
  Hittable *ptr;
  Point3 o;
};

class MixturePdf : public Pdf {
public:
  MixturePdf(std::shared_ptr<Pdf> p0, std::shared_ptr<Pdf> p1)
  {
    p[0] = p0;
    p[1] = p1;
  }

  virtual double value(const Vec3 &direction) const override
  {
    return 0.5 * p[0]->value(direction) + 0.5 *p[1]->value(direction);
  }

  virtual Vec3 generate() const override
  {
    if (random_double() < 0.5)
      return p[0]->generate();
    else
      return p[1]->generate();
  }

public:
  std::shared_ptr<Pdf> p[2];
};

inline Vec3 random_to_sphere(double radius, double distance_squared)
{
  auto r1 = random_double();
  auto r2 = random_double();
  auto z = 1 + r2 * (sqrt(1 - radius * radius / distance_squared) - 1);

  auto phi = 2 * pi * r1;
  auto x = cos(phi)*sqrt(1 - z * z);
  auto y = sin(phi)*sqrt(1 - z * z);

  return Vec3(x, y, z);
}
#endif
