import numpy

import psapi.enum
import psapi.util
import psapi.geometry


class Layer_8bit:

    @property
    def name(self: Layer_8bit) -> str:
        ...

    @name.setter
    def name(self: Layer_8bit, name: str) -> None:
        ...

    def has_mask(self: Layer_8bit, name: str) -> bool:
        ...

    @property
    def mask(self: Layer_8bit) -> numpy.ndarray:
        ...

    @mask.setter
    def mask(self: Layer_8bit, value: numpy.ndarray) -> None:
        ...

    @property
    def blend_mode(self: Layer_8bit) -> psapi.enum.BlendMode:
        ...

    @blend_mode.setter
    def blend_mode(self: Layer_8bit, value: psapi.enum.BlendMode) -> None:
        ...

    @property
    def opacity(self: Layer_8bit) -> float:
        ...

    @opacity.setter
    def opacity(self: Layer_8bit, value: float) -> None:
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

    @property
    def is_visible(self: Layer_8bit) -> bool:
        ...

    @is_visible.setter
    def is_visible(self: Layer_8bit, value: bool) -> None:
        ...

    @property
    def is_locked(self: Layer_8bit) -> bool:
        ...

    @is_locked.setter
    def is_locked(self: Layer_8bit, value: bool) -> None:
        ...

    @property
    def mask_disabled(self: Layer_8bit) -> bool:
        ...

    @mask_disabled.setter
    def mask_disabled(self: Layer_8bit, value: bool) -> None:
        ...

    @property
    def mask_relative_to_layer(self: Layer_8bit) -> bool:
        ...

    @mask_relative_to_layer.setter
    def mask_relative_to_layer(self: Layer_8bit, value: bool) -> None:
        ...

    @property
    def mask_default_color(self: Layer_8bit) -> int:
        ...

    @mask_relative_to_layer.setter
    def mask_default_color(self: Layer_8bit, value: int) -> None:
        ...

    @property
    def mask_density(self: Layer_8bit) -> int:
        ...

    @mask_density.setter
    def mask_density(self: Layer_8bit, value: int) -> None:
        ...

    @property
    def mask_feather(self: Layer_8bit) -> float:
        ...

    @mask_feather.setter
    def mask_feather(self: Layer_8bit, value: float) -> None:
        ...

    @property
    def mask_position(self: Layer_8bit) -> psapi.geometry.Point2D:
        ...

    @mask_position.setter
    def mask_position(self: Layer_8bit, value: psapi.geometry.Point2D) -> None:
        ...

    def mask_width(self: Layer_8bit) -> int:
        ...

    def mask_height(self: Layer_8bit) -> int:
        ...

    def set_mask_compression(self: Layer_8bit, compcode: psapi.enum.Compression) -> None:
        ...


class Layer_16bit:

    @property
    def name(self: Layer_16bit) -> str:
        ...

    @name.setter
    def name(self: Layer_16bit, name: str) -> None:
        ...

    def has_mask(self: Layer_16bit, name: str) -> bool:
        ...

    @property
    def mask(self: Layer_16bit) -> numpy.ndarray:
        ...

    @mask.setter
    def mask(self: Layer_16bit, value: numpy.ndarray) -> None:
        ...

    @property
    def blend_mode(self: Layer_16bit) -> psapi.enum.BlendMode:
        ...

    @blend_mode.setter
    def blend_mode(self: Layer_16bit, value: psapi.enum.BlendMode) -> None:
        ...

    @property
    def opacity(self: Layer_16bit) -> float:
        ...

    @opacity.setter
    def opacity(self: Layer_16bit, value: float) -> None:
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

    @property
    def is_visible(self: Layer_16bit) -> bool:
        ...

    @is_visible.setter
    def is_visible(self: Layer_16bit, value: bool) -> None:
        ...

    @property
    def is_locked(self: Layer_16bit) -> bool:
        ...

    @is_locked.setter
    def is_locked(self: Layer_16bit, value: bool) -> None:
        ...

    @property
    def mask_disabled(self: Layer_16bit) -> bool:
        ...

    @mask_disabled.setter
    def mask_disabled(self: Layer_16bit, value: bool) -> None:
        ...

    @property
    def mask_relative_to_layer(self: Layer_16bit) -> bool:
        ...

    @mask_relative_to_layer.setter
    def mask_relative_to_layer(self: Layer_16bit, value: bool) -> None:
        ...

    @property
    def mask_default_color(self: Layer_16bit) -> int:
        ...

    @mask_relative_to_layer.setter
    def mask_default_color(self: Layer_16bit, value: int) -> None:
        ...

    @property
    def mask_density(self: Layer_16bit) -> int:
        ...

    @mask_density.setter
    def mask_density(self: Layer_16bit, value: int) -> None:
        ...

    @property
    def mask_feather(self: Layer_16bit) -> float:
        ...

    @mask_feather.setter
    def mask_feather(self: Layer_16bit, value: float) -> None:
        ...

    @property
    def mask_position(self: Layer_16bit) -> psapi.geometry.Point2D:
        ...

    @mask_position.setter
    def mask_position(self: Layer_16bit, value: psapi.geometry.Point2D) -> None:
        ...

    def mask_width(self: Layer_16bit) -> int:
        ...

    def mask_height(self: Layer_16bit) -> int:
        ...

    def set_mask_compression(self: Layer_16bit, compcode: psapi.enum.Compression) -> None:
        ...



class Layer_32bit:

    @property
    def name(self: Layer_32bit) -> str:
        ...

    @name.setter
    def name(self: Layer_32bit, name: str) -> None:
        ...

    def has_mask(self: Layer_32bit, name: str) -> bool:
        ...

    @property
    def mask(self: Layer_32bit) -> numpy.ndarray:
        ...

    @mask.setter
    def mask(self: Layer_32bit, value: numpy.ndarray) -> None:
        ...

    @property
    def blend_mode(self: Layer_32bit) -> psapi.enum.BlendMode:
        ...

    @blend_mode.setter
    def blend_mode(self: Layer_32bit, value: psapi.enum.BlendMode) -> None:
        ...

    @property
    def opacity(self: Layer_32bit) -> float:
        ...

    @opacity.setter
    def opacity(self: Layer_32bit, value: float) -> None:
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

    @property
    def is_visible(self: Layer_32bit) -> bool:
        ...

    @is_visible.setter
    def is_visible(self: Layer_32bit, value: bool) -> None:
        ...

    @property
    def is_locked(self: Layer_32bit) -> bool:
        ...

    @is_locked.setter
    def is_locked(self: Layer_32bit, value: bool) -> None:
        ...

    @property
    def mask_disabled(self: Layer_32bit) -> bool:
        ...

    @mask_disabled.setter
    def mask_disabled(self: Layer_32bit, value: bool) -> None:
        ...

    @property
    def mask_relative_to_layer(self: Layer_32bit) -> bool:
        ...

    @mask_relative_to_layer.setter
    def mask_relative_to_layer(self: Layer_32bit, value: bool) -> None:
        ...

    @property
    def mask_default_color(self: Layer_32bit) -> int:
        ...

    @mask_relative_to_layer.setter
    def mask_default_color(self: Layer_32bit, value: int) -> None:
        ...

    @property
    def mask_density(self: Layer_32bit) -> int:
        ...

    @mask_density.setter
    def mask_density(self: Layer_32bit, value: int) -> None:
        ...

    @property
    def mask_feather(self: Layer_32bit) -> float:
        ...

    @mask_feather.setter
    def mask_feather(self: Layer_32bit, value: float) -> None:
        ...

    @property
    def mask_position(self: Layer_32bit) -> psapi.geometry.Point2D:
        ...

    @mask_position.setter
    def mask_position(self: Layer_32bit, value: psapi.geometry.Point2D) -> None:
        ...

    def mask_width(self: Layer_32bit) -> int:
        ...

    def mask_height(self: Layer_32bit) -> int:
        ...

    def set_mask_compression(self: Layer_32bit, compcode: psapi.enum.Compression) -> None:
        ...
