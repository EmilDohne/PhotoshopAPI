from typing import overload, Optional
import numpy

import psapi.enum
import psapi.util
from ._layer import Layer_8bit, Layer_16bit, Layer_32bit


class _ImageDataLayerType_8bit(Layer_8bit):

    @property
    def image_data(self: _ImageDataLayerType_8bit) -> dict[int, numpy.ndarray]:
        ...

    @property
    def num_channels(self: _ImageDataLayerType_8bit) -> int:
        ...

    @property
    def channels(self: _ImageDataLayerType_8bit) -> list[int]:
        ...
    
    def get_channel_by_id(self: _ImageDataLayerType_8bit, id: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        ...

    def get_channel_by_index(self: _ImageDataLayerType_8bit, index: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        ...

    def set_channel_by_id(self: _ImageDataLayerType_8bit, key: psapi.enum.ChannelID, value: numpy.ndarray) -> None:
        ...

    def set_channel_by_index(self: _ImageDataLayerType_8bit, key: int, value: numpy.ndarray) -> None:
        ...

    def get_image_data(self: _ImageDataLayerType_8bit, do_copy: bool = ...) -> dict[int, numpy.ndarray]:
        ...
        
    @overload
    def set_image_data(
        self: _ImageDataLayerType_8bit,
        image_data: numpy.ndarray,
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    @overload
    def set_image_data(
        self: _ImageDataLayerType_8bit,
        image_data: dict[int, numpy.ndarray],
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    @overload
    def set_image_data(
        self: _ImageDataLayerType_8bit,
        image_data: dict[psapi.enum.ChannelID, numpy.ndarray],
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    def set_compression(self: _ImageDataLayerType_8bit, compression: psapi.enum.Compression) -> None:
        ...

    def __getitem__(self: _ImageDataLayerType_8bit, key: psapi.enum.ChannelID | int) -> numpy.ndarray:
        ...
        
    def __setitem__(self: _ImageDataLayerType_8bit, key: psapi.enum.ChannelID | int, value: numpy.ndarray) -> None:
        ...


class _ImageDataLayerType_16bit(Layer_16bit):

    @property
    def image_data(self: _ImageDataLayerType_16bit) -> dict[int, numpy.ndarray]:
        ...

    @property
    def num_channels(self: _ImageDataLayerType_16bit) -> int:
        ...

    @property
    def channels(self: _ImageDataLayerType_16bit) -> list[int]:
        ...
    
    def get_channel_by_id(self: _ImageDataLayerType_16bit, id: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        ...

    def get_channel_by_index(self: _ImageDataLayerType_16bit, index: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        ...

    def set_channel_by_id(self: _ImageDataLayerType_16bit, key: psapi.enum.ChannelID, value: numpy.ndarray) -> None:
        ...

    def set_channel_by_index(self: _ImageDataLayerType_16bit, key: int, value: numpy.ndarray) -> None:
        ...

    def get_image_data(self: _ImageDataLayerType_16bit, do_copy: bool = ...) -> dict[int, numpy.ndarray]:
        ...
        
    @overload
    def set_image_data(
        self: _ImageDataLayerType_16bit,
        image_data: numpy.ndarray,
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    @overload
    def set_image_data(
        self: _ImageDataLayerType_16bit,
        image_data: dict[int, numpy.ndarray],
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    @overload
    def set_image_data(
        self: _ImageDataLayerType_16bit,
        image_data: dict[psapi.enum.ChannelID, numpy.ndarray],
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    def set_compression(self: _ImageDataLayerType_16bit, compression: psapi.enum.Compression) -> None:
        ...

    def __getitem__(self: _ImageDataLayerType_16bit, key: psapi.enum.ChannelID | int) -> numpy.ndarray:
        ...
        
    def __setitem__(self: _ImageDataLayerType_16bit, key: psapi.enum.ChannelID | int, value: numpy.ndarray) -> None:
        ...
        

class _ImageDataLayerType_16bit(Layer_16bit):

    @property
    def image_data(self: _ImageDataLayerType_16bit) -> dict[int, numpy.ndarray]:
        ...

    @property
    def num_channels(self: _ImageDataLayerType_16bit) -> int:
        ...

    @property
    def channels(self: _ImageDataLayerType_16bit) -> list[int]:
        ...
    
    def get_channel_by_id(self: _ImageDataLayerType_16bit, id: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        ...

    def get_channel_by_index(self: _ImageDataLayerType_16bit, index: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        ...

    def set_channel_by_id(self: _ImageDataLayerType_16bit, key: psapi.enum.ChannelID, value: numpy.ndarray) -> None:
        ...

    def set_channel_by_index(self: _ImageDataLayerType_16bit, key: int, value: numpy.ndarray) -> None:
        ...

    def get_image_data(self: _ImageDataLayerType_16bit, do_copy: bool = ...) -> dict[int, numpy.ndarray]:
        ...
        
    @overload
    def set_image_data(
        self: _ImageDataLayerType_16bit,
        image_data: numpy.ndarray,
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    @overload
    def set_image_data(
        self: _ImageDataLayerType_16bit,
        image_data: dict[int, numpy.ndarray],
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    @overload
    def set_image_data(
        self: _ImageDataLayerType_16bit,
        image_data: dict[psapi.enum.ChannelID, numpy.ndarray],
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    def set_compression(self: _ImageDataLayerType_16bit, compression: psapi.enum.Compression) -> None:
        ...

    def __getitem__(self: _ImageDataLayerType_16bit, key: psapi.enum.ChannelID | int) -> numpy.ndarray:
        ...
        
    def __setitem__(self: _ImageDataLayerType_16bit, key: psapi.enum.ChannelID | int, value: numpy.ndarray) -> None:
        ...
        

class _ImageDataLayerType_32bit(Layer_32bit):

    @property
    def image_data(self: _ImageDataLayerType_32bit) -> dict[int, numpy.ndarray]:
        ...

    @property
    def num_channels(self: _ImageDataLayerType_32bit) -> int:
        ...

    @property
    def channels(self: _ImageDataLayerType_32bit) -> list[int]:
        ...
    
    def get_channel_by_id(self: _ImageDataLayerType_32bit, id: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        ...

    def get_channel_by_index(self: _ImageDataLayerType_32bit, index: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        ...

    def set_channel_by_id(self: _ImageDataLayerType_32bit, key: psapi.enum.ChannelID, value: numpy.ndarray) -> None:
        ...

    def set_channel_by_index(self: _ImageDataLayerType_32bit, key: int, value: numpy.ndarray) -> None:
        ...

    def get_image_data(self: _ImageDataLayerType_32bit, do_copy: bool = ...) -> dict[int, numpy.ndarray]:
        ...
        
    @overload
    def set_image_data(
        self: _ImageDataLayerType_32bit,
        image_data: numpy.ndarray,
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    @overload
    def set_image_data(
        self: _ImageDataLayerType_32bit,
        image_data: dict[int, numpy.ndarray],
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    @overload
    def set_image_data(
        self: _ImageDataLayerType_32bit,
        image_data: dict[psapi.enum.ChannelID, numpy.ndarray],
        compression: psapi.enum.Compression = ...
    ) -> None:
        ...

    def set_compression(self: _ImageDataLayerType_32bit, compression: psapi.enum.Compression) -> None:
        ...

    def __getitem__(self: _ImageDataLayerType_32bit, key: psapi.enum.ChannelID | int) -> numpy.ndarray:
        ...
        
    def __setitem__(self: _ImageDataLayerType_32bit, key: psapi.enum.ChannelID | int, value: numpy.ndarray) -> None:
        ...