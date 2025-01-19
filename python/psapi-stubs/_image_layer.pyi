from typing import overload, Optional
import numpy

import psapi.enum
import psapi.util
from ._imagedata_layertype import _ImageDataLayerType_8bit, _ImageDataLayerType_16bit, _ImageDataLayerType_32bit


class ImageLayer_8bit(_ImageDataLayerType_8bit):
    
    @overload
    def __init__(
        self: ImageLayer_8bit,
        image_data: numpy.ndarray,
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
        is_visible: bool = ...,
        is_locked: bool = ...
    ) -> None:
        ...

    @overload
    def __init__(
        self: ImageLayer_8bit,
        image_data: dict[int, numpy.ndarray],
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
        is_visible: bool = ...,
        is_locked: bool = ...
    ) -> None:
        ...

    @overload
    def __init__(
        self: ImageLayer_8bit,
        image_data: dict[psapi.enum.ChannelID, numpy.ndarray],
        layer_name: str,
        layer_mask: Optional[numpy.ndarray] = ...,
        width: int = ...,
        height: int = ...,
        blend_mode: psapi.enum.BlendMode = ...,
        pos_x: int = ...,
        pos_y: int = ...,
        opacity: int = ...,
        compression: psapi.enum.Compression = ...,
        color_mode: psapi.enum.ColorMode = ...
    ) -> None:
        ...


class ImageLayer_16bit(_ImageDataLayerType_16bit):
    
    @overload
    def __init__(
        self: ImageLayer_16bit,
        image_data: numpy.ndarray,
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
        is_visible: bool = ...,
        is_locked: bool = ...
    ) -> None:
        ...

    @overload
    def __init__(
        self: ImageLayer_16bit,
        image_data: dict[int, numpy.ndarray],
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
        is_visible: bool = ...,
        is_locked: bool = ...
    ) -> None:
        ...

    @overload
    def __init__(
        self: ImageLayer_16bit,
        image_data: dict[psapi.enum.ChannelID, numpy.ndarray],
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
        is_visible: bool = ...,
        is_locked: bool = ...
    ) -> None:
        ...


class ImageLayer_32bit(_ImageDataLayerType_32bit):

    @overload
    def __init__(
        self: ImageLayer_32bit,
        image_data: numpy.ndarray,
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
        is_visible: bool = ...,
        is_locked: bool = ...
    ) -> None:
        ...

    @overload
    def __init__(
        self: ImageLayer_32bit,
        image_data: dict[int, numpy.ndarray],
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
        is_visible: bool = ...,
        is_locked: bool = ...
    ) -> None:
        ...

    @overload
    def __init__(
        self: ImageLayer_32bit,
        image_data: dict[psapi.enum.ChannelID, numpy.ndarray],
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
        is_visible: bool = ...,
        is_locked: bool = ...
    ) -> None:
        ...
