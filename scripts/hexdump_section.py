import os


def hexdump_section(path: str, offset: int, size: int):
    '''
    Hexdump a file section to an equally named file with the extension .bin
    '''
    with open(path, mode='rb') as binary_file:
        binary_file.seek(offset)
        buff = binary_file.read(size)
        out_path = os.path.join(os.path.dirname(path), os.path.splitext(os.path.basename(path))[0] + ".bin")
        with open(out_path, "wb") as out_file:
            print (f"Wrote to file: {out_path}")
            out_file.write(buff)

if __name__ == "__main__":
    hexdump_section("C:/Users/emild/Desktop/smartobject.psd", 21966, 1652)