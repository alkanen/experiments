from typing import NamedTuple


class Vec(NamedTuple):
    x: int
    y: int
    z: int = 0

    def __str__(self):
        return f"({self.x}, {self.y}, {self.z})"

    def dot(self, other: "Vec") -> float:
        return self.x * other.x + self.y * other.y + self.z * other.z


class Point(Vec):
    pass
