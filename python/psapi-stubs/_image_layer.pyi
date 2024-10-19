from typing import overload, Optional
import numpy

import psapi.enum
import psapi.util
from ._layer import Layer_8bit, Layer_16bit, Layer_32bit


class ImageLayer_8bit(Layer_8bit):

    @property
    def image_data(self: ImageLayer_8bit) -> dict[psapi.util.ChannelIDInfo, numpy.ndarray]:
        ...

    @property
    def num_channels(self: ImageLayer_8bit) -> int:
        ...

    @property
    def channels(self: ImageLayer_8bit) -> list[int]:
        ...
    
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
        color_mode: psapi.enum.ColorMode = ...
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
        color_mode: psapi.enum.ColorMode = ...
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

    def get_channel_by_id(self: ImageLayer_8bit, id: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        ...

    def get_channel_by_index(self: ImageLayer_8bit, index: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        ...

    def set_channel_by_id(self: ImageLayer_8bit, key: psapi.enum.ChannelID, value: numpy.ndarray) -> None:
        ...

    def set_channel_by_index(self: ImageLayer_8bit, key: int, value: numpy.ndarray) -> None:
        ...

    def get_image_data(self: ImageLayer_8bit, do_copy: bool = ...) -> dict[psapi.util.ChannelIDInfo, numpy.ndarray]:
        ...
        
    @overload
    def set_image_data(
        self: ImageLayer_8bit,
        image_data: numpy.ndarray,
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    @overload
    def set_image_data(
        self: ImageLayer_8bit,
        image_data: dict[int, numpy.ndarray],
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    @overload
    def set_image_data(
        self: ImageLayer_8bit,
        image_data: dict[psapi.enum.ChannelID, numpy.ndarray],
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    def set_compression(self: ImageLayer_8bit, compression: psapi.enum.Compression) -> None:
        ...

    def __getitem__(self: ImageLayer_8bit, key: psapi.enum.ChannelID | int) -> numpy.ndarray:
        ...
        
    def __setitem__(self: ImageLayer_8bit, key: psapi.enum.ChannelID | int, value: numpy.ndarray) -> None:
        ...



class ImageLayer_16bit(Layer_16bit):
    
    @property
    def image_data(self: ImageLayer_16bit) -> dict[psapi.util.ChannelIDInfo, numpy.ndarray]:
        ...

    @property
    def num_channels(self: ImageLayer_16bit) -> int:
        ...

    @property
    def channels(self: ImageLayer_16bit) -> list[int]:
        ...

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
        color_mode: psapi.enum.ColorMode = ...
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
        color_mode: psapi.enum.ColorMode = ...
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
        color_mode: psapi.enum.ColorMode = ...
    ) -> None:
        ...

    def get_channel_by_id(self: ImageLayer_16bit, id: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        ...

    def get_channel_by_index(self: ImageLayer_16bit, index: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        ...

    def set_channel_by_id(self: ImageLayer_16bit, key: psapi.enum.ChannelID, value: numpy.ndarray) -> None:
        ...

    def set_channel_by_index(self: ImageLayer_16bit, key: int, value: numpy.ndarray) -> None:
        ...

    def get_image_data(self: ImageLayer_16bit, do_copy: bool = ...) -> dict[psapi.util.ChannelIDInfo, numpy.ndarray]:
        ...
        
    @overload
    def set_image_data(
        self: ImageLayer_16bit,
        image_data: numpy.ndarray,
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    @overload
    def set_image_data(
        self: ImageLayer_16bit,
        image_data: dict[int, numpy.ndarray],
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    @overload
    def set_image_data(
        self: ImageLayer_16bit,
        image_data: dict[psapi.enum.ChannelID, numpy.ndarray],
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    def set_compression(self: ImageLayer_16bit, compression: psapi.enum.Compression) -> None:
        ...

    def __getitem__(self: ImageLayer_16bit, key: psapi.enum.ChannelID | int) -> numpy.ndarray:
        ...
        
    def __setitem__(self: ImageLayer_16bit, key: psapi.enum.ChannelID | int, value: numpy.ndarray) -> None:
        ...


class ImageLayer_32bit(Layer_32bit):

    @property
    def image_data(self: ImageLayer_32bit) -> dict[psapi.util.ChannelIDInfo, numpy.ndarray]:
        ...
    
    @property
    def num_channels(self: ImageLayer_32bit) -> int:
        ...

    @property
    def channels(self: ImageLayer_32bit) -> list[int]:
        ...

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
        color_mode: psapi.enum.ColorMode = ...
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
        color_mode: psapi.enum.ColorMode = ...
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
        color_mode: psapi.enum.ColorMode = ...
    ) -> None:
        ...

    def get_channel_by_id(self: ImageLayer_32bit, id: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        ...

    def get_channel_by_index(self: ImageLayer_32bit, index: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        ...
        
    def set_channel_by_id(self: ImageLayer_32bit, key: psapi.enum.ChannelID, value: numpy.ndarray) -> None:
        ...

    def set_channel_by_index(self: ImageLayer_32bit, key: int, value: numpy.ndarray) -> None:
        ...

    def get_image_data(self: ImageLayer_32bit, do_copy: bool = ...) -> dict[psapi.util.ChannelIDInfo, numpy.ndarray]:
        ...
        
    @overload
    def set_image_data(
        self: ImageLayer_32bit,
        image_data: numpy.ndarray,
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    @overload
    def set_image_data(
        self: ImageLayer_32bit,
        image_data: dict[int, numpy.ndarray],
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    @overload
    def set_image_data(
        self: ImageLayer_32bit,
        image_data: dict[psapi.enum.ChannelID, numpy.ndarray],
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    def set_compression(self: ImageLayer_32bit, compression: psapi.enum.Compression) -> None:
        ...

    def __getitem__(self: ImageLayer_32bit, key: psapi.enum.ChannelID | int) -> numpy.ndarray:
        ...

    def __setitem__(self: ImageLayer_32bit, key: psapi.enum.ChannelID | int, value: numpy.ndarray) -> None:
        ...