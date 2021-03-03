#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "rtweekend.hpp"

#include "perlin.hpp"

class Texture {
public:
  virtual Color value(double u, double v, const Point3 &p) const = 0;
};

class SolidColor : public Texture {
public:
  SolidColor() {}
  SolidColor(Color c) : color_value(c) {}

  SolidColor(double red, double green, double blue)
    : SolidColor(Color(red, green, blue)) {}

  virtual Color value(double u, double v, const Vec3 &p) const override {
    return color_value;
  }

private:
  Color color_value;
};

class CheckerTexture : public Texture {
public:
  CheckerTexture() {}

  CheckerTexture(Color c1, Color c2)
    : even(new SolidColor(c1)), odd(new SolidColor(c2)) {}
  CheckerTexture(Texture *even, Texture *odd)
    : even(even), odd(odd) {}

  virtual Color value(double u, double v, const Point3 &p) const override
  {
    auto sines = sin(10*p.x())*sin(10*p.y())*sin(10*p.z());
    if (sines < 0)
      return odd->value(u, v, p);
    else
      return even->value(u, v, p);
  }

public:
  Texture *even;
  Texture *odd;
};

class NoiseTexture : public Texture
{
public:
  NoiseTexture() {}
  NoiseTexture(double sc) : scale(sc) {}

  virtual Color value(double u, double v, const Point3& p) const override {
    return Color(1, 1, 1) * 0.5 * (1.0 + sin(scale * p.z() + 10*noise.turb(p)));
    //return Color(1, 1, 1) * noise.turb(scale * p);
  }

public:
  Perlin noise;
  double scale;
};

#endif
