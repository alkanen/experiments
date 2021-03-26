#include "pdf.hpp"

#include <memory>

#include "vec3.hpp"
#include "hittable.hpp"

Pdf::~Pdf()
{
}

CosinePdf::CosinePdf(const Vec3& w)
{
  uvw.build_from_w(w);
}

double CosinePdf::value(const Vec3& direction) const
{
  auto cosine = dot(unit_vector(direction), uvw.w());
  return (cosine <= 0) ? 0 : cosine / pi;
}

Vec3 CosinePdf::generate() const
{
  return uvw.local(random_cosine_direction());
}

HittablePdf::HittablePdf(Hittable *p, const Point3 &origin)
  : ptr(p)
  , o(origin)
{
}

double HittablePdf::value(const Vec3& direction) const
{
  auto retval = ptr->pdf_value(o, direction);
  // std::cout << "HittablePdf::value(" << direction << ") -> " << retval << std::endl;
  return retval;
}

Vec3 HittablePdf::generate() const
{
  auto retval =  ptr->random(o);
  // std::cout << "HittablePdf::generate() -> " << retval << std::endl;
  return retval;
}

MixturePdf::MixturePdf(std::shared_ptr<Pdf> p0, std::shared_ptr<Pdf> p1)
{
  p[0] = p0;
  p[1] = p1;
}

double MixturePdf::value(const Vec3 &direction) const
{
  return 0.5 * p[0]->value(direction) + 0.5 *p[1]->value(direction);
}

Vec3 MixturePdf::generate() const
{
  if (random_double() < 0.5)
    return p[0]->generate();
  else
    return p[1]->generate();
}

Vec3 random_to_sphere(double radius, double distance_squared)
{
  auto r1 = random_double();
  auto r2 = random_double();
  auto z = 1 + r2 * (sqrt(1 - radius * radius / distance_squared) - 1);

  auto phi = 2 * pi * r1;
  auto x = cos(phi)*sqrt(1 - z * z);
  auto y = sin(phi)*sqrt(1 - z * z);

  return Vec3(x, y, z);
}
