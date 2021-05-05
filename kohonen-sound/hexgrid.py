import math
from numba import jit
import numpy as np
from typing import Optional, Tuple

import colorcet as cc
from tqdm import trange

from plot import plot_map


class HexGrid:
    def __init__(
        self,
        columns: int,
        rows: int,
        vector_length: int,
        dtype: Optional[np.dtype] = np.float32,
    ):
        if (rows & 1) or (not columns & 1):
            raise ValueError(
                "Hexagonal grids can only be created with even height and odd width"
            )

        self.width: int = columns
        self.height: int = rows
        self.vector_length: int = vector_length
        self.dtype = dtype
        self.dirty = True

        self.grid = {
            "centers": np.array(
                [item for y in range(self.height) for item in self._row(y, self.width)],
                dtype=int,
            ),
            "x": np.array([self.width], dtype=int),
            "y": np.array([self.height], dtype=int),
        }

        self.u_height = self.height * 2
        self.u_width = self.width * 2 + 1
        self.umatrix_grid = {
            "centers": np.array(
                [
                    item
                    for y in range(self.u_height)
                    for item in self._row(y, self.u_width)
                ],
                dtype=int,
            ),
            "x": np.array([self.u_width], dtype=int),
            "y": np.array([self.u_height], dtype=int),
        }
        self.umatrix_values = np.zeros((self.u_height, self.u_width))

        # Distance matrix template to use for filtering influence et c.
        self._create_distance_matrix(self.width, self.height)

    def _create_distance_matrix(self, width: int, height: int):
        # Offset coordinates, twice the size to put origo in the middle
        col, row = np.meshgrid(
            np.arange(2 * height), np.arange(2 * width), indexing="ij"
        )

        @jit(nopython=True, fastmath=True)
        def meshes_to_dist(
            col: np.ndarray, row: np.ndarray, width: int, height: int
        ) -> np.ndarray:
            # Convert to cubic coordinates
            XX = col - (row + (row & 1)) // 2
            ZZ = row
            YY = -XX - ZZ

            # Calculate distances
            dx = np.abs(XX - XX[height - 1, width - 1])
            dy = np.abs(YY - YY[height - 1, width - 1])
            dz = np.abs(ZZ - ZZ[height - 1, width - 1])
            d = np.maximum(np.maximum(dx, dy), dz)
            d = d[height // 2 : height + height // 2, width // 2 : width + width // 2]

            return d

        dist = meshes_to_dist(col, row, width, height)

        self.distance_matrix = np.roll(
            np.roll(dist, -(height // 2 - 1), axis=0), -(width // 2), axis=1
        )

    def neighbours(self, x: int, y: int, max_distance: int = 1) -> set:
        dist = np.roll(np.roll(self.distance_matrix, y, axis=0), x, axis=1)

        idx = np.nonzero(dist <= max_distance)
        return np.stack((*idx, dist[idx])).T

    @jit(nopython=True)
    def _offset_to_cube(self, col: int, row: int) -> Tuple[int, int, int]:
        x = col - (row + (row & 1)) // 2
        z = row
        y = -x - z
        return (x, y, z)

    @jit(nopython=True)
    def _cube_to_offset(self, x: int, y: int, z: int) -> Tuple[int, int]:
        col = x + (z + (z & 1)) // 2
        row = z

        return (col % self.width, row % self.height)

    @jit(nopython=True, fastmath=True)
    def distance(self, x, y, fx, fy):
        if abs(x - fx) > self.width / 2:
            if x > fx:
                x -= self.width
            else:
                fx -= self.width
        if abs(y - fy) > self.height / 2:
            if y > fy:
                y -= self.height
            else:
                fy -= self.height

        a_x, a_y, a_z = self._offset_to_cube(x, y)
        b_x, b_y, b_z = self._offset_to_cube(fx, fy)
        d_x = abs(a_x - b_x)
        d_y = abs(a_y - b_y)
        d_z = abs(a_z - b_z)
        return max(d_x, d_y, d_z)

    def nearest(self, x: int, y: int) -> set:
        """
        Return a list of the offsets for all neighbours of the hexagon at position x, y:

        Odd rows:
         2     (-1,+1), ( 0,+1)
         1  (-1, 0)   x,y  (+1, 0)
         0     (-1,-1), ( 0,-1)

        Even rows:
         3     ( 0,+1), (+1,+1)
         2  (-1, 0)   x,y  (+1, 0)
         1     ( 0,-1), (+1,-1)
        """

        if y & 1:
            return {
                ((x - 1) % self.width, (y + 1) % self.height),
                ((x + 0) % self.width, (y + 1) % self.height),
                ((x - 1) % self.width, (y + 0) % self.height),
                ((x + 1) % self.width, (y + 0) % self.height),
                ((x - 1) % self.width, (y - 1) % self.height),
                ((x + 0) % self.width, (y - 1) % self.height),
            }
        else:
            return {
                ((x + 0) % self.width, (y + 1) % self.height),
                ((x + 1) % self.width, (y + 1) % self.height),
                ((x - 1) % self.width, (y + 0) % self.height),
                ((x + 1) % self.width, (y + 0) % self.height),
                ((x + 0) % self.width, (y - 1) % self.height),
                ((x + 1) % self.width, (y - 1) % self.height),
            }

    @classmethod
    def _row(cls, y, width):
        offs = 0 if y & 1 else 0.5

        y = math.sqrt(3) * (0.5 + y / 2)
        return [[x + offs, y] for x in range(1, width + 1)]

    def plot(
        self,
        weights: np.ndarray,
        index: Optional[int] = None,
        filename: Optional[str] = None,
        title: Optional[str] = None,
    ):
        if index is None and weights is None:
            raise ValueError("Either index or weights must be set for plot to work")

        if index is not None and weights is not None:
            raise ValueError("Either index or weights must be None for plot to work")

        if index is not None:
            d_matrix = weights[:, :, index].reshape((self.width * self.height))

        else:
            d_matrix = weights.reshape((self.width * self.height))

        arguments = {
            "cmap": cc.cm.coolwarm,
        }

        if title:
            arguments["title"] = title

        # All this junk should be done in plot_map instead
        plt, fig = plot_map(
            self.grid,
            d_matrix,
            **arguments,
        )

        if filename is None:
            plt.show()
        else:
            fig.savefig(filename)
            plt.draw()
            plt.clf()

        plt.close(fig)

    def plot_u_matrix(
        self,
        weights: np.ndarray,
        filename: Optional[str] = None,
        title: Optional[str] = None,
    ):
        """
        Calculate the U-Matrix if necessary, and save it to a file if <filename>
        is provided, or plot it to screen if <filename> is None.

        Normally the matrix should be twice the size of the weight planes in both
        X and Y dimension, with extra hexagons inserted between the weight nodes.
        The value of those extra hexagons should be the distance between the two
        weight nodes directly neighbouring them, while the hexagons represending
        the existing weight nodes should show the value of their average distance
        to their neighbouring weight nodes.

        The code below is a simplification which only shows the average distance
        of the weight nodes themselves, and not the extra connecting hexagons.
        """
        for y in trange(
            self.umatrix_values.shape[0] // 2,
            desc="Calculating U-matrix",
            position=1,
            leave=False,
        ):
            for x in range(self.umatrix_values.shape[1] // 2):
                neighbours = self.nearest(x, y)
                dist = 0
                for n_x, n_y in neighbours:
                    dist += np.sqrt(
                        np.sum(np.square(weights[y, x] - weights[n_y, n_x]))
                    )

                self.umatrix_values[y, x] = dist / len(neighbours)

        arguments = {
            "weights": self.umatrix_values[: self.height, : self.width],
            "filename": filename,
            "title": title if title else "U-Matrix",
        }

        self.plot(**arguments)

        # https://stackoverflow.com/questions/13631673/how-do-i-make-a-u-matrix
        # if self.dirty:
        #     # Update values here
        #     for y in range(self.umatrix_values.shape[0] // 2):
        #         for x in range(self.umatrix_values.shape[1] // 2):
        #             if not x & 1 and y & 1:
        #                 x_u = x * 2 + 1
        #             else:
        #                 x_u = x * 2
        #
        #                 self.umatrix_values[y * 2, x * 2] = self.weights[y, x, 0]
        #     self.dirty = False
        #
        # # return self.umatrix_values
        # plt = plot_map(
        #     self.umatrix_grid,
        #     self.umatrix_values.reshape(
        #         (self.umatrix_values.shape[0] * self.umatrix_values.shape[1])
        #     ),
        #     # cmap=cc.cm.CET_L2,
        #     cmap=cc.cm.coolwarm,
        # )
        #
        # if filename is None:
        #     plt.show()
        # else:
        #     plt.savefig(filename)
