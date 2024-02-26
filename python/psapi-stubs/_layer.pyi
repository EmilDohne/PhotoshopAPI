import numpy

import psapi.enum
import psapi.util


class Layer_8bit:

    @property
    def name(self: Layer_8bit) -> str:
        ...

    @name.setter
    def name(self: Layer_8bit, name: str) -> None:
        ...

    @property
    def layer_mask(self: Layer_8bit) -> numpy.ndarray:
        ...

    @property
    def blend_mode(self: Layer_8bit) -> psapi.enum.BlendMode:
        ...

    @blend_mode.setter
    def blend_mode(self: Layer_8bit, value: psapi.enum.BlendMode) -> None:
        ...

    @property
    def opacity(self: Layer_8bit) -> int:
        ...

    @opacity.setter
    def opacity(self: Layer_8bit, value: int) -> None:
        ...

    @property
    def width(self: Layer_8bit) -> int:
        ...

    @width.setter
    def width(self: Layer_8bit, value: int) -> None:
        ...

    @property
    def height(self: Layer_8bit) -> int:
        ...

    @height.setter
    def height(self: Layer_8bit, value: int) -> None:
        ...

    @property
    def center_x(self: Layer_8bit) -> float:
        ...

    @center_x.setter
    def center_x(self: Layer_8bit, value: float) -> None:
        ...

    @property
    def center_y(self: Layer_8bit) -> float:
        ...

    @center_y.setter
    def center_y(self: Layer_8bit, value: float) -> None:
        ...

    def get_mask_data(self: Layer_8bit, do_copy: bool = ...) -> numpy.ndarray:
        ...


class Layer_16bit:

    @property
    def name(self: Layer_16bit) -> str:
        ...

    @name.setter
    def name(self: Layer_16bit, name: str) -> None:
        ...

    @property
    def layer_mask(self: Layer_16bit) -> numpy.ndarray:
        ...

    @property
    def blend_mode(self: Layer_16bit) -> psapi.enum.BlendMode:
        ...

    @blend_mode.setter
    def blend_mode(self: Layer_16bit, value: psapi.enum.BlendMode) -> None:
        ...

    @property
    def opacity(self: Layer_16bit) -> int:
        ...

    @opacity.setter
    def opacity(self: Layer_16bit, value: int) -> None:
        ...

    @property
    def width(self: Layer_16bit) -> int:
        ...

    @width.setter
    def width(self: Layer_16bit, value: int) -> None:
        ...

    @property
    def height(self: Layer_16bit) -> int:
        ...

    @height.setter
    def height(self: Layer_16bit, value: int) -> None:
        ...

    @property
    def center_x(self: Layer_16bit) -> float:
        ...

    @center_x.setter
    def center_x(self: Layer_16bit, value: float) -> None:
        ...

    @property
    def center_y(self: Layer_16bit) -> float:
        ...

    @center_y.setter
    def center_y(self: Layer_16bit, value: float) -> None:
        ...

    def get_mask_data(self: Layer_16bit, do_copy: bool = ...) -> numpy.ndarray:
        ...


class Layer_32bit:

    @property
    def name(self: Layer_32bit) -> str:
        ...

    @name.setter
    def name(self: Layer_32bit, name: str) -> None:
        ...

    @property
    def layer_mask(self: Layer_32bit) -> numpy.ndarray:
        ...

    @property
    def blend_mode(self: Layer_32bit) -> psapi.enum.BlendMode:
        ...

    @blend_mode.setter
    def blend_mode(self: Layer_32bit, value: psapi.enum.BlendMode) -> None:
        ...

    @property
    def opacity(self: Layer_32bit) -> int:
        ...

    @opacity.setter
    def opacity(self: Layer_32bit, value: int) -> None:
        ...

    @property
    def width(self: Layer_32bit) -> int:
        ...

    @width.setter
    def width(self: Layer_32bit, value: int) -> None:
        ...

    @property
    def height(self: Layer_32bit) -> int:
        ...

    @height.setter
    def height(self: Layer_32bit, value: int) -> None:
        ...

    @property
    def center_x(self: Layer_32bit) -> float:
        ...

    @center_x.setter
    def center_x(self: Layer_32bit, value: float) -> None:
        ...

    @property
    def center_y(self: Layer_32bit) -> float:
        ...

    @center_y.setter
    def center_y(self: Layer_32bit, value: float) -> None:
        ...

    def get_mask_data(self: Layer_32bit, do_copy: bool = ...) -> numpy.ndarray:
        ...
        