# https://www.scratchapixel.com/lessons/advanced-rendering/introduction-acceleration-structure/bounding-volume-hierarchy-BVH-part1
# https://www.scratchapixel.com/lessons/advanced-rendering/introduction-acceleration-structure/bounding-volume
import argparse
from random import randint, seed

from PIL import Image
from tqdm import tqdm

from bvh import BVH
from geometry import Point
from objects import Cube, Sphere


def main():
    the_seed = 42
    seed(the_seed)

    top_down = False
    num_objects = 5
    num_dimensions = 2

    width = 1280
    height = 720
    depth = (width + height) / 2

    dimensions = [
        width,
        0 if num_dimensions < 2 else height,
        1 if num_dimensions < 3 else depth,
    ]

    min_radius = 10
    max_radius = 40

    objects = [
        (
            Sphere(
                *[randint(0, value) for value in dimensions],
                radius=randint(min_radius, max_radius),
            )
            if randint(0, 1)
            else Cube(
                *[randint(0, value) for value in dimensions[:2]],
                -1,
                side=randint(min_radius, max_radius),
            )
        )
        for _ in range(num_objects)
    ]
    # print(objects)

    bvh = BVH(objects, top_down=top_down)
    # print(bvh)

    img = Image.new("RGB", (width, height), "white")

    for y in tqdm(range(height)):
        for x in range(width):
            # Bounding box backgrounds
            for obj in objects:
                if obj.aabb.within(Point(x, y, 0)):
                    img.putpixel((x, y), (128, 128, 128))
                    break

            # Objects
            for obj in objects:
                if obj.within(Point(x, y, 0)):
                    img.putpixel((x, y), (0, 0, 0))
                    break

            # Object borders
            for obj in objects:
                if obj.aabb.border(Point(x, y, 0)):
                    img.putpixel((x, y), (255, 0, 0))
                    break

            if bvh.border(Point(x, y, 0)):
                img.putpixel((x, y), (0, 255, 0))

    img.save(
        f"bounds_{'topdown' if top_down else 'bottomup'}_s{the_seed}_o{num_objects}.png"
    )


main()
