import json
from random import randint
import requests
import sys

import numpy as np


def main():
    url = sys.argv[1] if len(sys.argv) > 1 else "https://alkanen.no-ip.biz/render"

    job_url = f"{url}/job/two_spheres_references"
    scene_url = f"{url}/job/two_spheres_references/scene"

    response = requests.get(job_url)
    if response.status_code != 200:
        print("Unexpected job response:", response)
        sys.exit(1)

    data = response.json()
    scene_hash = data["scene"]
    num_samples = data["samples"]
    reference = data["reference"]
    section = data["section"]

    response = requests.get(f"{scene_url}/{scene_hash}")
    if response.status_code != 200:
        print("Unexpected scene response:", response)
        sys.exit(1)

    scene = response.json()
    image_params = scene["image"]
    print("Image:", image_params)
    print("Section:", section)

    width = section["x1"] - section["x0"]
    height = section["y1"] - section["y0"]

    samples_odd = np.random.rand(height, width, 3)
    samples_even = np.random.rand(height, width, 3)
    counts = np.zeros((height, width))

    for y in range(height):
        for x in range(width):
            count = randint(num_samples / 2, 5 * num_samples)
            samples_odd[y, x] *= 255 * count
            samples_even[y, x] *= 255 * count
            counts[y, x] = count * 2

    retval = {
        "reference": reference,
        "data": {
            "samples_odd": samples_odd.astype(np.int64)
            .reshape(height * width * 3)
            .tolist(),
            "samples_even": samples_even.astype(np.int64)
            .reshape(height * width * 3)
            .tolist(),
            "counts": counts.astype(np.int64).reshape(height * width).tolist(),
        },
    }
    response = requests.post(job_url, json=retval)
    print(response)


if __name__ == "__main__":
    main()
