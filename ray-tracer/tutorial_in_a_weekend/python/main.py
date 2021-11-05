from math import inf

from plane import Rectangle
from point import Point
from ray import Ray
from vector import Vector


class Plane:
    def __init__(self, v0: Point, v1: Point, v2: Point):
        vec1 = v1 - v0
        vec2 = v2 - v0

        normal = vec1.cross(vec2) / vec1.cross(vec2).length()

        # print("vec1:", vec1)
        # print("vec2:", vec2)
        # print("norm:", normal)

        self.a = normal.x()
        self.b = normal.y()
        self.c = normal.z()
        self.d = normal.dot(v0)

        # print("abcd:", self.a, self.b, self.c, self.d)

    def on_plane(self, point: Point):
        return (
            abs(self.a * point.x() + self.b * point.y() + self.c * point.z() - self.d)
            < 0.0000001
        )

    def hit(self, origin: Point, direction: Vector):
        return 0.0


def main():
    p = Point(0, 0, -1)
    v1 = Vector(2, 0, 0)
    v2 = Vector(0, 2, 0)
    rectangle = Rectangle(p, v1, v2)

    p2 = p + v1
    p3 = p + v2
    plane = Plane(p, p2, p3)

    if False:

        print(p)
        print(p2)
        print(p3)

        offset = Vector(0.1, 0.03, 2)
        print(plane.on_plane(p))
        print(plane.on_plane(p2))
        print(plane.on_plane(p3))
        print(plane.on_plane(p + offset))
        print(plane.on_plane(p2 + offset))
        print(plane.on_plane(p3 + offset))
        print(plane.on_plane(p + v1))
        print(plane.on_plane(p2 + v1))
        print(plane.on_plane(p3 + v1))
        print(plane.on_plane(p + v2))
        print(plane.on_plane(p2 + v2))
        print(plane.on_plane(p3 + v2))

        return

    if True:
        look_from = Point(0, 0, 3)
        # look_from = Point(5, 5, 0)
        look_at = Vector(0, 0, -10)
        # intersect = look_from + look_at
        # print("Intersect on plane:", plane.on_plane(intersect))

        ray = Ray(look_from, look_at)
        hit_rec = {}
        res = rectangle.hit(ray, min_t=1e-8, max_t=inf, hit_rec=hit_rec)
        print("Hit on plane:", res)
        print("Record:", hit_rec)

        return

    results = {}
    for z in range(-10, 10):
        for y in range(-10, 10):
            for x in range(-10, 10):
                ray = Ray(Point(x / 10.0, y / 10.0, z / 10.0), Vector(0, 0, 1))
                res = rectangle.hit(ray)

                key = f"{x:d}x{y:d}x{z:d}"
                results[key] = res
                if res:
                    print(key)


if __name__ == "__main__":
    main()
