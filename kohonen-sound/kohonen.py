import concurrent.futures
import json
import math
import multiprocessing
import numpy as np
import random
import shutil
from typing import Any, Optional, Union

from tqdm import tqdm

from hexgrid import HexGrid


class DataSource:
    def __init__(self):
        pass

    def shape(self):
        """Return the shape of the entire sample space."""
        raise NotImplementedError

    def random(self, samples: int = 1):
        """Generate random samples, suitable for intializing a network node. NOT a
        randomly selected sample from the sample space.
        """
        raise NotImplementedError

    def distance(self, sample: Any, weights: np.ndarray) -> np.ndarray:
        """Return the distance between a sample and all weight vectors."""
        raise NotImplementedError

    def distance_squared(self, sample: Any, weights: np.ndarray) -> np.ndarray:
        """Return the square of the distance between a sample and all weight vectors."""
        raise NotImplementedError

    def __iter__(self):
        """Iterate through the sample space."""
        raise NotImplementedError

    def __len__(self):
        """Number of samples in the sample space."""
        raise NotImplementedError

    def __getitem__(self, key):
        """Get a specific sample, or slice of samples."""
        raise NotImplementedError

    def __contains__(self, key):
        """Not really used for anything right now?"""
        raise NotImplementedError


class KohonenSom:
    def __init__(
        self,
        columns: int,
        rows: int,
        data_source: DataSource,
        weights: Optional[Union[np.ndarray, str]] = None,
        dtype: Optional[np.dtype] = np.float32,
    ):
        self.width = columns
        self.height = rows
        self.vector_length = data_source.sample_length
        self.dtype = dtype
        self.hexgrid = HexGrid(
            columns=columns,
            rows=rows,
            vector_length=self.vector_length,
            dtype=dtype,
        )

        if weights:
            if isinstance(weights, str):
                print(f"Loading weights from {weights}")
                weights = np.load(weights, allow_pickle=False)
                print("  done")

            if weights.shape[0] != rows or weights.shape[1] != columns:
                raise ValueError(
                    "Weight data does not correspond to grid dimensions: "
                    f"{weights.shape} != ({rows}, {columns})"
                )

            if weights.dtype != self.dtype:
                self.weights = weights.astype(self.dtype)
            else:
                self.weights = weights

        else:
            # Replace this with datatype.random()
            self.weights = (
                np.random.random((rows, columns, self.vector_length)).astype(
                    dtype=self.dtype
                )
                * 2
                - 1
            )

    def best_matching_unit(self, targets: DataSource, index: int):
        """
        Return the XY coordinates of the unit closest to the target specified
        by <index> into <targets>.
        """
        bmu = targets.distance_squared(targets[index], self.weights).argmin()
        y = bmu // self.width
        x = bmu - y * self.width

        return (x, y)

    def train(
        self,
        targets: DataSource,
        learning_rate: float,
        max_iterations: int = 10,
        image_template: Optional[str] = None,
        state_filename: Optional[str] = None,
        weights_file: Optional[str] = None,
        batch_size: int = 0,
        radius: Optional[float] = None,
        steps_between_saves: Optional[int] = 1,
        steps_between_render: Optional[int] = None,
        steps_between_dumps: Optional[int] = None,
        iterations_between_dumps: Optional[int] = None,
    ):
        hg = self.hexgrid
        hg.dirty = True
        if radius is None:
            radius = max(hg.width, hg.height) // 2

        # Never use all cores
        self.num_threads = max(1, multiprocessing.cpu_count())
        self.executor = concurrent.futures.ThreadPoolExecutor(self.num_threads)

        lr = learning_rate
        rad = radius
        step = 0
        first_iteration = 0
        skip_indices = []

        if state_filename:
            try:
                with open(state_filename, "r") as f:
                    state = json.load(f)

                step = state["last_step"]
                first_iteration = state["current_iteration"]
                skip_indices = state["indices_trained"]

            except FileNotFoundError:
                pass

        if image_template is not None and state_filename is None:
            hg.plot_u_matrix(
                image_template.format(
                    step=step, iteration=0, learning_rate=lr, radius=rad, index=-1
                )
            )

        for i in range(first_iteration, max_iterations):
            print(f"Training iteration {i}/{max_iterations-1}")

            skipped = len(skip_indices)

            # Pick targets randomly from target list
            if batch_size:
                indices = [
                    int(x) for x in np.random.permutation(len(targets))[:batch_size]
                ]
                if skip_indices:
                    indices = list(set(indices) - set(skip_indices))[
                        : batch_size - skipped
                    ]
                total = batch_size

            else:
                if skip_indices:
                    indices = list(set(range(len(targets))) - set(skip_indices))
                else:
                    try:
                        indices = list(range(len(targets)))
                    except TypeError:
                        import pdb

                        pdb.set_trace()
                random.shuffle(indices)
                total = len(indices)

            print(f"Training on batch of {len(indices)} samples, step is {step}")
            for ti in tqdm(
                indices, initial=skipped, desc="Training samples", total=total
            ):
                # Decrease learning rate and area of influence after each iteration
                lr = learning_rate * math.exp(
                    -step
                    / (max_iterations * (batch_size if batch_size else len(targets)))
                )
                rad = radius * (
                    1
                    - step
                    / (
                        1.2
                        * (
                            max_iterations
                            * (batch_size if batch_size else len(targets))
                        )
                    )
                )

                target = targets[ti]

                # Find best matching unit in network
                bmu = targets.distance_squared(target, self.weights).argmin()
                y = bmu // self.width
                x = bmu - y * self.width

                # Update the surrounding neighbourhood to make them closer to the target
                neighbourhoood = hg.neighbours(x, y, rad)
                for n_x, n_y, dist in neighbourhoood:
                    distance = dist
                    influence = self._influence(distance, rad)

                    # Move the neuron slightly towards the target
                    self.weights[n_y, n_x] += (
                        (target - self.weights[n_y, n_x]) * lr * influence
                    )

                if (
                    image_template is not None
                    and steps_between_render is not None
                    and (step % steps_between_render) == 0
                ):
                    self.dirty = True
                    self.plot_u_matrix(
                        image_template.format(
                            step=step,
                            iteration=i,
                            learning_rate=lr,
                            radius=rad,
                            index=ti,
                        )
                    )

                if weights_file and (
                    steps_between_saves == 1 or (step % steps_between_saves == 0)
                ):
                    self.save(weights_file, suppress=True)

                # Keep track of trained indices but only after weights have been saved
                skip_indices.append(ti)

                # Save current state
                if state_filename:
                    try:
                        shutil.copy(state_filename, f"{state_filename}.bup")
                    except FileNotFoundError:
                        pass

                    with open(state_filename, "w") as f:
                        json.dump(
                            {
                                "current_iteration": i,
                                "initial_learning_rate": learning_rate,
                                "current_learning_rate": lr,
                                "last_step": step,
                                "indices_trained": skip_indices,
                                "initial_radius": radius,
                                "current_radius": rad,
                            },
                            f,
                            indent=4,
                        )

                if (
                    weights_file
                    and steps_between_dumps is not None
                    and (step % steps_between_dumps) == 0
                ):
                    self.save(f"{weights_file}_s{step}.npy")

                step += 1

            print(f"BETWEEN ITERATIONS, step: {step}")
            skip_indices = []

            if (
                weights_file
                and iterations_between_dumps is not None
                and (i % iterations_between_dumps) == 0
            ):
                self.save(f"{weights_file}_i{i}.npy")

        if weights_file:
            self.save(f"{weights_file}_final.npy")

    def _influence(self, distance, radius):
        # Roughly 0.01 at sqRadius, bell shaped curve centered around 0.0
        d = distance / radius
        return 2.0 / (1.0 + math.exp(5.33 * d * d))

    def save(self, filename: str, suppress: bool = False):
        if not suppress:
            print(f"\nSaving weights to {filename}")
        np.save(filename, self.weights, allow_pickle=False)

    def plot(
        self,
        index: Optional[int] = None,
        filename: Optional[str] = None,
        title: Optional[str] = None,
    ):
        return self.hexgrid.plot(self.weights, index, filename, title)

    def plot_u_matrix(
        self,
        filename: Optional[str] = None,
        title: Optional[str] = None,
    ):
        return self.hexgrid.plot_u_matrix(self.weights, filename, title)
