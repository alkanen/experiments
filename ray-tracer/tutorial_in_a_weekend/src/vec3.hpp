#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>

using std::sqrt;

class Vec3 {
public:
  Vec3();
  Vec3(double e0, double e1, double e2);

  double x() const;
  double y() const;
  double z() const;

  Vec3 operator-() const;
  double operator[](int i) const;
  double& operator[](int i);

  Vec3& operator+=(const Vec3 &v);

  Vec3& operator*=(const double t);

  Vec3& operator/=(const double t);

  double length() const;

  double length_squared() const;

  static Vec3 random();
  static Vec3 random(double min, double max);

  bool near_zero() const;

public:
  double e[3];
};

// Type aliases for Vec3
using Point3 = Vec3;   // 3D point

// Vec3 Utility Functions

std::ostream& operator<<(std::ostream &out, const Vec3 &v);
Vec3 operator+(const Vec3 &u, const Vec3 &v);
Vec3 operator-(const Vec3 &u, const Vec3 &v);
Vec3 operator*(const Vec3 &u, const Vec3 &v);
Vec3 operator*(double t, const Vec3 &v);
Vec3 operator*(const Vec3 &v, double t);
Vec3 operator/(Vec3 v, double t);
double dot(const Vec3 &u, const Vec3 &v);
Vec3 cross(const Vec3 &u, const Vec3 &v);
Vec3 unit_vector(Vec3 v);
Vec3 random_in_unit_sphere();
Vec3 random_unit_vector();
Vec3 random_in_hemisphere(const Vec3 &normal);
Vec3 random_cosine_direction();
Vec3 random_in_unit_disk();
Vec3 reflect(const Vec3 &v, const Vec3 &n);
Vec3 refract(const Vec3 &uv, const Vec3 &n, double etai_over_etat);

#endif
