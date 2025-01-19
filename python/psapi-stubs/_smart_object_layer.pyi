from typing import Optional, Dict
import numpy 

import psapi.enum
import psapi.util
from ._imagedata_layertype import _ImageDataLayerType_8bit, _ImageDataLayerType_16bit, _ImageDataLayerType_32bit
from ._smart_object_warp import SmartObjectWarp
from ._layered_file import LayeredFile_8bit, LayeredFile_16bit, LayeredFile_32bit

class SmartObjectLayer_8bit(_ImageDataLayerType_8bit):

    @property
    def warp(self: SmartObjectLayer_8bit) -> SmartObjectWarp:
        ...
        
    @warp.setter
    def warp(self: SmartObjectLayer_8bit, _warp: SmartObjectWarp) -> None:
        ...

    @property
    def linkage(self: SmartObjectLayer_8bit) -> psapi.enum.LinkedLayerType:
        ...
        
    @linkage.setter
    def linkage(self: SmartObjectLayer_8bit, type: psapi.enum.LinkedLayerType) -> None:
        ...
        
    def __init__(
        self: SmartObjectLayer_8bit,
        layered_file: LayeredFile_8bit,
        path: str,
        layer_name: str,
        link_type: psapi.enum.LinkedLayerType = ...,
        warp: SmartObjectWarp = ...,
        layer_mask: Optional[numpy.ndarray] = ...,
        blend_mode: psapi.enum.BlendMode = ...,
        opacity: int = ...,
        compression: psapi.enum.Compression = ...,
        color_mode: psapi.enum.ColorMode = ...,
        is_visible: bool = ...,
        is_locked: bool = ...
    ) -> None:
        ...

    def replace(self: SmartObjectLayer_8bit, path: str, link_externally: bool = ...) -> None:
        ...
        
    def hash(self: SmartObjectLayer_8bit) -> str:
        ...
        
    def filename(self: SmartObjectLayer_8bit) -> str:
        ...
        
    def filepath(self: SmartObjectLayer_8bit) -> str:
        ...
        
    def original_image_data(self: SmartObjectLayer_8bit) -> Dict[int, numpy.ndarray]:
        ...
        
    def original_width(self: SmartObjectLayer_8bit) -> int:
        ...
        
    def original_height(self: SmartObjectLayer_8bit) -> int:
        ...
        
    def move(self: SmartObjectLayer_8bit, x_offset: float, y_offset: float) -> None:
        ...
        
    def rotate(self: SmartObjectLayer_8bit, angle: float, x: float, y: float) -> None:
        ...
        
    def scale(self: SmartObjectLayer_8bit, x_scalar: float, y_scalar: float, x: float, y: float) -> None:
        ...
        
    def transform(self: SmartObjectLayer_8bit, matrix: numpy.ndarray) -> None:
        ...
        
    def reset_warp(self: SmartObjectLayer_8bit) -> None:
        ...
        
    def reset_transform(self: SmartObjectLayer_8bit) -> None:
        ...
        

class SmartObjectLayer_16bit(_ImageDataLayerType_16bit):

    @property
    def warp(self: SmartObjectLayer_16bit) -> SmartObjectWarp:
        ...
        
    @warp.setter
    def warp(self: SmartObjectLayer_16bit, _warp: SmartObjectWarp) -> None:
        ...

    @property
    def linkage(self: SmartObjectLayer_16bit) -> psapi.enum.LinkedLayerType:
        ...
        
    @linkage.setter
    def linkage(self: SmartObjectLayer_16bit, type: psapi.enum.LinkedLayerType) -> None:
        ...
        
    def __init__(
        self: SmartObjectLayer_16bit,
        layered_file: LayeredFile_16bit,
        path: str,
        layer_name: str,
        link_type: psapi.enum.LinkedLayerType = ...,
        warp: SmartObjectWarp = ...,
        layer_mask: Optional[numpy.ndarray] = ...,
        blend_mode: psapi.enum.BlendMode = ...,
        opacity: int = ...,
        compression: psapi.enum.Compression = ...,
        color_mode: psapi.enum.ColorMode = ...,
        is_visible: bool = ...,
        is_locked: bool = ...
    ) -> None:
        ...

    def replace(self: SmartObjectLayer_16bit, path: str, link_externally: bool = ...) -> None:
        ...
        
    def hash(self: SmartObjectLayer_16bit) -> str:
        ...
        
    def filename(self: SmartObjectLayer_16bit) -> str:
        ...
        
    def filepath(self: SmartObjectLayer_16bit) -> str:
        ...
        
    def original_image_data(self: SmartObjectLayer_16bit) -> Dict[int, numpy.ndarray]:
        ...
        
    def original_width(self: SmartObjectLayer_16bit) -> int:
        ...
        
    def original_height(self: SmartObjectLayer_16bit) -> int:
        ...
        
    def move(self: SmartObjectLayer_16bit, x_offset: float, y_offset: float) -> None:
        ...
        
    def rotate(self: SmartObjectLayer_16bit, angle: float, x: float, y: float) -> None:
        ...
        
    def scale(self: SmartObjectLayer_16bit, x_scalar: float, y_scalar: float, x: float, y: float) -> None:
        ...
        
    def transform(self: SmartObjectLayer_16bit, matrix: numpy.ndarray) -> None:
        ...
        
    def reset_warp(self: SmartObjectLayer_16bit) -> None:
        ...
        
    def reset_transform(self: SmartObjectLayer_16bit) -> None:
        ...
        

class SmartObjectLayer_32bit(_ImageDataLayerType_32bit):

    @property
    def warp(self: SmartObjectLayer_32bit) -> SmartObjectWarp:
        ...
        
    @warp.setter
    def warp(self: SmartObjectLayer_32bit, _warp: SmartObjectWarp) -> None:
        ...

    @property
    def linkage(self: SmartObjectLayer_32bit) -> psapi.enum.LinkedLayerType:
        ...
        
    @linkage.setter
    def linkage(self: SmartObjectLayer_32bit, type: psapi.enum.LinkedLayerType) -> None:
        ...
        
    def __init__(
        self: SmartObjectLayer_32bit,
        layered_file: LayeredFile_32bit,
        path: str,
        layer_name: str,
        link_type: psapi.enum.LinkedLayerType = ...,
        warp: SmartObjectWarp = ...,
        layer_mask: Optional[numpy.ndarray] = ...,
        blend_mode: psapi.enum.BlendMode = ...,
        opacity: int = ...,
        compression: psapi.enum.Compression = ...,
        color_mode: psapi.enum.ColorMode = ...,
        is_visible: bool = ...,
        is_locked: bool = ...
    ) -> None:
        ...

    def replace(self: SmartObjectLayer_32bit, path: str, link_externally: bool = ...) -> None:
        ...
        
    def hash(self: SmartObjectLayer_32bit) -> str:
        ...
        
    def filename(self: SmartObjectLayer_32bit) -> str:
        ...
        
    def filepath(self: SmartObjectLayer_32bit) -> str:
        ...
        
    def original_image_data(self: SmartObjectLayer_32bit) -> Dict[int, numpy.ndarray]:
        ...
        
    def original_width(self: SmartObjectLayer_32bit) -> int:
        ...
        
    def original_height(self: SmartObjectLayer_32bit) -> int:
        ...
        
    def move(self: SmartObjectLayer_32bit, x_offset: float, y_offset: float) -> None:
        ...
        
    def rotate(self: SmartObjectLayer_32bit, angle: float, x: float, y: float) -> None:
        ...
        
    def scale(self: SmartObjectLayer_32bit, x_scalar: float, y_scalar: float, x: float, y: float) -> None:
        ...
        
    def transform(self: SmartObjectLayer_32bit, matrix: numpy.ndarray) -> None:
        ...
        
    def reset_warp(self: SmartObjectLayer_32bit) -> None:
        ...
        
    def reset_transform(self: SmartObjectLayer_32bit) -> None:
        ...