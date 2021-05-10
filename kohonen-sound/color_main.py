# Since there's obviously some kind of memory leak, try this:
# https://www.fugue.co/blog/diagnosing-and-fixing-memory-leaks-in-python.html
# There's also an endless recursion bug in _recurse_neighbours for some reason.

import argparse
import math
import sys

import numpy as np
from PIL import Image

from kohonen import KohonenSom
from colorsource import ColorSource


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
        default=None,
    )
    parser.add_argument(
        "--palette", help="Number of colors in target palette", type=int, default=8
    )
    parser.add_argument(
        "--classify-data",
        help="WAV or MP3 file with sound data to 'classify' using a trained network.",
        type=str,
    )
    parser.add_argument(
        "--output",
        help="WAV or MP3 file generated from --classify-data and a trained network.",
        type=str,
        default="output.wav",
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
        "--initializer",
        "--initialiser",
        help=(
            "Initialization policy for a newly created SOM. Using 'random' will "
            "initialize a network with entirely random values, whereas 'sampling' "
            "picks random samples from the training data."
        ),
        choices=["random", "sampling"],
        default="random",
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
    parser.add_argument(
        "--ms-between-samples",
        help=(
            "Number of miliseconds to skip from beginning of sample 1 to beginning "
            "of sample 2.  Samples may overlap."
        ),
        default=5,
        type=int,
    )
    args = parser.parse_args()
    if args.train_data and args.classify_data:
        print(args.train_data)
        print(args.classify_data)
        print(
            "Error, cannot set both --train-data and --classify-data at the same time."
        )
        sys.exit(1)

    if args.train_data and args.store_weights is None:
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
                f = f
        except FileNotFoundError:
            print(f"Unable to open {args.load_weights}, generating new weights.")
            args.load_weights = None

    if args.train_data:
        data_source = ColorSource(args.palette)
        print(dir(data_source))
        print(data_source.sample_length)

        som = KohonenSom(
            columns=width,
            rows=height,
            data_source=data_source,
            weights=args.load_weights,
            init_policy=args.initializer,
        )

        som.train(
            max_iterations=args.num_iterations,
            batch_size=args.batch_size,
            targets=data_source,
            learning_rate=args.learning_rate,
            radius=args.radius,
            image_template=(
                "color_images/"
                "u_matrix_{step:08d}"
                "_iter{iteration:04d}"
                "_lr{learning_rate:.6f}"
                "_rad{radius:07.4f}"
                "_idx{index:08d}"
                ".png"
            ),
            state_filename=args.state,
            weights_file=args.store_weights,
            steps_between_saves=100,
            steps_between_render=100,
            steps_between_dumps=10000,
            iterations_between_dumps=1,
        )

        im = Image.fromarray(np.round(som.weights * 255).astype(np.uint8))
        im.save("weights.jpg")

    elif args.classify_data:
        data_source = ColorSource(args.palette)
        som = KohonenSom(
            columns=width,
            rows=height,
            data_source=data_source,
            weights=args.load_weights,
        )
        print("Classifyin!")

        # def to_segment(sample, data_source):
        #     return AudioSegment(
        #         sample.tobytes(),
        #         frame_rate=data_source.sample_rate,
        #         sample_width=data_source.sample_width,
        #         channels=1,
        #     )
        #
        # if args.output.lower().endswith(".wav"):
        #     output_filename = args.output[:-4]
        # else:
        #     output_filename = args.output
        #
        # segment = 1
        # output = None
        # for i in trange(len(data_source)):
        #     sample = som.best_matching_value(data_source, i) * (
        #         2 ** (8 * data_source.sample_width - 1)
        #     )
        #     if output is None:
        #         output = to_segment(sample, data_source)
        #     else:
        #         output += to_segment(sample, data_source)
        #
        #     if i + 1 >= segment * len(data_source) / 10:
        #         filename = f"{output_filename}_{segment:02d}.wav"
        #         print(f"Saving segment to {filename}")
        #
        #         output.export(filename, format="WAV")
        #         segment += 1
        #         output = None
        #
        #         sys.exit(0)


if __name__ == "__main__":
    main()
