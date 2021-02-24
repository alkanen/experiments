from point import Point
from vector import Vector


class Ray:
    def __init__(self, origin: Point, direction: Vector):
        self.origin = origin
        self.direction = direction

    def at(self, t: float):
        return self.origin + t * self.direction

    def __repr__(self):
        return f"Ray({repr(self.point)}, {repr(self.vector)}"
