#ifndef ONB_HPP
#define ONB_HPP

#include "vec3.hpp"

// Orthonormal basis
class Onb {
public:
  Onb();

  inline Vec3 operator[](int i) const;

  Vec3 u() const;
  Vec3 v() const;
  Vec3 w() const;

  Vec3 local(double a, double b, double c) const;
  Vec3 local(const Vec3 &a) const;
  void build_from_w(const Vec3&);

public:
  Vec3 axis[3];
};

#endif
