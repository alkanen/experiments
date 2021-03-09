from datetime import datetime, timedelta
import hashlib
from io import BytesIO
import json
from pathlib import Path
from random import randint
from typing import Union

import numpy as np
from PIL import Image


def create_folder(filename: Union[str, Path]):
    Path(filename).parent.mkdir(parents=True, exist_ok=True)


class JobsHandler:
    def __init__(
        self,
        folder: Path,
        timeout: int,
        num_samples: int,
        camera: dict,
        image: dict,
        textures: dict,
        materials: dict,
        objects: dict,
        variance_limit=1e-3,
        section_height=3,
        section_width=3,
    ):
        # Default number of samples to request
        self.num_samples = num_samples
        self.timeout = timeout
        self.camera = camera
        self.image = image
        self.textures = textures
        self.materials = materials
        self.objects = objects
        self.variance_limit = variance_limit
        self.scene_hash = hashlib.sha224(
            json.dumps(
                {
                    "camera": camera,
                    "image": image,
                    "textures": textures,
                    "materials": materials,
                    "objects": objects,
                },
                sort_keys=True,
            ).encode("utf8")
        ).hexdigest()

        width = image["width"]
        height = image["height"]

        self.files = {
            "samples_odd": Path(folder) / "samples_odd.npy",
            "samples_even": Path(folder) / "samples_even.npy",
            "heatmap": Path(folder) / "heatmap.npy",
            "status": Path(folder) / "status.json",
        }

        # Rendering meta data
        try:
            with open(self.files["status"], "r") as f:
                tmp = json.load(f)
                self.unfinished = tmp["unfinished"]
                self.dormant = tmp["dormant"]
                self.done = tmp["done"]
                self.rendering = {}

        except FileNotFoundError:
            # Default to having one job section per scanline for now
            self.unfinished = [
                {
                    "y0": y,
                    "x0": x,
                    "y1": min(y + section_height, height),
                    "x1": min(x + section_width, width),
                }
                for y in range(0, height - section_height, section_height)
                for x in range(0, width - section_width, section_width)
            ]
            self.dormant = []
            self.done = []
            self.rendering = {}

        # Samples and related data
        self.data = {}
        try:
            self.data["samples_odd"] = np.load(self.files["samples_odd"])
        except OSError as e:
            self.data["samples_odd"] = np.zeros((height, width, 3), dtype=np.int64)
        try:
            self.data["samples_even"] = np.load(self.files["samples_even"])
        except OSError:
            self.data["samples_even"] = np.zeros((height, width, 3), dtype=np.int64)
        try:
            self.data["heatmap"] = np.load(self.files["heatmap"])
        except OSError:
            self.data["heatmap"] = np.zeros((height, width), dtype=np.int64)

        create_folder(self.files["samples_odd"])

        self.last_save_time = datetime.now()
        self.last_save_counter = 10

    def save_status(self):
        with open(self.files["status"], "w") as f:
            json.dump(
                {
                    "dormant": self.dormant,
                    "done": self.done,
                    "unfinished": [
                        *self.unfinished,
                        *[self.rendering[key]["section"] for key in self.rendering],
                    ],
                },
                f,
                indent=4,
            )

    def get_status(self):
        return {
            "unfinished": len(self.unfinished),
            "rendering": len(self.rendering),
            "dormant": len(self.dormant),
            "done": len(self.done),
        }

    def get_job(self, client: str):
        # Cleanup
        too_old = datetime.now() - timedelta(seconds=self.timeout)
        clean_list = []
        for ref in self.rendering:
            if self.rendering[ref]["alive"] < too_old:
                clean_list.append(ref)
        for ref in clean_list:
            self.unfinished.append(self.rendering[ref]["section"])
            del self.rendering[ref]

        if len(self.unfinished) == 0:
            self.unfinished = self.dormant
            self.dormant = []

        if len(self.unfinished) == 0:
            if len(self.rendering):
                return {"status": "paused"}
            else:
                return {"status": "done"}

        index = randint(0, len(self.unfinished) - 1)
        tmp = self.unfinished[index]
        del self.unfinished[index]
        reference = hashlib.sha224(
            json.dumps(
                {
                    "client": client,
                    "random1": randint(0, 0xFFFFFFFFFFFFFFFF),
                    "random2": randint(0, 0xFFFFFFFFFFFFFFFF),
                },
                sort_keys=True,
            ).encode("utf8")
        ).hexdigest()

        self.rendering[reference] = {
            "client": client,
            "section": tmp,
            "created": datetime.now(),
            "alive": datetime.now(),
        }

        return {
            "status": "rendering",
            "scene": self.scene_hash,
            "reference": reference,
            "samples": self.num_samples,
            "section": tmp,
        }

    def report_job(self, reference, data):
        section = self.rendering[reference]["section"]
        samples_odd_raw = data["samples_odd"]
        samples_even_raw = data["samples_even"]
        sample_counts_raw = data["counts"]

        y0 = section["y0"]
        y1 = section["y1"]
        x0 = section["x0"]
        x1 = section["x1"]

        samples_odd = np.array(samples_odd_raw, dtype=np.int64).reshape(
            (y1 - y0, x1 - x0, 3)
        )
        samples_even = np.array(samples_even_raw, dtype=np.int64).reshape(
            (y1 - y0, x1 - x0, 3)
        )
        counts = np.array(sample_counts_raw, dtype=np.int64).reshape((y1 - y0, x1 - x0))

        # Update samples:
        self.data["samples_odd"][y0:y1, x0:x1] += samples_odd
        self.data["samples_even"][y0:y1, x0:x1] += samples_even
        # Update heatmap:
        self.data["heatmap"][y0:y1, x0:x1] += counts

        # Check if truly done, does not work whatsoever
        done = np.all(
            np.abs(
                np.divide(
                    (
                        self.data["samples_odd"][y0:y1, x0:x1]
                        - self.data["samples_even"][y0:y1, x0:x1]
                    ).astype(np.dtype("d")),
                    (
                        self.data["samples_odd"][y0:y1, x0:x1]
                        + self.data["samples_even"][y0:y1, x0:x1]
                    ).astype(np.dtype("d")),
                    out=np.zeros_like(self.data["samples_odd"][y0:y1, x0:x1]).astype(
                        np.dtype("d")
                    ),
                    where=(
                        self.data["samples_odd"][y0:y1, x0:x1]
                        + self.data["samples_even"][y0:y1, x0:x1]
                    )
                    != 0,
                )
            )
            < self.variance_limit
        )
        if done:
            self.done.append(section)
        else:
            self.dormant.append(section)
        del self.rendering[reference]

        self.last_save_counter -= 1
        if (
            self.last_save_counter <= 0
            or (datetime.now() - self.last_save_time).total_seconds() > 600
        ):
            self.last_save_counter = 10
            self.last_save_time = datetime.now()

            np.save(self.files["samples_odd"], self.data["samples_odd"])
            np.save(self.files["samples_even"], self.data["samples_even"])
            np.save(self.files["heatmap"], self.data["heatmap"])
            self.save_status()

        return "OK"

    def get_scene(self, scene_hash: str):
        if scene_hash != self.scene_hash:
            return "Invalid scene hash", 400

        return {
            "camera": self.camera,
            "image": self.image,
            "textures": self.textures,
            "materials": self.materials,
            "objects": self.objects,
        }

    def _create_image(self, which: str):
        mem = BytesIO()
        if which == "samples":
            data = self.data["samples_odd"] + self.data["samples_even"]
            tmp_count = np.stack((self.data["heatmap"],) * 3, -1)
            data = np.divide(
                data,
                tmp_count,
                out=np.zeros_like(data),
                where=tmp_count != 0,
                casting="unsafe",
            )
        else:
            # data = np.log(self.data["heatmap"] + 1e-9)
            data = np.sqrt(self.data["heatmap"] + 1e-9)

        maxval = np.max(data)
        minval = np.min(data)
        # We want to allow total blackness
        if minval > 0:
            minval = 0
        scalefactor = ((255.0 - minval) / (maxval - minval)) if maxval else 255.0

        data = (data * scalefactor).astype("uint8")
        mode = "RGB"
        if which != "samples":
            mode = "L"

        im = Image.fromarray(data, mode=mode)
        im.save(mem, format="png")
        mem.seek(0)

        return mem

    def get_image(self):
        return self._create_image("samples")

    def get_heatmap(self):
        return self._create_image("heatmap")
