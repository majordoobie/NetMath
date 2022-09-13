import argparse
import asyncio
import struct
from concurrent.futures import ThreadPoolExecutor, as_completed, wait, \
    ALL_COMPLETED
from pathlib import Path
import socket

FILE_NAME_MAX_LENGTH = 24
NET_HEADER_SIZE = 48


def main():
    args = _args()
    _verify_dirs(args)

    file_names = [file.resolve() for file in args.in_folder.iterdir()
                  if file.suffix == ".equ"]

    if not file_names:
        exit("Did not find any .equ files to parse")

    # loop = asyncio.get_event_loop()
    # User threadpool to call the client connection function. The threadpool
    # will use CPUnum + 4 for worker count
    with ThreadPoolExecutor() as executor:
        futures = [executor.submit(_client_connection, args, file_name)
                   for file_name in file_names]

        # explicitly wait for all tasks to complete
        wait(futures, return_when=ALL_COMPLETED)



def _client_connection(args: argparse.Namespace, file_name: Path) -> None:
    # print("got first callback")
    data = None
    with file_name.open("rb") as equ:
        data = equ.read()

    file_name_len = len(file_name.name)
    # Raise error if the file name is too long
    if file_name_len > FILE_NAME_MAX_LENGTH:
        raise ValueError(f"File {file_name.name} is too long")

    stream_size = file_name_len + len(data)

    header = struct.pack(">IIQ32s", NET_HEADER_SIZE, file_name_len,
                         stream_size, file_name.name.encode(encoding="utf-8"))

    if len(header) != NET_HEADER_SIZE:
        raise ValueError(f"Header size is too big for file {file_name.name}")

    print(f"[Client]\nHeader size: {NET_HEADER_SIZE}\nName Len: {file_name_len}\n"
          f"Total Packets: {stream_size}\nFilename: {file_name}")

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as fd:
        fd.connect((args.host, args.port))
        fd.sendall(header + data)


async def second_callback(args):
    print("Got second callbcak")



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
        "-s", "--source", dest="host", default="127.0.0.1", metavar="",
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
    # asyncio.run(main())