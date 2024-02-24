from typing import overload, Optional
import numpy

import psapi.enum
import psapi.util
from ._layer import Layer_8bit, Layer_16bit, Layer_32bit


class ImageLayer_8bit(Layer_8bit):
    '''
    This class defines a single image layer in a LayeredFile. There must be at least one of these
    in any given file for it to be valid
    
    Attributes
    -----------

    image_data : dict[numpy.ndarray]
        A dictionary of the image data mapped by :class:`psapi.util.ChannelIDInfo`
    name : str
        The name of the layer, cannot be longer than 255
    layer_mask : LayerMask_8bit
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
    def image_data(self: ImageLayer_8bit) -> dict[psapi.util.ChannelIDInfo, numpy.ndarray]:
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
        '''
        Construct an image layer from image data passed as numpy.ndarray

        :param image_data: 
            The image data as 2- or 3-Dimensional numpy array where the first dimension is the number of channels.
    
            If its a 2D ndarray the second dimension must hold the image data in row-major order with the size being height*width. 
            An example could be the following shape: (3, 1024) for an RGB layer that is 32*32px. 

            If its a 3D ndarray the second and third dimension hold height and width respectively.
            An example could be the following shape: (3, 32, 32) for the same RGB layer

            We also support adding alpha channels this way, those are always stored as the last channel and are optional. E.g. for RGB
            there could be a ndarray like this (4, 32, 32) and it would automatically identify the last channel as alpha. For the individual
            color modes there is always a set of required channels such as R, G and B for RGB or C, M, Y, K for CMYK with the optional alpha
            that can be appended to the end.

            The size **must** be the same as the width and height parameter
        :type image_data: numpy.ndarray

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

        :raises:
            ValueError: if length of layer name is greater than 255

            ValueError: if size of layer mask is not width*height

            ValueError: if width of layer is negative

            ValueError: if height of layer is negative

            ValueError: if opacity is not between 0-255

            ValueError: if the channel size is not the same as width * height
        '''
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
        '''
        Construct an image layer from image data passed as dict with integers as key

        :param image_data: 
            The image data as a dictionary with channel indices as integers. E.g. for a RGB image layer 
            
            .. code-block:: python

                data = {
                    0 : numpy.ndarray,
                    1 : numpy.ndarray,
                    2 : numpy.ndarray
                }

        :type image_data: dict[numpy.ndarray]

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

        :raises:
            ValueError: if length of layer name is greater than 255

            ValueError: if size of layer mask is not width*height

            ValueError: if width of layer is negative

            ValueError: if height of layer is negative

            ValueError: if opacity is not between 0-255

            ValueError: if the channel size is not the same as width * height
        '''
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
        '''
        Construct an image layer from image data passed as dict with psapi.enum.ChannelID as key

        :param image_data: 
            The image data as a dictionary with channel IDs as enums. E.g. for a RGB image layer 

            .. code-block:: python

                data = {
                    psapi.enum.ChannelID.red : numpy.ndarray,
                    psapi.enum.ChannelID.green : numpy.ndarray,
                    psapi.enum.ChannelID.blue : numpy.ndarray
                }

        :type image_data: dict[numpy.ndarray]

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

        :raises:
            ValueError: if length of layer name is greater than 255

            ValueError: if size of layer mask is not width*height

            ValueError: if width of layer is negative

            ValueError: if height of layer is negative

            ValueError: if opacity is not between 0-255

            ValueError: if the channel size is not the same as width * height
        '''

    def get_channel_by_id(self: ImageLayer_8bit, id: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        '''
        Extract a specified channel from the layer given its channel ID.
                
        :param id: The ID of the channel
        :type id: :class:`psapi.enum.ChannelID`

        :param do_copy: Defaults to true, whether to copy the data on extraction (if false the channel is invalidated)
        :type do_copy: bool            

        :return: The extracted channel
        :rtype: numpy.ndarray
        '''
        ...

    def get_channel_by_index(self: ImageLayer_8bit, index: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        '''
        Extract a specified channel from the layer given its channel index.
                
        :param index: The index of the channel
        :type index: int

        :param do_copy: Defaults to true, whether to copy the data on extraction (if false the channel is invalidated)
        :type do_copy: bool            

        :return: The extracted channel with dimensions (height, width)
        :rtype: numpy.ndarray
        '''
        ...

    def get_image_data(self: ImageLayer_8bit, do_copy: bool = ...) -> dict[psapi.util.ChannelIDInfo, numpy.ndarray]:
        '''
        Extract all the channels of the ImageLayer into an unordered_map.
                
        :param do_copy: Defaults to true, Whether to copy the data
        :type do_copy: bool

        :return: The extracted image data
        :rtype: dict[psapi.util.ChannelIDInfo, numpy.ndarray]
        '''
        ...

    def set_compression(self: ImageLayer_8bit, compression: psapi.enum.Compression) -> None:
        '''
        Change the compression codec of all the image channels.
                
        :param compression: The compression codec
        :type compression: :class:`psapi.enum.Compression`
        '''
        ...

    def __getitem__(self: ImageLayer_8bit, key: psapi.enum.ChannelID | int) -> numpy.ndarray:
        '''
        Extract a specified channel from the layer given its channel index.
                
        :param key: The ID or index of the channel
        :type key: :class:`psapi.enum.ChannelID` | int

        :return: The extracted channel with dimensions (height, width)
        :rtype: np.ndarray
        '''
        ...


class ImageLayer_16bit(Layer_16bit):
    '''
    This class defines a single image layer in a LayeredFile. There must be at least one of these
    in any given file for it to be valid
    
    Attributes
    -----------

    image_data : dict[:class:`psapi.util.ImageChannel`]
        A dictionary of the image data mapped by :class:`psapi.util.ChannelIDInfo`
    name : str
        The name of the layer, cannot be longer than 255
    layer_mask : LayerMask_16bit
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
    def image_data(self: ImageLayer_16bit) -> dict[psapi.util.ChannelIDInfo, numpy.ndarray]:
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
        '''
        Construct an image layer from image data passed as numpy.ndarray

        :param image_data: 
            The image data as 2- or 3-Dimensional numpy array where the first dimension is the number of channels.
    
            If its a 2D ndarray the second dimension must hold the image data in row-major order with the size being height*width. 
            An example could be the following shape: (3, 1024) for an RGB layer that is 32*32px. 

            If its a 3D ndarray the second and third dimension hold height and width respectively.
            An example could be the following shape: (3, 32, 32) for the same RGB layer

            We also support adding alpha channels this way, those are always stored as the last channel and are optional. E.g. for RGB
            there could be a ndarray like this (4, 32, 32) and it would automatically identify the last channel as alpha. For the individual
            color modes there is always a set of required channels such as R, G and B for RGB or C, M, Y, K for CMYK with the optional alpha
            that can be appended to the end.

            The size **must** be the same as the width and height parameter
        :type image_data: numpy.ndarray

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

        :raises:
            ValueError: if length of layer name is greater than 255

            ValueError: if size of layer mask is not width*height

            ValueError: if width of layer is negative

            ValueError: if height of layer is negative

            ValueError: if opacity is not between 0-255

            ValueError: if the channel size is not the same as width * height
        '''
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
        '''
        Construct an image layer from image data passed as dict with integers as key

        :param image_data: 
            The image data as a dictionary with channel indices as integers. E.g. for a RGB image layer 
            
            .. code-block:: python

                data = {
                    0 : numpy.ndarray,
                    1 : numpy.ndarray,
                    2 : numpy.ndarray
                }

        :type image_data: dict[numpy.ndarray]

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

        :raises:
            ValueError: if length of layer name is greater than 255

            ValueError: if size of layer mask is not width*height

            ValueError: if width of layer is negative

            ValueError: if height of layer is negative

            ValueError: if opacity is not between 0-255

            ValueError: if the channel size is not the same as width * height
        '''
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
        '''
        Construct an image layer from image data passed as dict with psapi.enum.ChannelID as key

        :param image_data: 
            The image data as a dictionary with channel IDs as enums. E.g. for a RGB image layer 

            .. code-block:: python

                data = {
                    psapi.enum.ChannelID.red : numpy.ndarray,
                    psapi.enum.ChannelID.green : numpy.ndarray,
                    psapi.enum.ChannelID.blue : numpy.ndarray
                }

        :type image_data: dict[numpy.ndarray]

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

        :raises:
            ValueError: if length of layer name is greater than 255

            ValueError: if size of layer mask is not width*height

            ValueError: if width of layer is negative

            ValueError: if height of layer is negative

            ValueError: if opacity is not between 0-255

            ValueError: if the channel size is not the same as width * height
        '''

    def get_channel_by_id(self: ImageLayer_16bit, id: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        '''
        Extract a specified channel from the layer given its channel ID.
                
        :param id: The ID of the channel
        :type id: :class:`psapi.enum.ChannelID`

        :param do_copy: Defaults to true, whether to copy the data on extraction (if false the channel is invalidated)
        :type do_copy: bool            

        :return: The extracted channel
        :rtype: numpy.ndarray
        '''
        ...

    def get_channel_by_index(self: ImageLayer_16bit, index: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        '''
        Extract a specified channel from the layer given its channel index.
                
        :param index: The index of the channel
        :type index: int

        :param do_copy: Defaults to true, whether to copy the data on extraction (if false the channel is invalidated)
        :type do_copy: bool            

        :return: The extracted channel with dimensions (height, width)
        :rtype: numpy.ndarray
        '''
        ...

    def get_image_data(self: ImageLayer_16bit, do_copy: bool = ...) -> dict[psapi.util.ChannelIDInfo, numpy.ndarray]:
        '''
        Extract all the channels of the ImageLayer into an unordered_map.
                
        :param do_copy: Defaults to true, Whether to copy the data
        :type do_copy: bool

        :return: The extracted image data
        :rtype: dict[psapi.util.ChannelIDInfo, numpy.ndarray]
        '''
        ...

    def set_compression(self: ImageLayer_16bit, compression: psapi.enum.Compression) -> None:
        '''
        Change the compression codec of all the image channels.
                
        :param compression: The compression codec
        :type compression: :class:`psapi.enum.Compression`
        '''
        ...

    def __getitem__(self: ImageLayer_16bit, key: psapi.enum.ChannelID | int) -> numpy.ndarray:
        '''
        Extract a specified channel from the layer given its channel index.
                
        :param key: The ID or index of the channel
        :type key: :class:`psapi.enum.ChannelID` | int

        :return: The extracted channel with dimensions (height, width)
        :rtype: np.ndarray
        '''
        ...


class ImageLayer_32bit(Layer_32bit):
    '''
    This class defines a single image layer in a LayeredFile. There must be at least one of these
    in any given file for it to be valid
    
    Attributes
    -----------

    image_data : dict[:class:`psapi.util.ImageChannel`]
        A dictionary of the image data mapped by :class:`psapi.util.ChannelIDInfo`
    name : str
        The name of the layer, cannot be longer than 255
    layer_mask : LayerMask_32bit
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
    def image_data(self: ImageLayer_32bit) -> dict[psapi.util.ChannelIDInfo, numpy.ndarray]:
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
        '''
        Construct an image layer from image data passed as numpy.ndarray

        :param image_data: 
            The image data as 2- or 3-Dimensional numpy array where the first dimension is the number of channels.
    
            If its a 2D ndarray the second dimension must hold the image data in row-major order with the size being height*width. 
            An example could be the following shape: (3, 1024) for an RGB layer that is 32*32px. 

            If its a 3D ndarray the second and third dimension hold height and width respectively.
            An example could be the following shape: (3, 32, 32) for the same RGB layer

            We also support adding alpha channels this way, those are always stored as the last channel and are optional. E.g. for RGB
            there could be a ndarray like this (4, 32, 32) and it would automatically identify the last channel as alpha. For the individual
            color modes there is always a set of required channels such as R, G and B for RGB or C, M, Y, K for CMYK with the optional alpha
            that can be appended to the end.

            The size **must** be the same as the width and height parameter
        :type image_data: numpy.ndarray

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

        :raises:
            ValueError: if length of layer name is greater than 255

            ValueError: if size of layer mask is not width*height

            ValueError: if width of layer is negative

            ValueError: if height of layer is negative

            ValueError: if opacity is not between 0-255

            ValueError: if the channel size is not the same as width * height
        '''
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
        '''
        Construct an image layer from image data passed as dict with integers as key

        :param image_data: 
            The image data as a dictionary with channel indices as integers. E.g. for a RGB image layer 
            
            .. code-block:: python

                data = {
                    0 : numpy.ndarray,
                    1 : numpy.ndarray,
                    2 : numpy.ndarray
                }

        :type image_data: dict[numpy.ndarray]

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

        :raises:
            ValueError: if length of layer name is greater than 255

            ValueError: if size of layer mask is not width*height

            ValueError: if width of layer is negative

            ValueError: if height of layer is negative

            ValueError: if opacity is not between 0-255

            ValueError: if the channel size is not the same as width * height
        '''
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
        '''
        Construct an image layer from image data passed as dict with psapi.enum.ChannelID as key

        :param image_data: 
            The image data as a dictionary with channel IDs as enums. E.g. for a RGB image layer 

            .. code-block:: python

                data = {
                    psapi.enum.ChannelID.red : numpy.ndarray,
                    psapi.enum.ChannelID.green : numpy.ndarray,
                    psapi.enum.ChannelID.blue : numpy.ndarray
                }

        :type image_data: dict[numpy.ndarray]

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

        :raises:
            ValueError: if length of layer name is greater than 255

            ValueError: if size of layer mask is not width*height

            ValueError: if width of layer is negative

            ValueError: if height of layer is negative

            ValueError: if opacity is not between 0-255

            ValueError: if the channel size is not the same as width * height
        '''

    def get_channel_by_id(self: ImageLayer_32bit, id: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        '''
        Extract a specified channel from the layer given its channel ID.
                
        :param id: The ID of the channel
        :type id: :class:`psapi.enum.ChannelID`

        :param do_copy: Defaults to true, whether to copy the data on extraction (if false the channel is invalidated)
        :type do_copy: bool            

        :return: The extracted channel
        :rtype: numpy.ndarray
        '''
        ...

    def get_channel_by_index(self: ImageLayer_32bit, index: psapi.enum.ChannelID, do_copy: bool = ...) -> numpy.ndarray:
        '''
        Extract a specified channel from the layer given its channel index.
                
        :param index: The index of the channel
        :type index: int

        :param do_copy: Defaults to true, whether to copy the data on extraction (if false the channel is invalidated)
        :type do_copy: bool            

        :return: The extracted channel with dimensions (height, width)
        :rtype: numpy.ndarray
        '''
        ...

    def get_image_data(self: ImageLayer_32bit, do_copy: bool = ...) -> dict[psapi.util.ChannelIDInfo, numpy.ndarray]:
        '''
        Extract all the channels of the ImageLayer into an unordered_map.
                
        :param do_copy: Defaults to true, Whether to copy the data
        :type do_copy: bool

        :return: The extracted image data
        :rtype: dict[psapi.util.ChannelIDInfo, numpy.ndarray]
        '''
        ...

    def set_compression(self: ImageLayer_32bit, compression: psapi.enum.Compression) -> None:
        '''
        Change the compression codec of all the image channels.
                
        :param compression: The compression codec
        :type compression: :class:`psapi.enum.Compression`
        '''
        ...

    def __getitem__(self: ImageLayer_32bit, key: psapi.enum.ChannelID | int) -> numpy.ndarray:
        '''
        Extract a specified channel from the layer given its channel index.
                
        :param key: The ID or index of the channel
        :type key: :class:`psapi.enum.ChannelID` | int

        :return: The extracted channel with dimensions (height, width)
        :rtype: np.ndarray
        '''
        ...
