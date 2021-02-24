from point import Point
from ray import Ray
from vector import Vector


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

        p = self.p
        print(f"p {p}")
        print("TL:")
        print(f"p {p} + v1 {v1}: {p + v1}")
        print(f"p {p} + v1 {v1} - v2 {v2}: {p + v1 - v2}")
        print("TR:")
        print(f"p {p} + v1 {v1}: {p + v1}")
        print(f"p {p} + v1 {v1} + v2 {v2}: {p + v1 + v2}")
        print("BR:")
        print(f"p {p} - v1 {v1}: {p - v1}")
        print(f"p {p} - v1 {v1} + v2 {v2}: {p - v1 + v2}")
        print("BL:")
        print(f"p {p} - v1 {v1}: {p - v1}")
        print(f"p {p} - v1 {v1} - v2 {v2}: {p - v1 - v2}")

        self.tl = self.p + self.v1 - self.v2
        self.tr = self.p + self.v1 + self.v2
        self.br = self.p - self.v1 + self.v2
        self.bl = self.p - self.v1 - self.v2

    def hit(self, ray: Ray) -> bool:
        print(f"Check top triangle {self.tl}, {self.tr}, {self.br}")
        topright = self._ray_intersect(ray, self.tl, self.tr, self.br)
        print(f"Check bot triangle {self.tl}, {self.br}, {self.bl}")
        bottomleft = self._ray_intersect(ray, self.tl, self.br, self.bl)

        if topright or bottomleft:
            return True

        return False

    @classmethod
    def _ray_intersect(cls, ray: Ray, v0: Point, v1: Point, v2: Point) -> bool:
        ray_origin = ray.origin
        ray_vector = ray.direction

        edge1 = v1 - v0
        edge2 = v2 - v0

        h = (ray_origin - Point(0, 0, 0)).cross(edge2)
        a = edge1.dot(h)

        # The ray is parallell to the triangle
        if abs(a) < cls.epsilon:
            print(f"Ray is parallell ({a})")
            return 0

        inv_a = 1.0 / a
        s = ray_origin - v0
        u = inv_a * s.dot(h)
        if u < 0 or u > 1:
            # Why?
            print("u outside of [0, 1]")
            return 0

        q = s.cross(edge1)
        v = inv_a * ray_vector.dot(q)
        if v < 0 or u + v > 1:
            # Why?
            print("v outside of [0, 1-v]")
            return 0

        # Multiples of ray_vector from ray_origin to intersection
        t = inv_a * edge2.dot(q)

        # Negative t values means intersection would be behind the ray
        if t <= cls.epsilon:
            print("t too small")

        return t > cls.epsilon
