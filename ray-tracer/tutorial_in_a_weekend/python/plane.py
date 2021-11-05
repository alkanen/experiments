from math import copysign, inf

from point import Point
from ray import Ray
from vector import Vector


def set_face_normal(ray: Ray, outward_normal: Vector):
    front_face = ray.direction.dot(outward_normal) < 0
    return outward_normal if front_face else -outward_normal


class Rectangle:
    epsilon = 0.0000001

    def __init__(self, point: Point, v1: Vector, v2: Vector):
        """Create a bounded plane defined by a point on the plane and two vectors
        lying on it.  The point is the center of the rectangle and the two
        vectors point to two orthogonal midpoints:

        0,0               0,4
         +-----------------+
         |                 |
         |                 |
         |       p   v1    |
         |        x<------>|
         |        ^        |
         |     v2 |        |
         |        v        |
         +-----------------+
        3,0               3,4

        p  = (1.5, 2, 0)
        v1 = (0, 2, 0)
        v2 = (1.5, 0, 0)

        The normal of the plane is in the direction of v1 x v2.
        """

        dot_prod = v1.dot(v2)
        if not abs(dot_prod) < self.epsilon:
            raise ValueError(
                "Vectors defining a plane must be orthogonal: "
                f"{v1} dot {v2} = {dot_prod}"
            )

        self.p = point
        self.v1 = v1
        self.v2 = v2
        self.normal = self.v1.cross(self.v2).to_unit()

        # Define triangle corners
        self.tl = self.p + self.v1 - self.v2
        self.tr = self.p + self.v1 + self.v2
        self.br = self.p - self.v1 + self.v2
        self.bl = self.p - self.v1 - self.v2

    def hit(
        self, ray: Ray, hit_rec: dict, min_t: float = 0, max_t: float = inf
    ) -> bool:
        vec = self.p - ray.origin
        sign = copysign(1, vec.dot(ray.direction))

        topright = self._ray_intersect(ray, self.tl, self.tr, self.br)
        if not topright:
            bottomleft = self._ray_intersect(ray, self.tl, self.br, self.bl)
            if not bottomleft:
                return False

            t = bottomleft
        else:
            t = topright

        t *= sign

        if t >= min_t and t < max_t:
            hit_rec["t"] = t
            hit_rec["point"] = ray.at(t)
            hit_rec["normal"] = set_face_normal(ray, self.normal)
            hit_rec["material"] = None
            return True

        return False

    @classmethod
    def _ray_intersect(cls, ray: Ray, v0: Point, v1: Point, v2: Point) -> bool:
        ray_origin = ray.origin
        ray_vector = ray.direction
        # A fictitious vector going from origo to the ray origin point
        ray_origin_vector = ray_origin - Point(0, 0, 0)

        # Edges from v0 to the other two vertices
        edge1 = v1 - v0
        edge2 = v2 - v0

        h = edge2.cross(ray_origin_vector)
        a = edge1.dot(h)
        print("edge1:      ", edge1)
        print("edge2:      ", edge2)
        print("ray_origin: ", ray_origin_vector)
        print("h (e2 x ro):", h)
        print("a (e1 . h): ", a)

        # The ray is parallell to the triangle
        if abs(a) < cls.epsilon:
            # print(f"Ray is parallell ({a})")
            return 0

        inv_a = 1.0 / a
        s = ray_origin - v0
        u = inv_a * s.dot(h)
        if u < 0 or u > 1:
            # Why?
            # print("u outside of [0, 1]")
            return 0

        q = s.cross(edge1)
        v = inv_a * ray_vector.dot(q)
        if v < 0 or u + v > 1:
            # Why?
            # print("v outside of [0, 1-v]")
            return 0

        # Multiples of ray_vector from ray_origin to intersection
        t = inv_a * edge2.dot(q)

        # Negative t values means intersection would be behind the ray
        return t
