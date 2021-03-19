import argparse
import json

from rest_api import RestApi


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--config",
        help="Config file for render farm",
        default="config.json",
        type=argparse.FileType("r"),
    )
    args = parser.parse_args()

    config = json.load(args.config)

    web = RestApi(config)
    web.run()


if __name__ == "__main__":
    main()
