from typing import List, Tuple, Union

from numba import jit
import numpy as np
from PIL import Image

from kohonen import DataSource


class ImageSource(DataSource):
    def __init__(
        self,
        filename: str,
        dtype: np.dtype = np.float32,
    ):
        super(ImageSource, self).__init__()
        image = Image.open(filename).convert("RGB")
        self._data = np.asarray(image) / 255.0
        self._shape = self._data.shape
        self._sample_length = 3
        self._length = self._shape[0] * self._shape[1]

    @property
    def sample_length(self):
        return self._sample_length

    @property
    def shape(self):
        return self._shape

    def random(self, samples: int = 1):
        return np.random.random((samples, self.sample_length))

    @jit(nopython=True)
    def distance(self, sample: np.ndarray, weights: np.ndarray) -> np.ndarray:
        """Return the distance between a sample and all weight vectors."""
        return np.sqrt(self.distance_squared(sample, weights))

    @jit(forceobj=True)
    def distance_squared(self, sample: np.ndarray, weights: np.ndarray) -> np.ndarray:
        """Return the square of the distance between a sample and all weight vectors."""
        return np.sum(
            np.square(sample - weights.reshape((-1, self.sample_length))),
            axis=1,
        )

    def __iter__(self):
        return self

    def __next__(self):
        for i in range(len(self)):
            yield self[i]

        raise StopIteration

    def __len__(self):
        return self._length

    def __getitem__(self, key) -> Union[np.ndarray, int]:
        if isinstance(key, tuple):
            if len(key) != 2:
                raise IndexError(
                    "Use one index value to index all pixels as an array, or two index "
                    "values to index by height and width."
                )
            y = key[0]
            x = key[1]

            return self._data[y, x]

        if isinstance(key, slice):
            start = key.start if key.start is not None else 0
            stop = key.stop if key.stop is not None else self._length
            step = key.step if key.step is not None else 1

            return np.array(
                [self._data[i] for i in range(start, stop, step)], dtype=self.dtype
            )

        else:  # isinstance(key, int):
            try:
                new_key = int(key)
                y = new_key // self._data.shape[1]
                x = new_key - (y * self._data.shape[1])

                if y >= self._data.shape[0] or x >= self._data.shape[1]:
                    raise IndexError(
                        f"index {new_key} is out of bounds for data with size "
                        f"{self._data.shape[0] * self._data.shape[1]} "
                        f"({self._data.shape[0]} x {self._data.shape[1]})"
                    )

                return self._data[y, x]
            except ValueError:
                raise IndexError(
                    "Index must be integer or slice of integers, not", type(key)
                ) from None

    def __contains__(self, key):
        raise NotImplementedError
