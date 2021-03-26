#ifndef MATERIAL_H
#define MATERIAL_H

#include <iostream>
#include <memory>

#include "ray.hpp"
#include "vec3.hpp"
#include "texture.hpp"
#include "pdf.hpp"

class Texture;
class HitRecord;

struct ScatterRecord {
    Ray specular_ray;
    bool is_specular;
    Color attenuation;
    std::shared_ptr<Pdf> pdf;
};

class Material {
public:
  void setName(std::string n);
  virtual bool scatter(const Ray& r_in, const HitRecord& rec, ScatterRecord &srec) const;
  virtual double scattering_pdf(const Ray &r_in, const HitRecord &rec, const Ray &scattered) const;
  virtual Color emitted(const Ray &ray_in, const HitRecord &rec, double u, double v, const Point3 &p) const;
  virtual void serialize(std::ostream &out) const;

public:
  std::string name;
};
std::ostream& operator<<(std::ostream &out, const Material &m);

class Lambertian : public Material {
public:
  Lambertian(const Color &a);
  Lambertian(Texture *a);
  virtual ~Lambertian();

  virtual bool scatter(
    const Ray &r_in, const HitRecord &rec, ScatterRecord &srec
  ) const override;

  double scattering_pdf(
    const Ray &r_in, const HitRecord &rec, const Ray &scattered
  ) const;

  virtual void serialize(std::ostream &out) const override;

public:
  Texture *albedo;
  bool allocated;
};

class Metal : public Material {
public:
  Metal(const Color& a, double f);

  virtual bool scatter(const Ray &r_in, const HitRecord &rec, ScatterRecord &srec) const override;

public:
  Color albedo;
  double fuzz;
};

class Dielectric : public Material {
public:
  Dielectric(double index_of_refraction);

  virtual bool scatter(
    const Ray &r_in, const HitRecord &rec, ScatterRecord &srec
  ) const override;

public:
  double ir; // Index of Refraction

private:
  static double reflectance(double cosine, double ref_idx);
};

class DiffuseLight : public Material
{
public:
  DiffuseLight(Texture *a);
  DiffuseLight(Color c);

  virtual bool scatter(
    const Ray &ray_in, const HitRecord &rec, ScatterRecord &srec
  ) const override;

  virtual Color emitted(
    const Ray &ray_in,
    const HitRecord &rec,
    double u,
    double v,
    const Point3 &p
  ) const override;

public:
  Texture *emit;
};

class Isotropic : public Material
{
public:
  Isotropic(Color c);
  Isotropic(Texture *a);

  virtual bool scatter(
    const Ray &ray_in, const HitRecord &rec, ScatterRecord &srec
  ) const override;

public:
  Texture *albedo;
};

#endif
