from aabb import AABB
from geometry import Point, Vec


class Object:
    @property
    def aabb(self) -> AABB:
        return AABB(
            Vec(0, 0, 0),
            (0, 0),
            Vec(0, 0, 0),
            (0, 0),
            Vec(0, 0, 0),
            (0, 0),
        )


class Cube(Object):
    def __init__(self, x, y, z=0, side=1):
        self.bottom_front_left = Point(x, y, z)
        self.side = side
        self._aabb = None

    @property
    def x(self):
        return self.bottom_front_left.x

    @property
    def y(self):
        return self.bottom_front_left.y

    @property
    def z(self):
        return self.bottom_front_left.z

    def within(self, p: Point):
        return (
            (self.bottom_front_left.x <= p.x <= self.bottom_front_left.x + self.side)
            and (
                self.bottom_front_left.y <= p.y <= self.bottom_front_left.y + self.side
            )
            and (
                self.bottom_front_left.z <= p.z <= self.bottom_front_left.z + self.side
            )
        )

    @property
    def aabb(self) -> AABB:
        if not self._aabb:
            n1 = Vec(1, 0, 0)
            d1_min = min(
                n1.dot(self.bottom_front_left) + self.side,
                n1.dot(self.bottom_front_left),
            )
            d1_max = max(
                n1.dot(self.bottom_front_left) + self.side,
                n1.dot(self.bottom_front_left),
            )
            n2 = Vec(0, 1, 0)
            d2_min = min(
                n2.dot(self.bottom_front_left) + self.side,
                n2.dot(self.bottom_front_left),
            )
            d2_max = max(
                n2.dot(self.bottom_front_left) + self.side,
                n2.dot(self.bottom_front_left),
            )
            n3 = Vec(0, 0, 1)
            d3_min = min(
                n3.dot(self.bottom_front_left) + self.side,
                n3.dot(self.bottom_front_left),
            )
            d3_max = max(
                n3.dot(self.bottom_front_left) + self.side,
                n3.dot(self.bottom_front_left),
            )
            self._aabb = AABB(
                normal1=n1,
                d1_min=d1_min,
                d1_max=d1_max,
                normal2=n2,
                d2_min=d2_min,
                d2_max=d2_max,
                normal3=n3,
                d3_min=d3_min,
                d3_max=d3_max,
            )

        return self._aabb

    def __str__(self):
        return (
            f"Cube(x: {self.bottom_front_left.x}, y: {self.bottom_front_left.y}, "
            f"z: {self.bottom_front_left.z}, side: {self.side})"
        )

    def __repr__(self):
        return str(self)


class Sphere(Object):
    def __init__(self, x, y, z=0, radius=1):
        self.center = Point(x, y, z)
        # # Don't do negative radii
        # self.r = abs(radius)
        self.r = radius
        self._aabb = None

    @property
    def x(self):
        return self.center.x

    @property
    def y(self):
        return self.center.y

    @property
    def z(self):
        return self.center.z

    def within(self, p: Point):
        return (p.x - self.x) ** 2 + (p.y - self.y) ** 2 + (
            p.z - self.z
        ) ** 2 <= self.r ** 2

    @property
    def aabb(self) -> AABB:
        if not self._aabb:
            n1 = Vec(1, 0, 0)
            d1_min = min(n1.dot(self.center) + self.r, n1.dot(self.center) - self.r)
            d1_max = max(n1.dot(self.center) + self.r, n1.dot(self.center) - self.r)
            n2 = Vec(0, 1, 0)
            d2_min = min(n2.dot(self.center) + self.r, n2.dot(self.center) - self.r)
            d2_max = max(n2.dot(self.center) + self.r, n2.dot(self.center) - self.r)
            n3 = Vec(0, 0, 1)
            d3_min = min(n3.dot(self.center) + self.r, n3.dot(self.center) - self.r)
            d3_max = max(n3.dot(self.center) + self.r, n3.dot(self.center) - self.r)
            self._aabb = AABB(
                normal1=n1,
                d1_min=d1_min,
                d1_max=d1_max,
                normal2=n2,
                d2_min=d2_min,
                d2_max=d2_max,
                normal3=n3,
                d3_min=d3_min,
                d3_max=d3_max,
            )

        return self._aabb

    def __str__(self):
        return f"Sphere(x: {self.center.x}, y: {self.center.y}, z: {self.center.z}, radius: {self.r})"

    def __repr__(self):
        return str(self)
