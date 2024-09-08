"""
hexdump.py

This script extracts a specified section of a binary file and saves it to a new file
with the same base name but a .bin extension.

Usage:
    python hexdump.py <path> <offset> <size>

Arguments:
    <path>    : The path to the input file (e.g., smartobject.psd).
    <offset>  : The offset (in bytes) from where to start reading the file.
    <size>    : The number of bytes to read from the offset.

Example:
    python hexdump.py smartobject.psd 21966 1652

This will read 1652 bytes from the file `smartobject.psd` starting at byte offset 21966
and will write the output to `smartobject.bin` in the same directory.
"""
import os
import argparse


def hexdump_section(path: str, offset: int, size: int):
    '''
    Hexdump a file section to an equally named file with the extension .bin
    
    :param path: The path to the input file.
    :param offset: The offset (in bytes) from where to start the dump.
    :param size: The number of bytes to read from the offset.
    """
    '''
    with open(path, mode='rb') as binary_file:
        binary_file.seek(offset)
        buff = binary_file.read(size)
        out_path = os.path.join(os.path.dirname(path), os.path.splitext(os.path.basename(path))[0] + ".bin")
        with open(out_path, "wb") as out_file:
            print (f"Wrote to file: {out_path}")
            out_file.write(buff)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Hexdump a section of a file to a new file with a .bin extension.")
    parser.add_argument('path', type=str, help="The path to the input file.")
    parser.add_argument('offset', type=int, help="The offset (in bytes) from where to start the dump.")
    parser.add_argument('size', type=int, help="The number of bytes to read from the offset.")
    
    args = parser.parse_args()
    hexdump_section(args.path, args.offset, args.size)