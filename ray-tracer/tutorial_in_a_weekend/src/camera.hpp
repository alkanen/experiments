#ifndef CAMERA_H
#define CAMERA_H

#include <nlohmann/json.hpp>

#include "rtweekend.hpp"

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
  ) {
    setup(look_from, look_at, vup, vfov, aspect_ratio, aperture, focus_dist, time0, time1);
  }

  Camera(json &conf, double aspect_ratio) {
    try {
      setup(
        Point3(conf["look_from"][0], conf["look_from"][1], conf["look_from"][2]),
        Point3(conf["look_at"][0], conf["look_at"][1], conf["look_at"][2]),
        Vec3(conf["up"][0], conf["up"][1], conf["up"][2]),
        conf["vertical_fov"],
        aspect_ratio,
        conf["dist_to_focus"],
        conf["aperture"],
        conf["time_start"],
        conf["time_end"]
      );
    } catch(nlohmann::detail::parse_error &e) {
      std::cerr << "Unable to parse camera parameters: " << e.what() << std::endl;
      throw(e);
    }
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
    void setup(
    Point3 look_from,
    Point3 look_at,
    Vec3 vup,
    double vfov,
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
