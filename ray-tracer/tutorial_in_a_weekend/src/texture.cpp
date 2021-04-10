#include "texture.hpp"

#include <string>
#include <cmath>

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
  , components_per_scanline(0)
  , bilinear(true)
{
}

ImageTexture::ImageTexture(const std::string &filename)
  : bilinear(true)
{
  components_per_pixel = 3;

  data = stbi_loadf(
    filename.c_str(), &width, &height, &valid_components, components_per_pixel
  );

  if(!data) {
    std::cerr << "ERROR: Could not load texture image file '" << filename << "'.\n";
    width = height = 0;
  }

  std::cerr << "Components per pixel: " << components_per_pixel << std::endl;
  std::cerr << "Valid components: " << valid_components << std::endl;
  std::cerr << "Is HDR: " << stbi_is_hdr(filename.c_str()) << std::endl;

  components_per_scanline = components_per_pixel * width;

  /*
  float minmax[4][2];
  for(int c=0; c<4; c++) {
    minmax[c][0] = 1e20;
    minmax[c][1] = -1e20;
  }
  for(int y=0; y<height; y++) {
    for(int x=0; x<width; x++) {
      auto pixel = data + y * components_per_scanline + x * components_per_pixel;
      for(int c = 0; c<components_per_pixel; c++) {
        if(minmax[c][0] > pixel[c])
          minmax[c][0] = pixel[c];
        if(minmax[c][1] < pixel[c])
          minmax[c][1] = pixel[c];
      }
      / *
      const float limit = 100;
      if(pixel[0] > limit || pixel[1] > limit || pixel[2] > limit) {
        std::cerr << "(" << y << ", " << x << ") - " << pixel[0] << ", " << pixel[1] << ", " << pixel[2] << " -> " << Color(pixel[0], pixel[1], pixel[2]) << std::endl;
      }
      * /
    }
  }

  std::cerr
    << "R: (" << minmax[0][0] << ", " << minmax[0][1] << "), "
    << "G: (" << minmax[1][0] << ", " << minmax[1][1] << "), "
    << "B: (" << minmax[2][0] << ", " << minmax[2][1] << "), "
    << "E: (" << minmax[3][0] << ", " << minmax[3][1] << ")" << std::endl;
  */
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

    auto pixel00 = data + (int_j + 0) % height * components_per_scanline + (int_i + 0) % width * components_per_pixel;
    auto pixel01 = data + (int_j + 0) % height * components_per_scanline + (int_i + 1) % width * components_per_pixel;
    auto pixel11 = data + (int_j + 1) % height * components_per_scanline + (int_i + 1) % width * components_per_pixel;
    auto pixel10 = data + (int_j + 1) % height * components_per_scanline + (int_i + 0) % width * components_per_pixel;

    auto color00 = Color(pixel00[0], pixel00[1], pixel00[2]);
    auto color01 = Color(pixel01[0], pixel01[1], pixel01[2]);
    auto color11 = Color(pixel11[0], pixel11[1], pixel11[2]);
    auto color10 = Color(pixel10[0], pixel10[1], pixel10[2]);

    auto color00_01 = color00 * (1 - frac_i) + color01 * frac_i;
    auto color10_11 = color10 * (1 - frac_i) + color11 * frac_i;

    return color00_01 * (1 - frac_j) + color10_11 * frac_j;
  } else {
    int i = static_cast<int>(u * width);
    int j = static_cast<int>(v * height);

    // Clamp integer mapping, since actual coordinates should be less than 1.0
    if (i >= width)  i = width-1;
    if (j >= height) j = height-1;

    auto pixel = data + j * components_per_scanline + i * components_per_pixel;
    if(pixel[0] > 1 || pixel[1] > 1 || pixel[2] > 1) {
      std::cerr << pixel[0] << ", " << pixel[1] << ", " << pixel[2] << " -> " << Color(pixel[0], pixel[1], pixel[2]) << std::endl;
    }
    return Color(pixel[0], pixel[1], pixel[2]);
  }
}
