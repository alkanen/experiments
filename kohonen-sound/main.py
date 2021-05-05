# Since there's obviously some kind of memory leak, try this:
# https://www.fugue.co/blog/diagnosing-and-fixing-memory-leaks-in-python.html
# There's also an endless recursion bug in _recurse_neighbours for some reason.

import argparse
import math

import numpy as np

from kohonen import KohonenSom
from soundfile import SoundFile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--load-weights",
        help="Numpy file containing weights to load.",
        type=str,
    )
    parser.add_argument(
        "--store-weights",
        help="Numpy file to store trained weights into.",
        type=str,
    )
    parser.add_argument(
        "--train-data",
        help="WAV or MP3 file with sound data to train from.",
        type=str,
        default="nightwatch.wav",
    )
    parser.add_argument(
        "--width",
        help="Width of the Kohonen network.  Must be an odd number.",
        type=int,
        default=11,
    )
    parser.add_argument(
        "--height",
        help=(
            "Height of the Kohonen network.  Must be even number and is computed from "
            "width if omitted."
        ),
        type=int,
        default=None,
    )
    parser.add_argument(
        "--learning-rate",
        "-l",
        help="Initial learning rate.",
        type=float,
        default=0.3,
    )
    parser.add_argument(
        "--radius",
        "-r",
        help="Initial neighbourhood radius, based on width and height if omitted.",
        type=float,
    )
    parser.add_argument(
        "--state",
        "-s",
        help="State file used for resuming training et c.",
        default="state.json",
        type=str,
    )
    parser.add_argument(
        "--batch-size",
        "-b",
        help=(
            "Number of samples to use in each iteration.  Defaults to entire training "
            "set if not specified."
        ),
        default=None,
        type=int,
    )
    parser.add_argument(
        "--num-iterations",
        "-n",
        help="Number of training iterations. Defaults to 100.",
        default=1,
        type=int,
    )
    parser.add_argument(
        "--ms-per-sample",
        "-m",
        help="Number of miliseconds per sample vector",
        default=50,
        type=int,
    )
    args = parser.parse_args()
    if args.store_weights is None:
        print("Warning, --store-weights has not been set, no progress will be saved.")

    width = args.width
    if not width & 1:
        width += 1
    # 2 * w / sqrt(3) to make a square grid
    # 1.125 * w / sqrt(3) to make roughly 16:9
    if args.height is None:
        height = int(1.125 * width / math.sqrt(3))
    else:
        height = args.height

    if height & 1:
        height += 1

    if args.load_weights:
        try:
            with open(args.load_weights, "r") as f:
                pass
        except FileNotFoundError:
            print(f"Unable to open {args.load_weights}, generating new weights.")
            args.load_weights = None

    if args.train_data:
        data_source = SoundFile(args.train_data, args.ms_per_sample)

        som = KohonenSom(
            columns=width,
            rows=height,
            data_source=data_source,
            dtype=np.float32,
            weights=args.load_weights,
        )

        samples = data_source[10000:10004]
        print(samples)
        print(samples.shape)
        print(samples.dtype)

        som.train(
            max_iterations=args.num_iterations,
            batch_size=args.batch_size,
            targets=data_source,
            learning_rate=args.learning_rate,
            radius=args.radius,
            image_template=(
                "images/"
                "u_matrix_{step:08d}"
                "_iter{iteration:07d}"
                "_lr{learning_rate:.6f}"
                "_rad{radius:05.2f}"
                "_idx{index:07d}"
                ".png"
            ),
            state_filename=args.state,
            weights_file=args.store_weights,
            steps_between_saves=5,
            steps_between_render=30,
            steps_between_dumps=1000,
            iterations_between_dumps=1,
        )


if __name__ == "__main__":
    main()
