#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.hpp"

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
  ) {
    auto theta = degrees_to_radians(vfov);
    auto h = tan(theta/2);
    auto viewport_height = 2.0 * h;
    auto viewport_width = aspect_ratio * viewport_height;

    w = unit_vector(look_from - look_at);
    u = unit_vector(cross(vup, w));
    v = cross(w, u);

    origin = look_from;
    horizontal = focus_dist * viewport_width * u;
    vertical = focus_dist * viewport_height * v;
    lower_left_corner = origin - horizontal/2 - vertical/2 - focus_dist * w;

    lens_radius = aperture / 2;

    this->time0 = time0;
    this->time1 = time1;
  }

  Ray get_ray(double s, double t) const {
    Vec3 rd = lens_radius * random_in_unit_disk();
    Vec3 offset = u * rd.x() + v * rd.y();

    return Ray(
      origin + offset,
      lower_left_corner + s * horizontal + t * vertical - origin - offset,
      random_double(time0, time1)
    );
  }

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
