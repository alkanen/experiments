# import vector


class Point:
    def __init__(self, x: float = 0, y: float = 0, z: float = 0) -> None:
        self.e = [x, y, z]

    def x(self) -> float:
        return self.e[0]

    def y(self) -> float:
        return self.e[1]

    def z(self) -> float:
        return self.e[2]

    def __add__(self, other) -> "Point":
        return Point(self.x() + other.x(), self.y() + other.y(), self.z() + other.z())

    def __sub__(self, other):  # -> "vector.Vector":
        import vector

        return vector.Vector(
            self.x() - other.x(), self.y() - other.y(), self.z() - other.z()
        )

    def __getitem__(self, i: int) -> float:
        return self.e[i]

    def __repr__(self):
        return f"Point({self.x()}, {self.y()}, {self.z()})"
