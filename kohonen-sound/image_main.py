# Since there's obviously some kind of memory leak, try this:
# https://www.fugue.co/blog/diagnosing-and-fixing-memory-leaks-in-python.html
# There's also an endless recursion bug in _recurse_neighbours for some reason.

import argparse
import math
import sys

import numpy as np
from PIL import Image
from tqdm import trange

from kohonen import KohonenSom
from imagesource import ImageSource


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
        help="Image file train from.",
        type=str,
        default=None,
    )
    parser.add_argument(
        "--classify-data",
        help="Image file to re-render using the 'palette' of a trained network.",
        type=str,
    )
    parser.add_argument(
        "--output",
        help="Image file generated from --classify-data and a trained network.",
        type=str,
        default="output.png",
    )
    parser.add_argument(
        "--dithering",
        "--dither",
        help="Enable Floyd-Steinberg dithering of output image.",
        action="store_true",
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
        default="image_state.json",
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
        print(f"Width has to be odd, increasing to {width}")
    # 2 * w / sqrt(3) to make a square grid
    # 1.125 * w / sqrt(3) to make roughly 16:9
    if args.height is None:
        height = int(1.125 * width / math.sqrt(3))
    else:
        height = args.height

    if height & 1:
        height += 1
        print(f"Height cannot be odd, increasing to {height}")

    if args.load_weights:
        try:
            with open(args.load_weights, "r") as f:
                f = f
        except FileNotFoundError:
            print(f"Unable to open {args.load_weights}, generating new weights.")
            args.load_weights = None

    if args.train_data:
        data_source = ImageSource(args.train_data)
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
            image_template=None,  # (
            #     "image_images/"
            #     "u_matrix_{step:08d}"
            #     "_iter{iteration:04d}"
            #     "_lr{learning_rate:.6f}"
            #     "_rad{radius:07.4f}"
            #     "_idx{index:08d}"
            #     ".png"
            # ),
            state_filename=args.state,
            weights_file=args.store_weights,
            steps_between_saves=None,
            steps_between_render=1200,
            steps_between_dumps=None,
            iterations_between_dumps=1,
        )

        im = Image.fromarray(np.round(som.weights * 255).astype(np.uint8))
        im.save("weights.jpg")

    elif args.classify_data:
        input_image = ImageSource(args.classify_data)
        som = KohonenSom(
            columns=width,
            rows=height,
            data_source=input_image,
            weights=args.load_weights,
        )
        print("Classifyin!")

        h = input_image.shape[0]
        w = input_image.shape[1]

        output_image = np.zeros((h, w, 3), dtype=np.uint8)
        orig_image = ImageSource(args.classify_data)

        for y in trange(h):
            values = range(0, w, 1) if y & 1 else range(w - 1, -1, -1)
            direction = 1 if y & 1 else -1

            for x in values:
                new_pixel = som.best_matching_value(input_image, y * w + x)
                output_image[y, x] = (new_pixel * 255.0).astype(np.uint8)

                if not args.dithering:
                    continue

                # If dithering, Floyd-Steinberg:
                error = (orig_image[y, x] - new_pixel) / 16
                if 0 <= x + direction <= w - 1:
                    pix = input_image[y, x + direction]
                    pix += error * 7

                if y + 1 < h:
                    pix = input_image[y + 1, x]
                    pix += error * 5

                    if 0 <= x - direction <= w - 1:
                        pix = input_image[y + 1, x - direction]
                        pix += error * 3

                    if 0 <= x + direction <= w - 1:
                        pix = input_image[y + 1, x + direction]
                        pix += error

        im = Image.fromarray(output_image)
        im.save(args.output)
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
