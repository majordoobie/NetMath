import argparse
import asyncio
from concurrent.futures import ThreadPoolExecutor, as_completed, wait, \
    ALL_COMPLETED
from pathlib import Path


def main():
    args = _args()
    _verify_dirs(args)

    file_names = [file.resolve() for file in args.in_folder.iterdir()
                  if file.suffix == ".equ"]

    # User threadpool to call the client connection function. The threadpool
    # will use CPUnum + 4 for worker count
    with ThreadPoolExecutor() as executor:
        futures = [executor.submit(_client_connection, args, file_name)
                   for file_name in file_names]

        # explicitly wait for all tasks to complete
        wait(futures, return_when=ALL_COMPLETED)


def _client_connection(args: argparse.Namespace, file_name: Path) -> None:
    print(f"f - {file_name}")


def _verify_dirs(args: argparse.Namespace) -> None:
    """
    Verify that the directory paths do exist

    :param args: Namespace of arguments
    """
    if args.in_folder.is_dir() and args.out_folder.is_dir():
        return
    exit(f"Either {args.in_folder} or {args.out_folder} does not exist")


def _args() -> argparse.Namespace:
    """
    Parse the user arguments and return the namespace containing the
    arguments for creating the TCP connection
    :return: argparser namespace
    """
    parser = argparse.ArgumentParser(
        description="Client application used to send equation files to the"
                    "server to be solved"
    )

    parser.add_argument(
        "-s", "--source", dest="ip", default="127.0.0.1", metavar="",
        help="Specify the address of the server. (Default: %(default)s)"
    )
    parser.add_argument(
        "-p", "--port", dest="port", default=31337, metavar="",
        help="Specify the port to connect to. (Default: %(default)s)"
    )
    parser.add_argument(
        "-i", "--in-folder", metavar="<dir>", required=True, dest="in_folder",
        type=Path,
        help="Folder to read EQU files from"
    )
    parser.add_argument(
        "-o", "--out-folder", metavar="<dir>", required=True, dest="out_folder",
        type=Path,
        help="Folder to write EQU files to"
    )
    return parser.parse_args()


if __name__ == "__main__":
    main()