#include "texture.hpp"

#include <string>

#include "color.hpp"
#include "rtweekend.hpp"

void Texture::setName(std::string n)
{
  name = n;
}

Texture::~Texture()
{
}

void Texture::serialize(std::ostream &out) const
{
  out << "Unknown texture type";
}

std::ostream& operator<<(std::ostream &out, const Texture &t)
{
  t.serialize(out);
  return out;
}

SolidColor::SolidColor()
{
}

SolidColor::SolidColor(Color c)
  : color_value(c)
{
}

SolidColor::SolidColor(double red, double green, double blue)
  : SolidColor(Color(red, green, blue))
{
}

Color SolidColor::value(double u, double v, const Vec3 &p) const
{
  return color_value;
}

void SolidColor::serialize(std::ostream &out) const
{
  out
    << "SolidColor("
    << color_value[0]
    << ", "
    << color_value[1]
    << ", "
    << color_value[2]
    << ")";
}

CheckerTexture::CheckerTexture()
{
}

CheckerTexture::CheckerTexture(Color c1, Color c2)
  : even(new SolidColor(c1))
  , odd(new SolidColor(c2))
{
}

CheckerTexture::CheckerTexture(Texture *even, Texture *odd)
  : even(even)
  , odd(odd)
{
}

Color CheckerTexture::value(double u, double v, const Point3 &p) const
{
  auto sines = sin(10*p.x())*sin(10*p.y())*sin(10*p.z());
  if (sines < 0)
    return odd->value(u, v, p);
  else
    return even->value(u, v, p);
}
void CheckerTexture::serialize(std::ostream &out) const
{
  out
    << "CheckerTexture("
    << (*even)
    << ", "
    << (*odd)
    << ")";
}

NoiseTexture::NoiseTexture()
{
}

NoiseTexture::NoiseTexture(double sc)
  : scale(sc)
{
}

Color NoiseTexture::value(double u, double v, const Point3& p) const
{
  return Color(1, 1, 1) * 0.5 * (1.0 + sin(scale * p.z() + 10*noise.turb(p)));
}

ImageTexture::ImageTexture()
  : data(nullptr)
  , width(0)
  , height(0)
  , bytes_per_scanline(0)
  , bilinear(true)
{
}

ImageTexture::ImageTexture(const std::string &filename)
  : bilinear(true)
{
  auto components_per_pixel = bytes_per_pixel;

  data = stbi_load(
    filename.c_str(), &width, &height, &components_per_pixel, components_per_pixel
  );

  if(!data) {
    std::cerr << "ERROR: Could not load texture image file '" << filename << "'.\n";
    width = height = 0;
  }

  bytes_per_scanline = bytes_per_pixel * width;
}

ImageTexture::~ImageTexture()
{
  delete data;
}

Color ImageTexture::value(double u, double v, const Vec3 &p) const
{
  // If we have no texture data, then return solid cyan as a debugging aid.
  if (data == nullptr)
    return Color(0, 1, 1);

  const auto color_scale = 1.0 / 255.0;

  // Clamp input texture coordinates to [0,1] x [1,0]
  u = clamp(u, 0.0, 1.0);
  v = 1.0 - clamp(v, 0.0, 1.0);  // Flip V to image coordinates

  if(bilinear) {
    auto i = u * width;
    auto j = v * height;

    auto int_i = static_cast<int>(i);
    auto int_j = static_cast<int>(j);

    auto frac_i = i - int_i;
    auto frac_j = j - int_j;

    auto pixel00 = data + (int_j + 0) % height * bytes_per_scanline + (int_i + 0) % width * bytes_per_pixel;
    auto pixel01 = data + (int_j + 0) % height * bytes_per_scanline + (int_i + 1) % width * bytes_per_pixel;
    auto pixel11 = data + (int_j + 1) % height * bytes_per_scanline + (int_i + 1) % width * bytes_per_pixel;
    auto pixel10 = data + (int_j + 1) % height * bytes_per_scanline + (int_i + 0) % width * bytes_per_pixel;

    auto color00 = Color(color_scale * pixel00[0], color_scale * pixel00[1], color_scale * pixel00[2]);
    auto color01 = Color(color_scale * pixel01[0], color_scale * pixel01[1], color_scale * pixel01[2]);
    auto color11 = Color(color_scale * pixel11[0], color_scale * pixel11[1], color_scale * pixel11[2]);
    auto color10 = Color(color_scale * pixel10[0], color_scale * pixel10[1], color_scale * pixel10[2]);

    auto color00_01 = color00 * (1 - frac_i) + color01 * frac_i;
    auto color10_11 = color10 * (1 - frac_i) + color11 * frac_i;

    return color00_01 * (1 - frac_j) + color10_11 * frac_j;
  } else {
    auto i = static_cast<int>(u * width);
    auto j = static_cast<int>(v * height);

    // Clamp integer mapping, since actual coordinates should be less than 1.0
    if (i >= width)  i = width-1;
    if (j >= height) j = height-1;

    auto pixel = data + j*bytes_per_scanline + i*bytes_per_pixel;

    return Color(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
  }
}
