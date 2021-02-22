#ifndef IMAGE_PLANE_H
#define IMAGE_PLANE_H

#include "hittable.hpp"
#include "vec3.hpp"



class ImagePlane : public Hittable {
public:
  ImagePlane() {}
  ImagePlane(Point3 top_left, Point3 bottom_right, Material *material) :
    tl(top_left), br(bottom_right), m(material) {
    // x is left-right
    // y is up-down
    // if z = 0:
    if(top_left.z() != bottom_right.z())
      fprintf(stdout, "Warning, plane requires Z to be the same for both corners\n");

    tr = Vec3(bottom_right.x(), top_left.y(), top_left.z());
    bl = Vec3(top_left.x(), bottom_right.y(), top_left.z());
  };

  virtual bool hit(
    const Ray &r, double t_min, double t_max, HitRecord &rec
  ) const override;

public:
  Point3 tl, tr, bl, br;
  Material *m;

private:
  bool hits_triangle(
    const Ray &r, double t_min, double t_max,
    const Point3 &v0, const Point3 &v1, const Point3 &v2, Point3 &IntersectPoint
  ) const;
};

bool ImagePlane::hit(const Ray &r, double t_min, double t_max, HitRecord &rec) const
{
  Point3 intersect;
  if(this->hits_triangle(r, t_min, t_max, tl, tr, bl, intersect)) {
    // fprintf(stderr, "First hit\n");
    // fprintf(stderr, "Intersect: %f, %f, %f\n", intersect.x(), intersect.y(), intersect.z());
  } else if(this->hits_triangle(r, t_min, t_max, tr, br, bl, intersect)) {
    // fprintf(stderr, "Second hit\n");
    // fprintf(stderr, "Intersect: %f, %f, %f\n", intersect.x(), intersect.y(), intersect.z());
  } else {
    /*
    auto o = r.origin();
    auto d = r.direction();
    fprintf(
      stderr,
      "No hit (%f, %f, %f) -> (%f, %f, %f)\n",
      o.x(), o.y(), o.z(),
      d.x(), d.y(), d.z()
    );
    */
    return false;
  }

  auto dist = (intersect - r.origin()).length();
  if(dist < t_min or dist > t_max) {
    fprintf(stderr, "Distance %f is outside limit of (%f, %f)\n", dist, t_min, t_max);
    return false;
  }

  rec.p = intersect;
  rec.t = dist;
  rec.set_face_normal(r, Vec3(0, 0, -1));
  rec.material = m;

  return true;
}

bool ImagePlane::hits_triangle(
  const Ray &r, double t_min, double t_max,
  const Point3 &v0, const Point3 &v1, const Point3 &v2, Point3 &intersectPoint
) const
{
  /*
  bool RayIntersectsTriangle(Vector3D rayOrigin, 
                           Vector3D rayVector, 
                           Triangle* inTriangle,
                           Vector3D& outIntersectionPoint)
  */

  const double EPSILON = 0.0000001;
  Point3 rayOrigin = r.origin();
  Vec3 rayVector = r.direction();

  Vec3 edge1, edge2, h, s, q;
  double a, f, u, v;
  edge1 = v1 - v0;
  edge2 = v2 - v0;
  h = cross(rayOrigin, edge2);
  a = dot(edge1, h);
  if (a > -EPSILON && a < EPSILON)
    return false;    // This ray is parallel to this triangle.
  f = 1.0/a;
  s = rayOrigin - v0;
  u = f * dot(s, h);
  if (u < 0.0 || u > 1.0)
    return false;
  q = cross(s, edge1);
  v = f * dot(rayVector, q);
  if (v < 0.0 || u + v > 1.0)
    return false;
  // At this stage we can compute t to find out where the intersection point is on the line.
  float t = f * dot(edge2, q);
  // ray intersection
  if(t > EPSILON) {
    intersectPoint = rayOrigin + rayVector * t;
    return true;
  } else {
    // This means that there is a line intersection but not a ray intersection.
    return false;
  }
}
#endif
