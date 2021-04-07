#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <iostream>
#include <string>

#include "stb_image.h"

#include "color.hpp"
#include "perlin.hpp"


class Texture {
public:
  void setName(std::string n);
  virtual Color value(double u, double v, const Point3 &p) const = 0;
  virtual ~Texture();
  virtual void serialize(std::ostream &out) const;

public:
  std::string name;
};
std::ostream& operator<<(std::ostream &out, const Texture &t);


class SolidColor : public Texture {
public:
  SolidColor();
  SolidColor(Color c);
  SolidColor(double red, double green, double blue);

  virtual Color value(double u, double v, const Vec3 &p) const override;
  virtual void serialize(std::ostream &out) const override;

public:
  Color color_value;
};


class CheckerTexture : public Texture {
public:
  CheckerTexture();
  CheckerTexture(Color c1, Color c2);
  CheckerTexture(Texture *even, Texture *odd);

  virtual Color value(double u, double v, const Point3 &p) const override;

  virtual void serialize(std::ostream &out) const override;

public:
  Texture *even;
  Texture *odd;
};

class NoiseTexture : public Texture
{
public:
  NoiseTexture();
  NoiseTexture(double sc);

  virtual Color value(double u, double v, const Point3& p) const override;

public:
  Perlin noise;
  double scale;
};

class ImageTexture : public Texture {
public:
  const static int bytes_per_pixel = 3;

  ImageTexture();
  ImageTexture(const std::string &filename);
  ~ImageTexture();

  virtual Color value(double u, double v, const Vec3 &p) const override;

private:
  unsigned char *data;
  int width, height;
  int bytes_per_scanline;
};

#endif
