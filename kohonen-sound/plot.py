# Possible fix for mem leak?
# https://stackoverflow.com/a/63062218/5297811

import colorcet as cc
import matplotlib.pyplot as plt

# from matplotlib import cm
from matplotlib.collections import RegularPolyCollection
from mpl_toolkits.axes_grid1 import make_axes_locatable
import math
import numpy as np


def plot_map(
    grid, d_matrix, w=1080, dpi=72.0, title="SOM Hit map", cmap=None, colors=None
):
    """
    Plot hexagon map where each neuron is represented by a hexagon. The hexagon
    color is given by the distance between the neurons (D-Matrix)

    Args:
    - grid: Grid dictionary (keys: centers, x, y ),
    - d_matrix: array contaning the distances between each neuron
    - w: width of the map in inches
    - title: map title

    Returns the Matplotlib SubAxis instance
    """
    n_centers = grid["centers"]
    x, y = grid["x"], grid["y"]
    # Size of figure in inches
    xinch = (x * w / y) / dpi
    yinch = (y * w / x) / dpi
    fig = plt.figure(figsize=(xinch, yinch), dpi=dpi)
    ax = fig.add_subplot(111, aspect="equal")
    # Get pixel size between to data points
    xpoints = n_centers[:, 0]
    ypoints = n_centers[:, 1]
    ax.scatter(xpoints, ypoints, s=0.0, marker="s")
    ax.axis(
        [min(xpoints) - 1.0, max(xpoints) + 1.0, min(ypoints) - 1.0, max(ypoints) + 1.0]
    )
    xy_pixels = ax.transData.transform(np.vstack([xpoints, ypoints]).T)
    xpix, ypix = xy_pixels.T

    # In matplotlib, 0,0 is the lower left corner, whereas it's usually the
    # upper right for most image software, so we'll flip the y-coords
    width, height = fig.canvas.get_width_height()
    ypix = height - ypix

    # discover radius and hexagon
    apothem = 0.9 * (xpix[1] - xpix[0]) / math.sqrt(3)
    area_inner_circle = math.pi * (apothem ** 2)
    arguments = {
        "numsides": 6,  # a hexagon
        "rotation": 0,
        "sizes": (area_inner_circle,),
        "edgecolors": (0, 0, 0, 1),
        "array": d_matrix,
        "offsets": n_centers,
        "transOffset": ax.transData,
    }

    if cmap is not None:
        arguments["cmap"] = cmap
    # if colors is not None:
    #    arguments["facecolor"] = colors

    collection_bg = RegularPolyCollection(**arguments)
    if colors is not None:
        collection_bg.set_facecolor(colors[4])

    ax.add_collection(collection_bg, autolim=True)

    ax.axis("off")
    ax.autoscale_view()
    ax.set_title(title)
    divider = make_axes_locatable(ax)
    cax = divider.append_axes("right", size="10%", pad=0.05)
    plt.colorbar(collection_bg, cax=cax)

    return plt, fig


# For writing patches with colors specified for each patch:
# https://stackoverflow.com/questions/28782390/matplotlib-regularpolycollection-with-static-data-like-sizes
