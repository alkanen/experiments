#ifndef MATERIAL_H
#define MATERIAL_H

#include "rtweekend.hpp"

#include "texture.hpp"

class HitRecord;

class Material {
public:
  virtual bool scatter(
    const Ray& r_in, const HitRecord& rec, Color& attenuation, Ray& scattered, double &pdf
  ) const {
    return false;
  }
  virtual double scattering_pdf(
    const Ray &r_in, const HitRecord &rec, const Ray &scattered
  ) const {
    return 0;
  }
  virtual Color emitted(double u, double v, const Point3 &p) const
  {
    return Color(0,0,0);
  }
};

class Lambertian : public Material {
public:
  Lambertian(const Color &a) : albedo(new SolidColor(a)) {}
  Lambertian(Texture *a) : albedo(a) {}

  virtual bool scatter(
    const Ray &r_in, const HitRecord &rec, Color &alb, Ray &scattered, double &pdf
  ) const override
  {
    auto scatter_direction = random_in_hemisphere(rec.normal);

    // Catch degenerate scatter direction
    if (scatter_direction.near_zero())
      scatter_direction = rec.normal;

    scattered = Ray(rec.p, unit_vector(scatter_direction), r_in.time());
    alb = albedo->value(rec.u, rec.v, rec.p);
    pdf = 0.5 / pi;

    return true;
  }

  double scattering_pdf(
    const Ray &r_in, const HitRecord &rec, const Ray &scattered
  ) const {
    auto cosine = dot(rec.normal, unit_vector(scattered.direction()));
    return cosine < 0 ? 0 : cosine/pi;
  }

public:
  Texture *albedo;
};

class Metal : public Material {
public:
  Metal(const Color& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

  virtual bool scatter(
    const Ray &r_in, const HitRecord &rec, Color &attenuation, Ray &scattered, double &pdf
  ) const override {
    Vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
    scattered = Ray(rec.p, reflected + fuzz * random_in_unit_sphere(), r_in.time());
    attenuation = albedo;
    return (dot(scattered.direction(), rec.normal) > 0);
  }

public:
  Color albedo;
  double fuzz;
};

class Dielectric : public Material {
public:
  Dielectric(double index_of_refraction) : ir(index_of_refraction) {}

  virtual bool scatter(
    const Ray &r_in, const HitRecord &rec, Color &attenuation, Ray &scattered, double &pdf
  ) const override {
    attenuation = Color(1.0, 1.0, 1.0);
    double refraction_ratio = rec.front_face ? (1.0/ir) : ir;

    Vec3 unit_direction = unit_vector(r_in.direction());
    double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
    double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

    bool cannot_refract = refraction_ratio * sin_theta > 1.0;
    Vec3 direction;

    if(cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
      direction = reflect(unit_direction, rec.normal);
    else
      direction = refract(unit_direction, rec.normal, refraction_ratio);

    scattered = Ray(rec.p, direction, r_in.time());
    return true;
  }

public:
  double ir; // Index of Refraction

private:
  static double reflectance(double cosine, double ref_idx) {
    // Use Schlick's approximation for reflectance.
    auto r0 = (1-ref_idx) / (1+ref_idx);
    r0 = r0*r0;
    return r0 + (1-r0)*pow((1 - cosine),5);
  }
};

class DiffuseLight : public Material
{
public:
  DiffuseLight(Texture *a) : emit(a) {}
  DiffuseLight(Color c) : emit(new SolidColor(c)) {}

  virtual bool scatter(
    const Ray &ray_in, const HitRecord &rec, Color &attenuation, Ray &scattered, double &pdf
  ) const override {
    return false;
  }

  virtual Color emitted(double u, double v, const Point3 &p) const override
  {
    return emit->value(u, v, p);
  }

public:
  Texture *emit;
};

class Isotropic : public Material
{
public:
  Isotropic(Color c) : albedo(new SolidColor(c)) {}
  Isotropic(Texture *a) : albedo(a) {}

  virtual bool scatter(
    const Ray &ray_in, const HitRecord &rec, Color &attenuation, Ray &scattered, double &pdf
  ) const override {
    scattered = Ray(rec.p, random_in_unit_sphere(), ray_in.time());
    attenuation = albedo->value(rec.u, rec.v, rec.p);
    return true;
  }

public:
  Texture *albedo;
};

#endif
