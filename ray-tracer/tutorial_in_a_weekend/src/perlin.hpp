#ifndef PERLIN_HPP
#define PERLIN_HPP

#include "vec3.hpp"

class Perlin {
public:
  Perlin();
  ~Perlin();

  double noise(const Point3 &p) const;
  double turb(const Point3 &p, int depth=7) const;

private:
  static const int point_count = 256;
  Vec3* ranvec;
  int* perm_x;
  int* perm_y;
  int* perm_z;

  static int *perlin_generate_perm();
  static void permute(int *p, int n);
  static double perlin_interp(Vec3 c[2][2][2], double u, double v, double w);
};

#endif
