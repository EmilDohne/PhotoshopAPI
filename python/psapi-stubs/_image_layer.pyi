from typing import overload, Optional, List, Dict, Union
import numpy

import psapi.enum
import psapi.util
from ._layer import Layer_8bit, Layer_16bit, Layer_32bit


class ImageLayer_8bit(Layer_8bit):
    
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
        image_data: Dict[int, numpy.ndarray],
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
        image_data: Dict[psapi.enum.ChannelID, numpy.ndarray],
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

    def channel_indices(self: ImageLayer_8bit) -> List[int]:
        ...

    def num_channels(self: ImageLayer_8bit) -> int:
        ...

    def get_image_data(self: ImageLayer_8bit) -> Dict[int, numpy.ndarray]:
        ...

    def set_image_data(
        self: ImageLayer_8bit, 
        data: Union[numpy.ndarray, Dict[int, numpy.ndarray]],
        width: Optional[int] = ...,
        height: Optional[int] = ...) -> None:
        ...

    def __getitem__(self: ImageLayer_8bit, key: int) -> numpy.ndarray:
        ...

    def __setitem__(self: ImageLayer_8bit, key: int, data: numpy.ndarray) -> None:
        ...

    def get_channel_by_index(self: ImageLayer_8bit, key: int) -> numpy.ndarray:
        ...

    def set_channel_by_index(self: ImageLayer_8bit, key: int, data: numpy.ndarray) -> None:
        ...

    def get_channel_by_id(self: ImageLayer_8bit, key: psapi.enum.ChannelID) -> numpy.ndarray:
        ...

    def set_channel_by_index(self: ImageLayer_8bit, key: psapi.enum.ChannelID, data: numpy.ndarray) -> None:
        ...


class ImageLayer_16bit(Layer_16bit):
    
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
        image_data: Dict[int, numpy.ndarray],
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
        image_data: Dict[psapi.enum.ChannelID, numpy.ndarray],
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

    def channel_indices(self: ImageLayer_16bit) -> List[int]:
        ...

    def num_channels(self: ImageLayer_16bit) -> int:
        ...

    def get_image_data(self: ImageLayer_16bit) -> Dict[int, numpy.ndarray]:
        ...

    def set_image_data(
        self: ImageLayer_16bit, 
        data: Union[numpy.ndarray, Dict[int, numpy.ndarray]],
        width: Optional[int] = ...,
        height: Optional[int] = ...) -> None:
        ...

    def __getitem__(self: ImageLayer_16bit, key: int) -> numpy.ndarray:
        ...

    def __setitem__(self: ImageLayer_16bit, key: int, data: numpy.ndarray) -> None:
        ...

    def get_channel_by_index(self: ImageLayer_16bit, key: int) -> numpy.ndarray:
        ...

    def set_channel_by_index(self: ImageLayer_16bit, key: int, data: numpy.ndarray) -> None:
        ...

    def get_channel_by_id(self: ImageLayer_16bit, key: psapi.enum.ChannelID) -> numpy.ndarray:
        ...

    def set_channel_by_index(self: ImageLayer_16bit, key: psapi.enum.ChannelID, data: numpy.ndarray) -> None:
        ...    
        

class ImageLayer_32bit(Layer_32bit):
    
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
        image_data: Dict[int, numpy.ndarray],
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
        image_data: Dict[psapi.enum.ChannelID, numpy.ndarray],
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

    def channel_indices(self: ImageLayer_32bit) -> List[int]:
        ...

    def num_channels(self: ImageLayer_32bit) -> int:
        ...

    def get_image_data(self: ImageLayer_32bit) -> Dict[int, numpy.ndarray]:
        ...

    def set_image_data(
        self: ImageLayer_32bit, 
        data: Union[numpy.ndarray, Dict[int, numpy.ndarray]],
        width: Optional[int] = ...,
        height: Optional[int] = ...) -> None:
        ...

    def __getitem__(self: ImageLayer_32bit, key: int) -> numpy.ndarray:
        ...

    def __setitem__(self: ImageLayer_32bit, key: int, data: numpy.ndarray) -> None:
        ...

    def get_channel_by_index(self: ImageLayer_32bit, key: int) -> numpy.ndarray:
        ...

    def set_channel_by_index(self: ImageLayer_32bit, key: int, data: numpy.ndarray) -> None:
        ...

    def get_channel_by_id(self: ImageLayer_32bit, key: psapi.enum.ChannelID) -> numpy.ndarray:
        ...

    def set_channel_by_index(self: ImageLayer_32bit, key: psapi.enum.ChannelID, data: numpy.ndarray) -> None:
        ...