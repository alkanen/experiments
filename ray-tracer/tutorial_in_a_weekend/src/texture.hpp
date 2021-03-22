#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <iostream>

#include "rtweekend.hpp"

#include "perlin.hpp"
#include "rtw_stb_image.hpp"


class Texture {
public:
  void setName(std::string n) {name = n;}
  virtual Color value(double u, double v, const Point3 &p) const = 0;
  virtual ~Texture() {};
  virtual void serialize(std::ostream &out) const {
    out << "Unknown texture type";
  }

public:
  std::string name;
};
inline std::ostream& operator<<(std::ostream &out, const Texture &t)
{
  t.serialize(out);
  return out;
}

class SolidColor : public Texture {
public:
  SolidColor() {}
  SolidColor(Color c) : color_value(c) {}

  SolidColor(double red, double green, double blue)
    : SolidColor(Color(red, green, blue)) {}

  virtual Color value(double u, double v, const Vec3 &p) const override {
    return color_value;
  }
  virtual void serialize(std::ostream &out) const override {
    out
      << "SolidColor("
      << color_value[0]
      << ", "
      << color_value[1]
      << ", "
      << color_value[2]
      << ")";
  }

public:
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
  virtual void serialize(std::ostream &out) const override {
    out
      << "CheckerTexture("
      << (*even)
      << ", "
      << (*odd)
      << ")";
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

class ImageTexture : public Texture {
public:
  const static int bytes_per_pixel = 3;

  ImageTexture()
    : data(nullptr), width(0), height(0), bytes_per_scanline(0) {}

  ImageTexture(const char *filename) {
    auto components_per_pixel = bytes_per_pixel;

    data = stbi_load(
      filename, &width, &height, &components_per_pixel, components_per_pixel
    );

    if(!data) {
      std::cerr << "ERROR: Could not load texture image file '" << filename << "'.\n";
      width = height = 0;
    }

    bytes_per_scanline = bytes_per_pixel * width;
  }

  ~ImageTexture() {
    delete data;
  }

  virtual Color value(double u, double v, const Vec3 &p) const override {
    // If we have no texture data, then return solid cyan as a debugging aid.
    if (data == nullptr)
      return Color(0, 1, 1);

    // Clamp input texture coordinates to [0,1] x [1,0]
    u = clamp(u, 0.0, 1.0);
    v = 1.0 - clamp(v, 0.0, 1.0);  // Flip V to image coordinates

    auto i = static_cast<int>(u * width);
    auto j = static_cast<int>(v * height);

    // Clamp integer mapping, since actual coordinates should be less than 1.0
    if (i >= width)  i = width-1;
    if (j >= height) j = height-1;

    const auto color_scale = 1.0 / 255.0;
    auto pixel = data + j*bytes_per_scanline + i*bytes_per_pixel;

    return Color(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
  }

private:
  unsigned char *data;
  int width, height;
  int bytes_per_scanline;
};

#endif
