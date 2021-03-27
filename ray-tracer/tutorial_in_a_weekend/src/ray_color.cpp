#include "color.hpp"
#include "ray.hpp"
#include "world.hpp"

Color ray_color(const Ray &r, World &world, int depth)
{
  HitRecord rec;
  if(depth <= 0)
    return Color(0, 0, 0);

  if(!world.objects.hit(r, 0.001, infinity, rec))
    return world.background;

  ScatterRecord srec;
  //std::cerr << "ray_color before rec.material->emitted()" << std::endl;
  Color emitted = rec.material->emitted(r, rec, rec.u, rec.v, rec.p);

  //std::cerr << "ray_color before !rec.material->scatter()" << std::endl;
  if(!rec.material->scatter(r, rec, srec))
    return emitted;

  if(srec.is_specular) {
    //std::cerr << "ray_color before srec.attenuation * ray_color()" << std::endl;
    return srec.attenuation
      * ray_color(srec.specular_ray, world, depth-1);
  }

  Vec3 scatter_direction;
  Ray scattered;
  double pdf;
  if(dynamic_cast<HittableList*>(&world.lights)->size()) {
    //std::cerr << "ray_color before MixturePdf" << std::endl;
    auto p0 = std::make_shared<HittablePdf>(&world.lights, rec.p);
    MixturePdf mixed_pdf = MixturePdf(p0, srec.pdf);
    //std::cerr << "ray_color before mixed_pdf.generate()" << std::endl;
    scatter_direction = mixed_pdf.generate();
    scattered = Ray(rec.p, scatter_direction, r.time());
    //std::cerr << "ray_color before mixed_pdf.value()" << std::endl;
    pdf = mixed_pdf.value(scattered.direction());
  } else {
    //std::cerr << "ray_color before srec.pdf->generat()" << std::endl;
    scatter_direction = srec.pdf->generate();
    scattered = Ray(rec.p, scatter_direction, r.time());
    //std::cerr << "ray_color before srec.pdf->value()" << std::endl;
    pdf = srec.pdf->value(scattered.direction());
  }

  //std::cerr << "ray_color return" << std::endl;
  return emitted
    + srec.attenuation
    * rec.material->scattering_pdf(r, rec, scattered) / pdf
    * ray_color(scattered, world, depth-1);
}