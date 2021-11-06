from math import inf

from geometry import Point


class BVH:
    def __init__(self, objects, num_dimensions=3, top_down=False):
        self.c1 = None
        self.c2 = None
        self._aabb = None

        if not objects:
            raise ValueError("Can't create a bounding volume with no objects")

        if len(objects) == 1:
            self.c1 = objects[0]
            self._aabb = self.c1.aabb
            # print(f"AABB from c1: {self._aabb}")
            return

        if len(objects) == 2:
            self.c1 = objects[0]
            self.c2 = objects[1]
            self._aabb = self.c1.aabb + self.c2.aabb
            # print(f"AABB from c1 + c2: {self._aabb}")
            return

        if top_down:
            self.top_down(objects, num_dimensions)
        else:
            self.bottom_up(list(objects), num_dimensions)

    def bottom_up(self, objects, num_dimensions):
        while len(objects) > 2:
            # Find the pair of objects which creates the smallest combined AABB
            best_pair = (None, None)
            best_volume = inf
            for i, obj1 in enumerate(objects[:-1]):
                for j, obj2 in enumerate(objects[i + 1 :]):
                    j += i + 1  # Change to proper list index
                    if (vol := (obj1.aabb + obj2.aabb).volume()) < best_volume:
                        # print(f"  Smaller: {vol} < {best_volume}")
                        best_volume = vol
                        best_pair = (i, j)
                    # else:
                    #     print(f"  Not smaller: {vol} < {best_volume}")

            i, j = best_pair
            print(
                f"Smallest volume found: {best_volume} at ({i}, {j}) of {len(objects)}"
            )

            # Replace the pair of objects with a subtree
            objects[i] = BVH([objects[i], objects[j]], top_down=False)
            last_obj = objects.pop()
            if j < len(objects):
                objects[j] = last_obj

        if isinstance(objects[0], BVH):
            self.c1 = objects[0]
        else:
            self.c1 = BVH([objects[0]], top_down=False)

        if isinstance(objects[1], BVH):
            self.c2 = objects[1]
        else:
            self.c2 = BVH([objects[1]], top_down=False)

        self._aabb = self.c1.aabb + self.c2.aabb
        # print(f"AABB from child tree: {self._aabb}, {self._aabb.volume()}")

    def top_down(self, objects, num_dimensions):
        min_dimensions = [inf] * num_dimensions
        max_dimensions = [-inf] * num_dimensions

        for obj in objects:
            aabb = obj.aabb
            min_dimensions[0] = min(aabb.d1[0], min_dimensions[0])
            max_dimensions[0] = max(aabb.d1[1], max_dimensions[0])

            min_dimensions[1] = min(aabb.d2[0], min_dimensions[2])
            max_dimensions[1] = max(aabb.d2[1], max_dimensions[2])

            min_dimensions[2] = min(aabb.d3[0], min_dimensions[2])
            max_dimensions[2] = max(aabb.d3[1], max_dimensions[2])

        deltas = [max_dimensions[i] - min_dimensions[i] for i in range(num_dimensions)]
        largest = deltas.index(max(deltas))

        # Sort and split along the longest dimension
        partitioning = sorted(objects, key=lambda obj: [obj.x, obj.y, obj.z][largest])
        self.c1 = BVH(
            partitioning[: len(partitioning) // 2],
            num_dimensions=num_dimensions,
            top_down=True,
        )
        self.c2 = BVH(
            partitioning[len(partitioning) // 2 :],
            num_dimensions=num_dimensions,
            top_down=True,
        )

        self._aabb = self.c1.aabb + self.c2.aabb
        # print(f"AABB from child tree: {self._aabb}")

    @property
    def aabb(self):
        return self._aabb

    def border(self, p: Point):
        if self.aabb.border(p):
            return True

        if self.c1 and isinstance(self.c1, BVH) and self.c1.border(p):
            return True

        if self.c2 and isinstance(self.c2, BVH) and self.c2.border(p):
            return True

        return False

    def __str__(self):
        return "\n".join(
            [
                "BVC(",
                *(
                    [f"    {x}" for x in str(self.c1).split("\n")]
                    if isinstance(self.c1, BVH)
                    else ["    " + str(self.c1)]
                ),
                *(
                    [f"    {x}" for x in str(self.c2).split("\n")]
                    if isinstance(self.c2, BVH)
                    else ["    " + str(self.c2)]
                ),
                ")",
            ]
        )

    def __repr__(self):
        return str(self)
