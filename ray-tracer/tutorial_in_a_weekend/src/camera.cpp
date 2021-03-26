#include <nlohmann/json.hpp>

#include "camera.hpp"
#include "ray.hpp"
#include "vec3.hpp"
#include "rtweekend.hpp"

Camera::Camera(
  Point3 look_from,
  Point3 look_at,
  Vec3 vup,
  double vfov,
  double aspect_ratio,
  double aperture,
  double focus_dist,
  double time0,
  double time1
)
{
  setup(look_from, look_at, vup, vfov, aspect_ratio, aperture, focus_dist, time0, time1);
}

Camera::Camera(const nlohmann::json &conf, double aspect_ratio)
{
  setup(
    Point3(conf["look_from"][0], conf["look_from"][1], conf["look_from"][2]),
    Point3(conf["look_at"][0], conf["look_at"][1], conf["look_at"][2]),
    Vec3(conf["up"][0], conf["up"][1], conf["up"][2]),
    conf["vertical_fov"],
    aspect_ratio,
    conf["aperture"],
    conf["dist_to_focus"],
    conf["time_start"],
    conf["time_end"]
  );
}

void Camera::setup(
  Point3 look_from,
  Point3 look_at,
  Vec3 vup,
  double vfov,
  double aspect_ratio,
  double aperture,
  double focus_dist,
  double time0,
  double time1
)
{
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

Ray Camera::get_ray(double s, double t) const
{
  Vec3 rd = lens_radius * random_in_unit_disk();
  Vec3 offset = u * rd.x() + v * rd.y();

  return Ray(
    origin + offset,
    lower_left_corner + s * horizontal + t * vertical - origin - offset,
    random_double(time0, time1)
  );
}
