from typing import overload, Optional
import numpy

import psapi.enum
import psapi.util
from ._layer import Layer_8bit, Layer_16bit, Layer_32bit
from ._layered_file import LayeredFile_8bit, LayeredFile_16bit, LayeredFile_32bit


class GroupLayer_8bit(Layer_8bit):
    '''
    
    Attributes
    -----------

    layers : list[psapi.Layer_8bit]
        The layers under the group, may be empty. These are polymorphic so it may be a group layer, an image layer etc.
        Retrieving them will cast them to their appropriate type
    is_collapsed : bool
        Whether or not the group is collapsed or not
    name : str
        The name of the layer, cannot be longer than 255
    layer_mask : psapi.LayerMask_*
        The pixel mask applied to the layer
    blend_mode : enum.BlendMode
        The blend mode of the layer, 'Passthrough' is reserved for group layers
    opacity : int
        The layers opacity from 0-255 with 255 being 100%
    width : int
        The width of the layer ranging up to 30,000 for PSD and 300,000 for PSB,
        this does not have to match the files width
    height : int
        The height of the layer ranging up to 30,000 for PSD and 300,000 for PSB,
        this does not have to match the files height
    center_x : float
        The center of the layer in regards to the canvas, a layer at center_x = 0 is
        perfectly centered around the document
    center_y : float
        The center of the layer in regards to the canvas, a layer at center_y = 0 is
        perfectly centered around the document
    '''

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
        '''
        Construct a group layer instance

        :param layer_name: The name of the group, its length must not exceed 255
        :type layer_name: str

        :param layer_mask: 
            Optional layer mask, must have the same dimensions as height * width but can be a 1- or 2-dimensional array with row-major ordering (for a numpy
            2D array this would mean with a shape of (height, width)
        :type layer_mask: numpy.ndarray

        :param width: 
            Optional, width of the layer, does not have to be the same size as the document, limited to 30,000 for PSD files and 300,000 for PSB files.
            For group layers this is only relevant for the layer mask and can be left out otherwise
        :type width: int

        :param height: 
            Optional, height of the layer, does not have to be the same size as the document, limited to 30,000 for PSD files and 300,000 for PSB files.
            For group layers this is only relevant for the layer mask and can be left out otherwise
        :type height: int

        :param blend_mode: Optional, the blend mode of the layer, 'Passthrough' is the default for groups.
        :type blend_mode: psapi.enum.BlendMode

        :param pos_x: 
            Optional, the relative offset of the layer to the center of the document, 0 indicates the layer is centered.
            For group layers this is only relevant for the layer mask and can be left out otherwise
        :type pos_x: int

        :param pos_y: 
            Optional, the relative offset of the layer to the center of the document, 0 indicates the layer is centered.
            For group layers this is only relevant for the layer mask and can be left out otherwise
        :type pos_y: int

        :param opacity: The opacity of the layer from 0-255 where 0 is 0% and 255 is 100%. Defaults to 255
        :type opacity: int

        :param compression: The compression to apply to all the channels of the layer, including mask channels
        :type compression: psapi.enum.Compression

        :param color_mode: The color mode of the Layer, this must be identical to the color mode of the document. Defaults to RGB
        :type color_mode: psapi.enum.ColorMode

        :param is_collapsed: Whether or not the group is collapsed (closed)
        :type is_collapsed: bool

        :raises:
            ValueError: if length of layer name is greater than 255

            ValueError: if size of layer mask is not width*height

            ValueError: if width of layer is negative

            ValueError: if height of layer is negative

            ValueError: if opacity is not between 0-255
        '''
        ...

    def add_layer(self: GroupLayer_8bit, layered_file: LayeredFile_8bit, layer: Layer_8bit) -> None:
        '''
        Add the specified layer to the group

        :param layered_file: The top level LayeredFile instance, required to ensure a layer doesnt get added twice
        :type layered_file: psapi.LayeredFile_8bit

        :param layer: the layer instance to insert under the group
        :type layer: Layer_8bit
        '''
        ...

    @overload
    def remove_layer(self: GroupLayer_8bit, index: int) -> None:
        '''
        Remove the specified layer from the group, raises a warning if the index isnt valid

        :param index: The index of the layer to be removed
        :type index: int
        '''
        ...

    @overload
    def remove_layer(self: GroupLayer_8bit, layer: Layer_8bit) -> None:
        '''
        Remove the specified layer from the group, raises a warning if the layer isnt under the group

        :param layer: The layer to be removed
        :type layer: Layer_8bit
        '''
        ...
    
    @overload
    def remove_layer(self: GroupLayer_8bit, layer_name: str) -> None:
        '''
        Remove the specified layer from the group, raises a warning if the layer isnt under the group

        :param layer_name: The layer to be removed
        :type layer_name: str
        '''
        ...

    def __getitem__(self: GroupLayer_8bit, value: str) -> Layer_8bit:
        '''
        Get the specified layer from the group using dict-like indexing. This may be chained as deep as the layer hierarchy goes

        .. code-block:: python

            group_layer: GroupLayer_8bit = # Our group layer instance
            nested_img_layer = group_layer["NestedGroup"]["Image"]

        :param value: The name of the layer to search for
        :type value: str

        :raises:
            KeyError: If the requested layer is not found
            
        :return: The requested layer instance
        '''
        ...


class GroupLayer_16bit(Layer_16bit):
    '''
    
    Attributes
    -----------

    layers : list[psapi.Layer_16bit]
        The layers under the group, may be empty. These are polymorphic so it may be a group layer, an image layer etc.
        Retrieving them will cast them to their appropriate type
    is_collapsed : bool
        Whether or not the group is collapsed or not
    name : str
        The name of the layer, cannot be longer than 255
    layer_mask : psapi.LayerMask_*
        The pixel mask applied to the layer
    blend_mode : enum.BlendMode
        The blend mode of the layer, 'Passthrough' is reserved for group layers
    opacity : int
        The layers opacity from 0-255 with 255 being 100%
    width : int
        The width of the layer ranging up to 30,000 for PSD and 300,000 for PSB,
        this does not have to match the files width
    height : int
        The height of the layer ranging up to 30,000 for PSD and 300,000 for PSB,
        this does not have to match the files height
    center_x : float
        The center of the layer in regards to the canvas, a layer at center_x = 0 is
        perfectly centered around the document
    center_y : float
        The center of the layer in regards to the canvas, a layer at center_y = 0 is
        perfectly centered around the document
    '''

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
        '''
        Construct a group layer instance

        :param layer_name: The name of the group, its length must not exceed 255
        :type layer_name: str

        :param layer_mask: 
            Optional layer mask, must have the same dimensions as height * width but can be a 1- or 2-dimensional array with row-major ordering (for a numpy
            2D array this would mean with a shape of (height, width)
        :type layer_mask: numpy.ndarray

        :param width: 
            Optional, width of the layer, does not have to be the same size as the document, limited to 30,000 for PSD files and 300,000 for PSB files.
            For group layers this is only relevant for the layer mask and can be left out otherwise
        :type width: int

        :param height: 
            Optional, height of the layer, does not have to be the same size as the document, limited to 30,000 for PSD files and 300,000 for PSB files.
            For group layers this is only relevant for the layer mask and can be left out otherwise
        :type height: int

        :param blend_mode: Optional, the blend mode of the layer, 'Passthrough' is the default for groups.
        :type blend_mode: psapi.enum.BlendMode

        :param pos_x: 
            Optional, the relative offset of the layer to the center of the document, 0 indicates the layer is centered.
            For group layers this is only relevant for the layer mask and can be left out otherwise
        :type pos_x: int

        :param pos_y: 
            Optional, the relative offset of the layer to the center of the document, 0 indicates the layer is centered.
            For group layers this is only relevant for the layer mask and can be left out otherwise
        :type pos_y: int

        :param opacity: The opacity of the layer from 0-255 where 0 is 0% and 255 is 100%. Defaults to 255
        :type opacity: int

        :param compression: The compression to apply to all the channels of the layer, including mask channels
        :type compression: psapi.enum.Compression

        :param color_mode: The color mode of the Layer, this must be identical to the color mode of the document. Defaults to RGB
        :type color_mode: psapi.enum.ColorMode

        :param is_collapsed: Whether or not the group is collapsed (closed)
        :type is_collapsed: bool

        :raises:
            ValueError: if length of layer name is greater than 255

            ValueError: if size of layer mask is not width*height

            ValueError: if width of layer is negative

            ValueError: if height of layer is negative

            ValueError: if opacity is not between 0-255
        '''
        ...

    def add_layer(self: GroupLayer_16bit, layered_file: LayeredFile_16bit, layer: Layer_16bit) -> None:
        '''
        Add the specified layer to the group

        :param layered_file: The top level LayeredFile instance, required to ensure a layer doesnt get added twice
        :type layered_file: psapi.LayeredFile_16bit

        :param layer: the layer instance to insert under the group
        :type layer: Layer_16bit
        '''
        ...

    @overload
    def remove_layer(self: GroupLayer_16bit, index: int) -> None:
        '''
        Remove the specified layer from the group, raises a warning if the index isnt valid

        :param index: The index of the layer to be removed
        :type index: int
        '''
        ...

    @overload
    def remove_layer(self: GroupLayer_16bit, layer: Layer_16bit) -> None:
        '''
        Remove the specified layer from the group, raises a warning if the layer isnt under the group

        :param layer: The layer to be removed
        :type layer: Layer_16bit
        '''
        ...
    
    @overload
    def remove_layer(self: GroupLayer_16bit, layer_name: str) -> None:
        '''
        Remove the specified layer from the group, raises a warning if the layer isnt under the group

        :param layer_name: The layer to be removed
        :type layer_name: str
        '''
        ...

    def __getitem__(self: GroupLayer_16bit, value: str) -> Layer_16bit:
        '''
        Get the specified layer from the group using dict-like indexing. This may be chained as deep as the layer hierarchy goes

        .. code-block:: python

            group_layer: GroupLayer_16bit = # Our group layer instance
            nested_img_layer = group_layer["NestedGroup"]["Image"]

        :param value: The name of the layer to search for
        :type value: str

        :raises:
            KeyError: If the requested layer is not found
            
        :return: The requested layer instance
        '''
        ...


class GroupLayer_32bit(Layer_32bit):
    '''
    
    Attributes
    -----------

    layers : list[psapi.Layer_32bit]
        The layers under the group, may be empty. These are polymorphic so it may be a group layer, an image layer etc.
        Retrieving them will cast them to their appropriate type
    is_collapsed : bool
        Whether or not the group is collapsed or not
    name : str
        The name of the layer, cannot be longer than 255
    layer_mask : psapi.LayerMask_*
        The pixel mask applied to the layer
    blend_mode : enum.BlendMode
        The blend mode of the layer, 'Passthrough' is reserved for group layers
    opacity : int
        The layers opacity from 0-255 with 255 being 100%
    width : int
        The width of the layer ranging up to 30,000 for PSD and 300,000 for PSB,
        this does not have to match the files width
    height : int
        The height of the layer ranging up to 30,000 for PSD and 300,000 for PSB,
        this does not have to match the files height
    center_x : float
        The center of the layer in regards to the canvas, a layer at center_x = 0 is
        perfectly centered around the document
    center_y : float
        The center of the layer in regards to the canvas, a layer at center_y = 0 is
        perfectly centered around the document
    '''

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
        '''
        Construct a group layer instance

        :param layer_name: The name of the group, its length must not exceed 255
        :type layer_name: str

        :param layer_mask: 
            Optional layer mask, must have the same dimensions as height * width but can be a 1- or 2-dimensional array with row-major ordering (for a numpy
            2D array this would mean with a shape of (height, width)
        :type layer_mask: numpy.ndarray

        :param width: 
            Optional, width of the layer, does not have to be the same size as the document, limited to 30,000 for PSD files and 300,000 for PSB files.
            For group layers this is only relevant for the layer mask and can be left out otherwise
        :type width: int

        :param height: 
            Optional, height of the layer, does not have to be the same size as the document, limited to 30,000 for PSD files and 300,000 for PSB files.
            For group layers this is only relevant for the layer mask and can be left out otherwise
        :type height: int

        :param blend_mode: Optional, the blend mode of the layer, 'Passthrough' is the default for groups.
        :type blend_mode: psapi.enum.BlendMode

        :param pos_x: 
            Optional, the relative offset of the layer to the center of the document, 0 indicates the layer is centered.
            For group layers this is only relevant for the layer mask and can be left out otherwise
        :type pos_x: int

        :param pos_y: 
            Optional, the relative offset of the layer to the center of the document, 0 indicates the layer is centered.
            For group layers this is only relevant for the layer mask and can be left out otherwise
        :type pos_y: int

        :param opacity: The opacity of the layer from 0-255 where 0 is 0% and 255 is 100%. Defaults to 255
        :type opacity: int

        :param compression: The compression to apply to all the channels of the layer, including mask channels
        :type compression: psapi.enum.Compression

        :param color_mode: The color mode of the Layer, this must be identical to the color mode of the document. Defaults to RGB
        :type color_mode: psapi.enum.ColorMode

        :param is_collapsed: Whether or not the group is collapsed (closed)
        :type is_collapsed: bool

        :raises:
            ValueError: if length of layer name is greater than 255

            ValueError: if size of layer mask is not width*height

            ValueError: if width of layer is negative

            ValueError: if height of layer is negative

            ValueError: if opacity is not between 0-255
        '''
        ...

    def add_layer(self: GroupLayer_32bit, layered_file: LayeredFile_32bit, layer: Layer_32bit) -> None:
        '''
        Add the specified layer to the group

        :param layered_file: The top level LayeredFile instance, required to ensure a layer doesnt get added twice
        :type layered_file: psapi.LayeredFile_32bit

        :param layer: the layer instance to insert under the group
        :type layer: Layer_32bit
        '''
        ...

    @overload
    def remove_layer(self: GroupLayer_32bit, index: int) -> None:
        '''
        Remove the specified layer from the group, raises a warning if the index isnt valid

        :param index: The index of the layer to be removed
        :type index: int
        '''
        ...

    @overload
    def remove_layer(self: GroupLayer_32bit, layer: Layer_32bit) -> None:
        '''
        Remove the specified layer from the group, raises a warning if the layer isnt under the group

        :param layer: The layer to be removed
        :type layer: Layer_32bit
        '''
        ...
    
    @overload
    def remove_layer(self: GroupLayer_32bit, layer_name: str) -> None:
        '''
        Remove the specified layer from the group, raises a warning if the layer isnt under the group

        :param layer_name: The layer to be removed
        :type layer_name: str
        '''
        ...

    def __getitem__(self: GroupLayer_32bit, value: str) -> Layer_32bit:
        '''
        Get the specified layer from the group using dict-like indexing. This may be chained as deep as the layer hierarchy goes

        .. code-block:: python

            group_layer: GroupLayer_32bit = # Our group layer instance
            nested_img_layer = group_layer["NestedGroup"]["Image"]

        :param value: The name of the layer to search for
        :type value: str

        :raises:
            KeyError: If the requested layer is not found
            
        :return: The requested layer instance
        '''
        ...