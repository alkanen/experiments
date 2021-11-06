from geometry import Point, Vec


class AABB:
    def __init__(
        # self, p1: Point, normal1: Vec, p2: Point, normal2: Vec, p3: Point, normal3: Vec
        self,
        normal1: Vec,
        d1_min: float,
        d1_max: float,
        normal2: Vec,
        d2_min: float,
        d2_max: float,
        normal3: Vec,
        d3_min: float,
        d3_max: float,
    ):
        self.n1 = normal1
        self.n2 = normal2
        self.n3 = normal3

        self.d1 = (d1_min, d1_max)
        self.d2 = (d2_min, d2_max)
        self.d3 = (d3_min, d3_max)

    def volume(self):
        d1 = abs(self.d1[1] - self.d1[0])
        d2 = abs(self.d2[1] - self.d2[0])
        d3 = abs(self.d3[1] - self.d3[0])
        # print("Volume:")
        # print(f"  {d1} = abs({self.d1[1]} - {self.d1[0]})")
        # print(f"  {d2} = abs({self.d2[1]} - {self.d2[0]})")
        # print(f"  {d3} = abs({self.d3[1]} - {self.d3[0]})")
        # print(f"  {(d1 if d1 else 1) * (d2 if d2 else 1) * (d3 if d3 else 1)}")

        return (d1 if d1 else 1) * (d2 if d2 else 1) * (d3 if d3 else 1)

    def within(self, p: Point):
        return (
            (self.d1[0] <= p.x <= self.d1[1])
            and (self.d2[0] <= p.y <= self.d2[1])
            and (self.d3[0] <= p.z <= self.d3[1])
        )

    def border(self, p: Point):
        if (
            (p.x == self.d1[0] or p.x == self.d1[1])
            and (self.d2[0] <= p.y <= self.d2[1])
            and (self.d3[0] <= p.z <= self.d3[1])
        ):
            return True

        if (
            (p.y == self.d2[0] or p.y == self.d2[1])
            and (self.d1[0] <= p.x <= self.d1[1])
            and (self.d3[0] <= p.z <= self.d3[1])
        ):
            return True

        if (
            (p.z == self.d3[0] or p.z == self.d3[1])
            and (self.d1[0] <= p.x <= self.d1[1])
            and (self.d2[0] <= p.y <= self.d2[1])
        ):
            return True

        return False

    def __add__(self, other: "AABB") -> "AABB":
        if self.n1 != other.n1 or self.n2 != other.n2 or self.n3 != other.n3:
            raise ValueError("Can't combine bounding boxes with different normals")

        d1_min = min(self.d1[0], other.d1[0])
        d1_max = max(self.d1[1], other.d1[1])
        d2_min = min(self.d2[0], other.d2[0])
        d2_max = max(self.d2[1], other.d2[1])
        d3_min = min(self.d3[0], other.d3[0])
        d3_max = max(self.d3[1], other.d3[1])
        # print("Add:")
        # print(f"  min: {d1_min} = min({self.d1[0]}, {other.d1[0]})")
        # print(f"  max: {d1_max} = max({self.d1[1]}, {other.d1[1]})")
        # print(f"  min: {d2_min} = min({self.d2[0]}, {other.d2[0]})")
        # print(f"  max: {d2_max} = max({self.d2[1]}, {other.d2[1]})")
        # print(f"  min: {d3_min} = min({self.d3[0]}, {other.d3[0]})")
        # print(f"  max: {d3_max} = max({self.d3[1]}, {other.d3[1]})")

        return AABB(
            normal1=self.n1,
            d1_min=d1_min,
            d1_max=d1_max,
            normal2=self.n2,
            d2_min=d2_min,
            d2_max=d2_max,
            normal3=self.n3,
            d3_min=d3_min,
            d3_max=d3_max,
        )

    def __str__(self):
        return f"{self.n1} @ {self.d1}, {self.n2} @ {self.d2}, {self.n3} @ {self.d3}"

    def __repr__(self):
        return str(self)
