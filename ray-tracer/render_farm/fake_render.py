import json
from random import randint
import requests
import sys
import time

import numpy as np
from tqdm import tqdm


def get_gradient_2d(start, stop, width, height, is_horizontal):
    if is_horizontal:
        return np.tile(np.linspace(start, stop, width), (height, 1))
    else:
        return np.tile(np.linspace(start, stop, height), (width, 1)).T


def get_gradient_3d(width, height, start_list, stop_list, is_horizontal_list):
    result = np.zeros((height, width, len(start_list)), dtype=np.float)

    for i, (start, stop, is_horizontal) in enumerate(
        zip(start_list, stop_list, is_horizontal_list)
    ):
        result[:, :, i] = get_gradient_2d(start, stop, width, height, is_horizontal)

    return result


scene_hash = None
image_params = None
target_image = None


def run(url):
    global scene_hash
    global image_params
    global target_image

    job_url = f"{url}/job/two_spheres_references"
    scene_url = f"{url}/job/two_spheres_references/scene"

    response = requests.get(job_url)
    if response.status_code != 200:
        print("Unexpected job response:", response)
        sys.exit(1)

    data = response.json()
    if data["status"] == "paused":
        time.sleep(30)
        return
    elif data["status"] == "done":
        print("Rendering is done")
        sys.exit(0)
    elif data["status"] == "rendering":
        pass
    else:
        print(f"Unknown server status: '{data['status']}'")
        sys.exit(1)

    num_samples = data["samples"]
    reference = data["reference"]
    section = data["section"]
    if scene_hash != data["scene"]:
        scene_hash = data["scene"]

        response = requests.get(f"{scene_url}/{scene_hash}")
        if response.status_code != 200:
            print("Unexpected scene response:", response)
            sys.exit(1)

        scene = response.json()
        image_params = scene["image"]

        height = image_params["height"]
        width = image_params["width"]

        target_image = get_gradient_3d(
            width,
            height,
            (0, 0, 192),
            (255, 255, 64),
            (True, False, False),
        )

    height = image_params["height"]
    width = image_params["width"]
    section_width = section["x1"] - section["x0"]
    section_height = section["y1"] - section["y0"]

    target_section = target_image[
        section["y0"] : section["y1"], section["x0"] : section["x1"]
    ]

    samples_odd = target_section + (
        np.random.rand(section_height, section_width, 3) - 0.5
    ) * section["x0"] * section["y0"] / (width * height)
    samples_even = target_section + (
        np.random.rand(section_height, section_width, 3) - 0.5
    ) * section["x0"] * section["y0"] / (width * height)
    counts = np.zeros((section_height, section_width))

    for y in range(section_height):
        for x in range(section_width):
            count = randint(num_samples / 2, 5 * num_samples)
            samples_odd[y, x] *= count
            samples_even[y, x] *= count
            counts[y, x] = count * 2

    retval = {
        "reference": reference,
        "data": {
            "samples_odd": samples_odd.astype(np.int64)
            .reshape(section_height * section_width * 3)
            .tolist(),
            "samples_even": samples_even.astype(np.int64)
            .reshape(section_height * section_width * 3)
            .tolist(),
            "counts": counts.astype(np.int64)
            .reshape(section_height * section_width)
            .tolist(),
        },
    }
    response = requests.post(job_url, json=retval)
    # print(response)


def main():
    num_passes = int(sys.argv[1]) if len(sys.argv) > 1 else 10000
    url = sys.argv[2] if len(sys.argv) > 2 else "https://alkanen.no-ip.biz/render"

    for i in tqdm(range(num_passes)):
        run(url)


if __name__ == "__main__":
    main()
