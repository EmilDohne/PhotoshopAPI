import numpy

import psapi.enum
import psapi.util


class Layer_8bit:
    '''
    Base type that all layers inherit from, this class should not be instantiated
    and instead the derivatives such as :class:`psapi.GroupLayer_8bit` or :class:`psapi.ImageLayer_8bit`
    should be used (with the appropriate bit depth).

    Attributes
    -----------

    name : str
        The name of the layer, cannot be longer than 255
    layer_mask : np.ndarray
        The pixel mask applied to the layer, read only
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
        '''
        Get the pixel mask data associated with the layer (if it exists), if it doesnt
        a warning gets raised and a null-size numpy.ndarray is returned.

        The size of the mask is not necessarily the same as the layer

        :param do_copy: Whether or not to copy the image data on extraction, if False the mask channel is freed
        :type do_copy: bool

        :return: The extracted channel with dimensions (mask_height, mask_width)
        :rtype: numpy.ndarray
        '''
        ...


class Layer_16bit:
    '''
    Base type that all layers inherit from, this class should not be instantiated
    and instead the derivatives such as :class:`psapi.GroupLayer_16bit` or :class:`psapi.ImageLayer_16bit`
    should be used (with the appropriate bit depth).

    Attributes
    -----------

    name : str
        The name of the layer, cannot be longer than 255
    layer_mask : np.ndarray
        The pixel mask applied to the layer, read only
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
        '''
        Get the pixel mask data associated with the layer (if it exists), if it doesnt
        a warning gets raised and a null-size numpy.ndarray is returned.

        The size of the mask is not necessarily the same as the layer

        :param do_copy: Whether or not to copy the image data on extraction, if False the mask channel is freed
        :type do_copy: bool

        :return: The extracted channel with dimensions (mask_height, mask_width)
        :rtype: numpy.ndarray
        '''
        ...


class Layer_32bit:
    '''
    Base type that all layers inherit from, this class should not be instantiated
    and instead the derivatives such as :class:`psapi.GroupLayer_32bit` or :class:`psapi.ImageLayer_32bit`
    should be used (with the appropriate bit depth).

    Attributes
    -----------

    name : str
        The name of the layer, cannot be longer than 255
    layer_mask : np.ndarray
        The pixel mask applied to the layer, read only
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
        '''
        Get the pixel mask data associated with the layer (if it exists), if it doesnt
        a warning gets raised and a null-size numpy.ndarray is returned.

        The size of the mask is not necessarily the same as the layer

        :param do_copy: Whether or not to copy the image data on extraction, if False the mask channel is freed
        :type do_copy: bool

        :return: The extracted channel with dimensions (mask_height, mask_width)
        :rtype: numpy.ndarray
        '''
        ...
        