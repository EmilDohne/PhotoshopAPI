from typing import overload, Optional
import numpy
import os

import psapi.enum
import psapi.util
from ._layer import Layer_8bit, Layer_16bit, Layer_32bit


class LayeredFile:
			
    @staticmethod
    def read(path: os.PathLike) -> LayeredFile_8bit | LayeredFile_16bit | LayeredFile_32bit:
        ...


class LayeredFile_8bit:

    @property
    def icc(self: LayeredFile_8bit) -> numpy.ndarray:
        ...

    @icc.setter
    def icc(self: LayeredFile_8bit, value: numpy.ndarray) -> None:
        ...

    @overload
    @icc.setter
    def icc(self: LayeredFile_8bit, path: os.PathLike) -> None:
        ...

    # We put it here as its property-like
    def compression(self: LayeredFile_8bit, compression: psapi.enum.Compression) -> None:
        ...

    @property
    def num_channels(self: LayeredFile_8bit) -> int:
        ...

    @property
    def layers(self: LayeredFile_8bit) -> list[Layer_8bit]:
        ...

    @property
    def layers_flat(self: LayeredFile_8bit) -> list[Layer_8bit]:
        ...

    @property
    def bit_depth(self: LayeredFile_8bit) -> psapi.enum.BitDepth:
        ...

    @property
    def dpi(self: LayeredFile_8bit) -> float:
        ...

    @dpi.setter
    def dpi(self: LayeredFile_8bit, value: float) -> None:
        ...

    @property
    def width(self: LayeredFile_8bit) -> int:
        ...

    @width.setter
    def width(self: LayeredFile_8bit, value: int) -> None:
        ...
    
    @property
    def height(self: LayeredFile_8bit) -> int:
        ...

    @height.setter
    def height(self: LayeredFile_8bit, value: int) -> None:
        ...
    
    @overload
    def __init__(self: LayeredFile_8bit) -> None:
        ...

    @overload
    def __init__(self: LayeredFile_8bit, color_mode: psapi.enum.ColorMode, width: int, height: int) -> None:
        ...

    def find_layer(self: LayeredFile_8bit, path: str) -> Layer_8bit:
        ...

    def add_layer(self: LayeredFile_8bit, layer: Layer_8bit) -> None:
        ...

    @overload
    def move_layer(self: LayeredFile_8bit, child: Layer_8bit, parent: Optional[Layer_8bit]) -> None:
        ...

    @overload
    def move_layer(self: LayeredFile_8bit, child: str, parent: Optional[str]) -> None:
        ...

    @overload
    def remove_layer(self: LayeredFile_8bit, layer: Layer_8bit) -> None:
        ...

    @overload
    def remove_layer(self: LayeredFile_8bit, layer: str) -> None:
        ...

    def is_layer_in_document(self: LayeredFile_8bit, layer: Layer_8bit) -> bool:
        ...

    @staticmethod
    def read(path: os.PathLike) -> LayeredFile_8bit:
        ...

    def write(self: LayeredFile_8bit, path: os.PathLike, force_overwrite: bool = ...) -> None:
        ...

    def __getitem__(self: LayeredFile_8bit, name: str) -> Layer_8bit:
        ...


class LayeredFile_16bit:

    @property
    def icc(self: LayeredFile_16bit) -> numpy.ndarray:
        ...

    @icc.setter
    def icc(self: LayeredFile_16bit, value: numpy.ndarray) -> None:
        ...

    @overload
    @icc.setter
    def icc(self: LayeredFile_16bit, path: os.PathLike) -> None:
        ...

    # We put it here as its property-like
    def compression(self: LayeredFile_16bit, compression: psapi.enum.Compression) -> None:
        ...

    @property
    def num_channels(self: LayeredFile_16bit) -> int:
        ...

    @property
    def layers(self: LayeredFile_16bit) -> list[Layer_16bit]:
        ...

    @property
    def layers_flat(self: LayeredFile_16bit) -> list[Layer_16bit]:
        ...

    @property
    def bit_depth(self: LayeredFile_16bit) -> psapi.enum.BitDepth:
        ...

    @property
    def dpi(self: LayeredFile_16bit) -> float:
        ...

    @dpi.setter
    def dpi(self: LayeredFile_16bit, value: float) -> None:
        ...

    @property
    def width(self: LayeredFile_16bit) -> int:
        ...

    @width.setter
    def width(self: LayeredFile_16bit, value: int) -> None:
        ...
    
    @property
    def height(self: LayeredFile_16bit) -> int:
        ...

    @height.setter
    def height(self: LayeredFile_16bit, value: int) -> None:
        ...
    
    @overload
    def __init__(self: LayeredFile_16bit) -> None:
        ...

    @overload
    def __init__(self: LayeredFile_16bit, color_mode: psapi.enum.ColorMode, width: int, height: int) -> None:
        ...

    def find_layer(self: LayeredFile_16bit, path: str) -> Layer_16bit:
        ...

    def add_layer(self: LayeredFile_16bit, layer: Layer_16bit) -> None:
        ...

    @overload
    def move_layer(self: LayeredFile_16bit, child: Layer_16bit, parent: Optional[Layer_16bit]) -> None:
        ...

    @overload
    def move_layer(self: LayeredFile_16bit, child: str, parent: Optional[str]) -> None:
        ...

    @overload
    def remove_layer(self: LayeredFile_16bit, layer: Layer_16bit) -> None:
        ...

    @overload
    def remove_layer(self: LayeredFile_16bit, layer: str) -> None:
        ...

    def is_layer_in_document(self: LayeredFile_16bit, layer: Layer_16bit) -> bool:
        ...

    @staticmethod
    def read(path: os.PathLike) -> LayeredFile_16bit:
        ...

    def write(self: LayeredFile_16bit, path: os.PathLike, force_overwrite: bool = ...) -> None:
        ...

    def __getitem__(self: LayeredFile_16bit, name: str) -> Layer_16bit:
        ...


class LayeredFile_32bit:

    @property
    def icc(self: LayeredFile_32bit) -> numpy.ndarray:
        ...

    @icc.setter
    def icc(self: LayeredFile_32bit, value: numpy.ndarray) -> None:
        ...

    @overload
    @icc.setter
    def icc(self: LayeredFile_32bit, path: os.PathLike) -> None:
        ...

    # We put it here as its property-like
    def compression(self: LayeredFile_32bit, compression: psapi.enum.Compression) -> None:
        ...

    @property
    def num_channels(self: LayeredFile_32bit) -> int:
        ...

    @property
    def layers(self: LayeredFile_32bit) -> list[Layer_32bit]:
        ...

    @property
    def layers_flat(self: LayeredFile_32bit) -> list[Layer_32bit]:
        ...

    @property
    def bit_depth(self: LayeredFile_32bit) -> psapi.enum.BitDepth:
        ...

    @property
    def dpi(self: LayeredFile_32bit) -> float:
        ...

    @dpi.setter
    def dpi(self: LayeredFile_32bit, value: float) -> None:
        ...

    @property
    def width(self: LayeredFile_32bit) -> int:
        ...

    @width.setter
    def width(self: LayeredFile_32bit, value: int) -> None:
        ...
    
    @property
    def height(self: LayeredFile_32bit) -> int:
        ...

    @height.setter
    def height(self: LayeredFile_32bit, value: int) -> None:
        ...
    
    @overload
    def __init__(self: LayeredFile_32bit) -> None:
        ...

    @overload
    def __init__(self: LayeredFile_32bit, color_mode: psapi.enum.ColorMode, width: int, height: int) -> None:
        ...

    def find_layer(self: LayeredFile_32bit, path: str) -> Layer_32bit:
        ...

    def add_layer(self: LayeredFile_32bit, layer: Layer_32bit) -> None:
        ...

    @overload
    def move_layer(self: LayeredFile_32bit, child: Layer_32bit, parent: Optional[Layer_32bit]) -> None:
        ...

    @overload
    def move_layer(self: LayeredFile_32bit, child: str, parent: Optional[str]) -> None:
        ...

    @overload
    def remove_layer(self: LayeredFile_32bit, layer: Layer_32bit) -> None:
        ...

    @overload
    def remove_layer(self: LayeredFile_32bit, layer: str) -> None:
        ...

    def is_layer_in_document(self: LayeredFile_32bit, layer: Layer_32bit) -> bool:
        ...

    @staticmethod
    def read(path: os.PathLike) -> LayeredFile_32bit:
        ...

    def write(self: LayeredFile_32bit, path: os.PathLike, force_overwrite: bool = ...) -> None:
        ...

    def __getitem__(self: LayeredFile_32bit, name: str) -> Layer_32bit:
        ...
