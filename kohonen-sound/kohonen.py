import concurrent.futures
import json
import math
import multiprocessing
from numba import jit
import numpy as np
import random
import shutil
from typing import Any, Callable, List, Optional, Tuple, Union

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

    def __next__(self):
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
        init_policy: str = "random",  # ["random" -> random values from 0.0 to 1.0, "sampling"]
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
            if init_policy == "random":
                print(f"Create random weight init, {rows} x {columns}")
                self.weights = (
                    data_source.random(rows * columns)
                    .reshape((rows, columns, -1))
                    .astype(self.dtype)
                )
                print(f"Shape: {self.weights.shape}")
            elif init_policy == "sampling":
                sample_indices = random.sample(range(len(data_source)), rows * columns)
                self.weights = np.zeros(rows * columns * self.vector_length).reshape(
                    (rows, columns, -1)
                )

                i = 0
                for y in range(rows):
                    for x in range(columns):
                        self.weights[y, x] = data_source[sample_indices[i]]
                        i += 1
            else:
                ValueError(f"Unknown intialization policy: '{init_policy}'")

    # @jit(nopython=True)
    def best_matching_unit(self, targets: DataSource, index: int):
        """
        Return the XY coordinates of the unit closest to the target specified
        by <index> into <targets>.
        """
        bmu = targets.distance_squared(targets[index], self.weights).argmin()
        y = bmu // self.width
        x = bmu - y * self.width

        return (x, y)

    def best_matching_value(self, targets: DataSource, index: int):
        x, y = self.best_matching_unit(targets, index)
        return self.weights[y, x]

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
        skip_indices: List[int] = []

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
            target_length = len(targets)

            def gen_indices(
                batch_size: int, skip_indices: List[int]
            ) -> Tuple[List[int], int]:
                # Pick targets randomly from target list
                if batch_size:
                    indices = list(
                        np.random.permutation(target_length).astype(int)[:batch_size]
                    )
                    if len(skip_indices):
                        print(f"Skipping {skipped} finished samples.")
                        indices = list(set(indices) - set(skip_indices))[
                            : batch_size - skipped
                        ]
                    total = batch_size

                else:
                    if len(skip_indices):
                        print(f"Skipping {skipped} finished samples.")
                        indices = list(set(range(target_length)) - set(skip_indices))
                    else:
                        indices = list(range(target_length))
                    random.shuffle(indices)
                    total = len(indices)

                return indices, total

            indices, total = gen_indices(batch_size, skip_indices)

            print(f"Training on batch of {len(indices)} samples, step is {step}")
            for ti in tqdm(
                indices, initial=skipped, desc="Training samples", total=total
            ):
                # Decrease learning rate and area of influence after each iteration
                lr = learning_rate * math.exp(
                    -step
                    / (max_iterations * (batch_size if batch_size else target_length))
                )
                rad = radius * (
                    1
                    - step
                    / (
                        1.2
                        * (
                            max_iterations
                            * (batch_size if batch_size else target_length)
                        )
                    )
                )
                inv_rad = 1.0 / rad
                target = targets[ti]

                # Find best matching unit in network
                bmu = targets.distance_squared(target, self.weights).argmin()
                y = bmu // self.width
                x = bmu - y * self.width

                # Update the surrounding neighbourhood to make them closer to the target
                neighbourhood = hg.neighbours(x, y, rad)

                for n_y, n_x, dist in neighbourhood:
                    distance = dist
                    # Roughly 0.01 at sqRadius, bell shaped curve centered around 0.0
                    d = distance * inv_rad
                    infl = 2.0 / (1.0 + np.exp(5.33 * d * d))

                    # Move the neuron slightly towards the target
                    self.weights[n_y, n_x] += (
                        (target - self.weights[n_y, n_x]) * lr * infl
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

                if (
                    weights_file
                    and steps_between_saves
                    and (steps_between_saves == 1 or (step % steps_between_saves == 0))
                ):
                    try:
                        shutil.copy(weights_file, f"{weights_file}.bup")
                    except FileNotFoundError:
                        pass
                    self.save(weights_file, suppress=True)

                # Keep track of trained indices but only after weights have been saved
                # so the state shows what's been saved.
                skip_indices.append(int(ti))

                # Save current state
                if (
                    state_filename
                    and steps_between_saves
                    and (steps_between_saves == 1 or (step % steps_between_saves == 0))
                ):
                    try:
                        shutil.copy(state_filename, f"{state_filename}.bup")
                    except FileNotFoundError:
                        pass

                    with open(state_filename, "w") as f:
                        json.dump(
                            {
                                "rows": self.height,
                                "columns": self.width,
                                "vector_length": self.vector_length,
                                "data_type": str(self.dtype),
                                "initial_learning_rate": learning_rate,
                                "initial_radius": radius,
                                "current_learning_rate": lr,
                                "current_radius": rad,
                                "current_iteration": i,
                                "batch_progress": len(skip_indices),
                                "last_step": step,
                                "indices_trained": skip_indices,
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
