from typing import Any, Union

from numba import jit
import numpy as np
from pydub import AudioSegment

from kohonen import DataSource


class SoundFile(DataSource):
    def __init__(
        self,
        filename: str,
        ms_per_sample: int = 50,
        ms_between_samples: int = 5,
        dtype: np.dtype = np.float32,
    ):
        super(SoundFile, self).__init__()
        self._filename = filename
        self.ms_per_sample = ms_per_sample
        self.ms_between_samples = ms_between_samples
        self.dtype = dtype

        # Replace these lines with just AudioSegment.from_file()?
        if filename.endswith(".wav"):
            a = AudioSegment.from_wav(self._filename)
        elif filename.endswith(".mp3"):
            a = AudioSegment.from_mp3(self._filename)
        else:
            raise ValueError(f"Unknown filetype: '{filename}'")

        if a.channels != 1:
            raise ValueError("Only mono files are supported at the moment.")

        self._sample_rate = a.frame_rate
        self._values_per_ms = self._sample_rate / 1000
        self._sample_length = int(round(self.ms_per_sample * self._values_per_ms))
        self._values_between_samples = int(
            round(self.ms_between_samples * self._values_per_ms)
        )

        samples = a.get_array_of_samples()
        # Only handle full samples
        end = len(samples) - (len(samples) % self._values_between_samples)
        self._data = np.array(samples[:end], dtype=self.dtype) / (
            2 ** (8 * a.sample_width - 1)
        )
        self._length = (end - self.sample_length) // self._values_between_samples

    @property
    def sample_length(self):
        return self._sample_length

    @property
    def shape(self):
        return (self._length, self.sample_length)

    def random(self, samples: int = 1):
        return np.random.random((samples, self.sample_length)).astype(self.dtype)

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
        # for sample in data:
        #     yield sample
        raise NotImplementedError

    def __len__(self):
        return self._length

    def __getitem__(self, key) -> Union[np.ndarray, int]:
        if isinstance(key, slice):
            start = (key.start if key.start is not None else 0) * (
                self._values_between_samples
            )
            stop = (key.stop if key.stop is not None else self._length) * (
                self._values_between_samples
            )
            step = (key.step if key.step is not None else 1) * (
                self._values_between_samples
            )

            return np.array(
                [
                    self._data[i : i + self.sample_length]
                    for i in range(start, stop, step)
                ]
            )

        else:  # isinstance(key, int):
            try:
                new_key = int(key)
                return self._data[
                    new_key
                    * self._values_between_samples : new_key
                    * self._values_between_samples
                    + self.sample_length
                ]
            except ValueError:
                raise IndexError(
                    "Index must be integer or slice of integers, not", type(key)
                ) from None

    def __contains__(self, key):
        raise NotImplementedError
