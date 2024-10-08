from typing import overload, Optional
import numpy

import psapi.enum
import psapi.util
from ._layer import Layer_8bit, Layer_16bit, Layer_32bit
from ._layered_file import LayeredFile_8bit, LayeredFile_16bit, LayeredFile_32bit


class GroupLayer_8bit(Layer_8bit):

    @property
    def layers(self: GroupLayer_8bit) -> list[Layer_8bit]:
        ...

    @layers.setter
    def layers(self: GroupLayer_8bit, value: list[Layer_8bit]) -> None:
        ...

    @property
    def is_collapsed(self: GroupLayer_8bit) -> bool:
        ...

    @is_collapsed.setter
    def is_collapsed(self: GroupLayer_8bit, value: bool) -> None:
        ...

    def __init__(
        self: GroupLayer_8bit,    
        layer_name: str,
        layer_mask: Optional[numpy.ndarray] = ...,
        width: int = ...,
        height: int = ...,
        blend_mode: psapi.enum.BlendMode = ...,
        pos_x: int = ...,
        pos_y: int = ...,
        opacity: int = ...,
        compression: psapi.enum.Compression = ...,
        color_mode: psapi.enum.ColorMode = ...,
        is_collapsed: bool = ...) -> None:
        ...

    def add_layer(self: GroupLayer_8bit, layered_file: LayeredFile_8bit, layer: Layer_8bit) -> None:
        ...

    @overload
    def remove_layer(self: GroupLayer_8bit, index: int) -> None:
        ...

    @overload
    def remove_layer(self: GroupLayer_8bit, layer: Layer_8bit) -> None:
        ...
    
    @overload
    def remove_layer(self: GroupLayer_8bit, layer_name: str) -> None:
        ...

    def __getitem__(self: GroupLayer_8bit, value: str) -> Layer_8bit:
        ...


class GroupLayer_16bit(Layer_16bit):

    @property
    def layers(self: GroupLayer_16bit) -> list[Layer_16bit]:
        ...

    @layers.setter
    def layers(self: GroupLayer_16bit, value: list[Layer_16bit]) -> None:
        ...

    @property
    def is_collapsed(self: GroupLayer_16bit) -> bool:
        ...

    @is_collapsed.setter
    def is_collapsed(self: GroupLayer_16bit, value: bool) -> None:
        ...

    def __init__(
        self: GroupLayer_16bit,    
        layer_name: str,
        layer_mask: Optional[numpy.ndarray] = ...,
        width: int = ...,
        height: int = ...,
        blend_mode: psapi.enum.BlendMode = ...,
        pos_x: int = ...,
        pos_y: int = ...,
        opacity: int = ...,
        compression: psapi.enum.Compression = ...,
        color_mode: psapi.enum.ColorMode = ...,
        is_collapsed: bool = ...) -> None: 
        ...

    def add_layer(self: GroupLayer_16bit, layered_file: LayeredFile_16bit, layer: Layer_16bit) -> None:
        ...

    @overload
    def remove_layer(self: GroupLayer_16bit, index: int) -> None:
        ...

    @overload
    def remove_layer(self: GroupLayer_16bit, layer: Layer_16bit) -> None:
        ...
    
    @overload
    def remove_layer(self: GroupLayer_16bit, layer_name: str) -> None:
        ...

    def __getitem__(self: GroupLayer_16bit, value: str) -> Layer_16bit:
        ...


class GroupLayer_32bit(Layer_32bit):

    @property
    def layers(self: GroupLayer_32bit) -> list[Layer_32bit]:
        ...

    @layers.setter
    def layers(self: GroupLayer_32bit, value: list[Layer_32bit]) -> None:
        ...

    @property
    def is_collapsed(self: GroupLayer_32bit) -> bool:
        ...

    @is_collapsed.setter
    def is_collapsed(self: GroupLayer_32bit, value: bool) -> None:
        ...

    def __init__(
        self: GroupLayer_32bit,    
        layer_name: str,
        layer_mask: Optional[numpy.ndarray] = ...,
        width: int = ...,
        height: int = ...,
        blend_mode: psapi.enum.BlendMode = ...,
        pos_x: int = ...,
        pos_y: int = ...,
        opacity: int = ...,
        compression: psapi.enum.Compression = ...,
        color_mode: psapi.enum.ColorMode = ...,
        is_collapsed: bool = ...) -> None: 
        ...

    def add_layer(self: GroupLayer_32bit, layered_file: LayeredFile_32bit, layer: Layer_32bit) -> None:
        ...

    @overload
    def remove_layer(self: GroupLayer_32bit, index: int) -> None:
        ...

    @overload
    def remove_layer(self: GroupLayer_32bit, layer: Layer_32bit) -> None:
        ...
    
    @overload
    def remove_layer(self: GroupLayer_32bit, layer_name: str) -> None:
        ...

    def __getitem__(self: GroupLayer_32bit, value: str) -> Layer_32bit:
        ...