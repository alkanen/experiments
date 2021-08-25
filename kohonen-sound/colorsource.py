from typing import List, Tuple, Union

from numba import jit
import numpy as np

from kohonen import DataSource


class ColorSource(DataSource):
    def __init__(
        self,
        colors: Union[List[Tuple[int, int, int]], int],
        dtype: np.dtype = np.float32,
    ):
        super(ColorSource, self).__init__()
        self._length = 3
        self.dtype = dtype

        if type(colors) == int:
            retval = None
            if colors == 1:
                retval = np.array(
                    [
                        # White
                        [1, 1, 1]
                    ]
                )
            elif colors == 2:
                retval = np.array(
                    [
                        # Red
                        [1, 0, 0],
                        # Cyan
                        [0, 1, 1],
                    ]
                )
            elif colors == 3:
                retval = np.array(
                    [
                        # Red
                        [1, 0, 0],
                        # Green
                        [0, 1, 0],
                        # Blue
                        [0, 0, 1],
                    ]
                )
            elif colors == 4:
                retval = np.array(
                    [
                        # Black
                        [0, 0, 0],
                        # Red
                        [1, 0, 0],
                        # Green
                        [0, 1, 0],
                        # Blue
                        [0, 0, 1],
                    ]
                )
            elif colors == 5:
                retval = np.array(
                    [
                        # Black
                        [0, 0, 0],
                        # Red
                        [1, 0, 0],
                        # Green
                        [0, 1, 0],
                        # Blue
                        [0, 0, 1],
                        # White
                        [1, 1, 1],
                    ]
                )
            elif colors == 6:
                retval = np.array(
                    [
                        # Black
                        [0, 0, 0],
                        # Red
                        [1, 0, 0],
                        # Green
                        [0, 1, 0],
                        # Blue
                        [0, 0, 1],
                        # Yellow
                        [1, 1, 0],
                        # White
                        [1, 1, 1],
                    ]
                )
            elif colors == 7:
                retval = np.array(
                    [
                        # Black
                        [0, 0, 0],
                        # Red
                        [1, 0, 0],
                        # Green
                        [0, 1, 0],
                        # Blue
                        [0, 0, 1],
                        # Yellow
                        [1, 1, 0],
                        # Magenta
                        [1, 0, 1],
                        # White
                        [1, 1, 1],
                    ]
                )
            elif colors == 8:
                retval = np.array(
                    [
                        # Black
                        [0, 0, 0],
                        # Red
                        [1, 0, 0],
                        # Green
                        [0, 1, 0],
                        # Blue
                        [0, 0, 1],
                        # Yellow
                        [1, 1, 0],
                        # Magenta
                        [1, 0, 1],
                        # Cyan
                        [0, 1, 1],
                        # White
                        [1, 1, 1],
                    ]
                )
            else:
                retval = np.random.random(colors * 3).reshape(-1, self.sample_length)
            self._data = retval.astype(self.dtype)
        else:
            self._data = list(colors)

    @property
    def sample_length(self):
        return self._length

    @property
    def shape(self):
        return self._data.shape

    def random(self, samples: int = 1):
        return np.random.random((samples, self.sample_length))

    @jit(nopython=True)
    def distance(self, sample: np.ndarray, weights: np.ndarray) -> np.ndarray:
        """Return the distance between a sample and all weight vectors."""
        return np.sqrt(self.distance_squared(sample, weights))

    # @jit(forceobj=True)
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
        return 100000  # self._data.shape[0]

    def __getitem__(self, key) -> Union[np.ndarray, int]:
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
                return self._data[new_key % self._data.shape[0]]
            except ValueError:
                raise IndexError(
                    "Index must be integer or slice of integers, not", type(key)
                ) from None

    def __contains__(self, key):
        raise NotImplementedError
