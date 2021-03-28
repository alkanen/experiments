#ifndef CAMERA_H
#define CAMERA_H

#include <nlohmann/json.hpp>

#include "ray.hpp"
#include "vec3.hpp"

using json = nlohmann::json;

class Camera {
public:
  Camera(
    Point3 look_from,
    Point3 look_at,
    Vec3 vup,
    double vfov, // Vertical field-of-view in degrees
    double aspect_ratio,
    double aperture,
    double focus_dist,
    double time0 = 0,
    double time1 = 0
  );
  Camera(const json &conf, double aspect_ratio);

  Ray get_ray(double s, double t) const;

  double start_time(void) const;
  double end_time(void) const;

private:
  void setup(
    Point3 look_from,
    Point3 look_at,
    Vec3 vup,
    double vfov,
    double aspect_ratio,
    double aperture,
    double focus_dist,
    double time0,
    double time1
  );

private:
  Point3 origin;
  Point3 lower_left_corner;
  Vec3 horizontal;
  Vec3 vertical;
  Vec3 u, v, w;
  double lens_radius;
  double time0, time1;
};

#endif
