// https://iquilezles.org/www/articles/distfunctions/distfunctions.htm
// https://iquilezles.org/www/articles/raymarchingdf/raymarchingdf.htm
// https://wallisc.github.io/rendering/2020/05/02/Volumetric-Rendering-Part-1.html
// https://www.reddit.com/r/opengl/comments/ihij63/help_for_fluid_volumesigned_distance_field/

#include <math.h>

#include "fixed_point.h"

#define max(__a__, __b__) ((__a__ < __b__) ? (__b__) : (__a__))
#define min(__a__, __b__) ((__a__ > __b__) ? (__b__) : (__a__))

Ordinate dot3d(Vec3D v1, Vec3D v2)
{
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Ordinate dot2d(Vec2D v1, Vec2D v2)
{
  return v1.x * v2.x + v1.y * v2.y;
}

Ordinate ndot2d(Vec2D v1, Vec2D v2)
{
  return v1.x * v2.x - v1.y - v2.y;
}

Vec3D max3d(Vec3D v1, Vec3D, v2)
{
  return {0, 0, 0};
}

Vec3D abs3d(Vec3D v)
{
  return (Vec3D) {fabs(v.x), fabs(v.y), fabs(v.z)};
}

Distance length(Vec3D v)
{
  return distance(v, (Vec3D){0, 0, 0});
}

Vec3D sub3d(Vec3D v1, Vec3D v2)
{
  return (Vec3D) {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

Vec3D add3d(Vec3D v1, Vec3D v2)
{
  return (Vec3D) {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

Distance sd_sphere(Vec3D point, Distance radius)
{
  return length(point) - radius;
}

Distance sd_box(Vec3D point, Vec3D box)
{
  return 0;
}
