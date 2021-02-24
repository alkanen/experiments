from math import sqrt
from typing import Union

# from point import Point
import point


class Vector(point.Point):
    def __repr__(self):
        return f"Vector({self.x()}, {self.y()}, {self.z()})"

    def __neg__(self) -> "Vector":
        return Vector(-self.x, -self.y, -self.z)

    def __sub__(self, other: "Vector") -> "Vector":
        return Vector(self.x() - other.x(), self.y() - other.y(), self.z() - other.z())

    def __rsub__(
        self, other: Union[point.Point, "Vector"]
    ) -> Union[point.Point, "Vector"]:
        if isinstance(other, point.Point):
            return point.Point(
                other.x() - self.x(), other.y() - self.y(), other.z() - self.z()
            )
        else:
            return Vector(
                other.x() - self.x(), other.y() - self.y(), other.z() - self.z()
            )

    def __iadd__(self, other: "Vector") -> "Vector":
        self.e[0] += other.e[0]
        self.e[1] += other.e[1]
        self.e[2] += other.e[2]

        return self

    def __imul__(self, const: float) -> "Vector":
        self.e[0] *= const
        self.e[1] *= const
        self.e[2] *= const

        return self

    def __mul__(self, const: float) -> "Vector":
        return Vector(self.x() * const, self.y() * const, self.z() * const)

    def __itruediv__(self, const: float) -> "Vector":
        self *= 1 / const
        return self

    def __truediv__(self, const: float) -> "Vector":
        return self * (1 / const)

    def length(self) -> float:
        return sqrt(self.length_squared())

    def length_squared(self) -> float:
        return sum([self.e[i] ** 2 for i in range(3)])

    def abs(self) -> "Vector":
        return Vector(abs(self.x()), abs(self.y()), abs(self.z()))

    def dot(self, other: "Vector") -> float:
        return self.x() * other.x() + self.y() * other.y() + self.z() * other.z()

    def cross(self, other: "Vector") -> "Vector":
        return Vector(
            self.y() * other.z() - self.z() * other.y(),
            self.z() * other.x() - self.x() * other.z(),
            self.x() * other.y() - self.y() * other.x(),
        )

    def near_zero(self) -> bool:
        """Return true if the vector is close to zero in all dimensions."""
        s = 1e-8
        return (abs(self.x()) < s) and (abs(self.y()) < s) and (abs(self.z()) < s)
