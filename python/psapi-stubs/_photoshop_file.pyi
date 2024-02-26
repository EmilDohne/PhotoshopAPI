import os

import psapi.util
import psapi.enum

class PhotoshopFile:

    def __init__(self: PhotoshopFile) -> None:
        ...

    def read(self: PhotoshopFile, document: psapi.util.File) -> None:
        ...

    def write(self: PhotoshopFile, document: psapi.util.File) -> None:
        ...
    
    @staticmethod
    def find_bitdepth(path: os.PathLike) -> psapi.enum.BitDepth:
        ...
