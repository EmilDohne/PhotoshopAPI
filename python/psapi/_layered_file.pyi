from typing import overload, Optional
import numpy
import os

import psapi.enum
import psapi.util
from ._layer import Layer_8bit, Layer_16bit, Layer_32bit


class LayeredFile:
    '''
    A wrapper class for the different LayeredFile subtypes that we can call read() on to
    return the appropriate LayeredFile instance.

    .. warning::
        
        The psapi.LayeredFile class' only job is to simplify the read of a LayeredFile_*bit from 
        disk with automatic type deduction. It does not however hold any of the data itself.
    '''
			
    @staticmethod
    def read(path: os.PathLike) -> LayeredFile_8bit | LayeredFile_16bit | LayeredFile_32bit:
        '''
        Read a layeredfile into the appropriate type based on the actual bit-depth
        of the document

        :param path: The path to the Photoshop file
        :type path: str

        :rtype: :class:`psapi.LayeredFile_8bit` | :class:`psapi.LayeredFile_16bit` | :class:`psapi.LayeredFile_32bit`
        '''
        ...


class LayeredFile_8bit:
    '''
    This class defines a layered file structure, where each file contains a hierarchy of layers. Layers can be grouped and organized within this structure.

    Attributes
    -------------
    icc : numpy.ndarray
        Property for setting and retrieving the ICC profile attached to the file. This does not do any color conversions
        but simply tells photoshop how to interpret the data. The assignment is overloaded such that you need to pass
        a path to the ICC file you want to load and loading will be done internally.

    compression : psapi.enum.Compression
        Write-only property which sets the compression of all the layers in the LayeredFile

    num_channels : int
        Read-only property to retrieve the number of channels from the file (excludes mask channels)

    bit_depth : psapi.enum.BitDepth
        Read-only property to retrieve the bit-depth

    layers : list[Layer_8bit]
        Read-only property to retrieve a list of all the layers in the root of the file

    dpi : int
        The document DPI settings

    width : int
        The width of the document, must not exceed 30,000 for PSD or 300,000 for PSB

    height : int
        The height of the document, must not exceed 30,000 for PSD or 300,000 for PSB
    '''

    @property
    def icc(self: LayeredFile_8bit) -> numpy.ndarray:
        ...

    @icc.setter
    def icc(self: LayeredFile_8bit, path: os.PathLike) -> None:
        ...

    # We put it here as its property-like
    def compression(self: LayeredFile_8bit, compression: psapi.enum.Compression) -> None:
        ...

    @property
    def num_channels(self: LayeredFile_8bit) -> int:
        ...

    @property
    def layers(self: LayeredFile_8bit) -> list[Layer_8bit]:
        ...

    @property
    def bit_depth(self: LayeredFile_8bit) -> psapi.enum.BitDepth:
        ...

    @property
    def dpi(self: LayeredFile_8bit) -> float:
        ...

    @dpi.setter
    def dpi(self: LayeredFile_8bit, value: float) -> None:
        ...

    @property
    def width(self: LayeredFile_8bit) -> int:
        ...

    @width.setter
    def width(self: LayeredFile_8bit, value: int) -> None:
        ...
    
    @property
    def height(self: LayeredFile_8bit) -> int:
        ...

    @height.setter
    def height(self: LayeredFile_8bit, value: int) -> None:
        ...
    
    @overload
    def __init__(self: LayeredFile_8bit) -> None:
        ...

    @overload
    def __init__(self: LayeredFile_8bit, color_mode: psapi.enum.ColorMode, width: int, height: int) -> None:
        ...

    def find_layer(self: LayeredFile_8bit, path: str) -> Layer_8bit:
        '''
        Find a layer based on the given path

        :param path: The path to the requested layer
        :type path: str

        :return: The requested layer

        :raises:
            ValueError: If the path is not a valid path to a layer
        '''
        ...

    def add_layer(self: LayeredFile_8bit, layer: Layer_8bit) -> None:
        ...

    @overload
    def move_layer(self: LayeredFile_8bit, child: Layer_8bit, parent: Optional[Layer_8bit]) -> None:
        '''
        Move the child layer to the provided parent layer, if none is provided we move to scene root
        instead
        '''
        ...

    @overload
    def move_layer(self: LayeredFile_8bit, child: str, parent: Optional[str]) -> None:
        '''
        Move the child layer to the provided parent layer, if none is provided we move to scene root
        instead
        '''
        ...

    @overload
    def remove_layer(self: LayeredFile_8bit, layer: Layer_8bit) -> None:
        ''' 
        Remove the specified layer from root of the layered_file, if you instead wish to remove 
        from a group call remove_layer on a GroupLayer_8bit instance instead
        '''
        ...

    @overload
    def remove_layer(self: LayeredFile_8bit, layer: str) -> None:
        ''' 
        Remove the specified layer from root of the layered_file, if you instead wish to remove 
        from a group call remove_layer on a GroupLayer_8bit instance instead
        '''
        ...

    def is_layer_in_document(self: LayeredFile_8bit, layer: Layer_8bit) -> bool:
        '''
        Check if the layer already exists in the LayeredFile at any level of nesting,
        this check is done internally on add_layer().
        '''
        ...

    @staticmethod
    def read(path: os.PathLike) -> LayeredFile_8bit:
        '''
        Read and create a LayeredFile from disk. If the bit depth isnt known ahead of time use LayeredFile.read() instead 
        which will return the appropriate type
        '''
        ...

    def write(self: LayeredFile_8bit, path: os.PathLike, force_overwrite: bool = ...) -> None:
        '''
        Write the LayeredFile_8bit instance to disk invalidating the data, after this point trying to use the 
        instance is undefined behaviour. 

        :param path: 
            The path of the output file, must have a .psd or .psb extension. Conversion between these two types 
            is taken care of internally
        :type path: 
            os.PathLike
        
        :param force_overwrite: 
            Defaults to True, whether to forcefully overwrite the file if it exists. if False the write-op fails
            and emits an error message
        :type force_overwrite: bool
        '''
        ...

    def __getitem__(self: LayeredFile_8bit, name: str) -> Layer_8bit:
        '''
        Get the specified layer from the root of the layered file. Unlike :func:`find_layer` this does not 
        accept a path but rather a single layer located in the root layer. This is to make chaining of paths
        more pythonic since group layers also implement a __getitem__ function

        .. code-block:: python

            layered_file: LayeredFile_8bit = # Our layered file instance
            nested_img_layer = layered_file["Group"]["Image"]

        :param name: The name of the layer to search for
        :type name: str

        :raises:
            KeyError: If the requested layer is not found

        :return: The requested layer instance
        '''
        ...


class LayeredFile_16bit:
    '''
    This class defines a layered file structure, where each file contains a hierarchy of layers. Layers can be grouped and organized within this structure.

    Attributes
    -------------
    icc : numpy.ndarray
        Property for setting and retrieving the ICC profile attached to the file. This does not do any color conversions
        but simply tells photoshop how to interpret the data. The assignment is overloaded such that you need to pass
        a path to the ICC file you want to load and loading will be done internally.

    compression : psapi.enum.Compression
        Write-only property which sets the compression of all the layers in the LayeredFile

    num_channels : int
        Read-only property to retrieve the number of channels from the file (excludes mask channels)

    bit_depth : psapi.enum.BitDepth
        Read-only property to retrieve the bit-depth

    layers : list[Layer_16bit]
        Read-only property to retrieve a list of all the layers in the root of the file

    dpi : int
        The document DPI settings

    width : int
        The width of the document, must not exceed 30,000 for PSD or 300,000 for PSB

    height : int
        The height of the document, must not exceed 30,000 for PSD or 300,000 for PSB
    '''

    @property
    def icc(self: LayeredFile_16bit) -> numpy.ndarray:
        ...

    @icc.setter
    def icc(self: LayeredFile_16bit, path: os.PathLike) -> None:
        ...

    # We put it here as its property-like
    def compression(self: LayeredFile_16bit, compression: psapi.enum.Compression) -> None:
        ...

    @property
    def num_channels(self: LayeredFile_16bit) -> int:
        ...

    @property
    def layers(self: LayeredFile_16bit) -> list[Layer_16bit]:
        ...

    @property
    def bit_depth(self: LayeredFile_16bit) -> psapi.enum.BitDepth:
        ...

    @property
    def dpi(self: LayeredFile_16bit) -> float:
        ...

    @dpi.setter
    def dpi(self: LayeredFile_16bit, value: float) -> None:
        ...

    @property
    def width(self: LayeredFile_16bit) -> int:
        ...

    @width.setter
    def width(self: LayeredFile_16bit, value: int) -> None:
        ...
    
    @property
    def height(self: LayeredFile_16bit) -> int:
        ...

    @height.setter
    def height(self: LayeredFile_16bit, value: int) -> None:
        ...
    
    @overload
    def __init__(self: LayeredFile_16bit) -> None:
        ...

    @overload
    def __init__(self: LayeredFile_16bit, color_mode: psapi.enum.ColorMode, width: int, height: int) -> None:
        ...

    def find_layer(self: LayeredFile_16bit, path: str) -> Layer_16bit:
        '''
        Find a layer based on the given path

        :param path: The path to the requested layer
        :type path: str

        :return: The requested layer

        :raises:
            ValueError: If the path is not a valid path to a layer
        '''
        ...

    def add_layer(self: LayeredFile_16bit, layer: Layer_16bit) -> None:
        ...

    @overload
    def move_layer(self: LayeredFile_16bit, child: Layer_16bit, parent: Optional[Layer_16bit]) -> None:
        '''
        Move the child layer to the provided parent layer, if none is provided we move to scene root
        instead
        '''
        ...

    @overload
    def move_layer(self: LayeredFile_16bit, child: str, parent: Optional[str]) -> None:
        '''
        Move the child layer to the provided parent layer, if none is provided we move to scene root
        instead
        '''
        ...

    @overload
    def remove_layer(self: LayeredFile_16bit, layer: Layer_16bit) -> None:
        ''' 
        Remove the specified layer from root of the layered_file, if you instead wish to remove 
        from a group call remove_layer on a GroupLayer_16bit instance instead
        '''
        ...

    @overload
    def remove_layer(self: LayeredFile_16bit, layer: str) -> None:
        ''' 
        Remove the specified layer from root of the layered_file, if you instead wish to remove 
        from a group call remove_layer on a GroupLayer_16bit instance instead
        '''
        ...

    def is_layer_in_document(self: LayeredFile_16bit, layer: Layer_16bit) -> bool:
        '''
        Check if the layer already exists in the LayeredFile at any level of nesting,
        this check is done internally on add_layer().
        '''
        ...

    @staticmethod
    def read(path: os.PathLike) -> LayeredFile_16bit:
        '''
        Read and create a LayeredFile from disk. If the bit depth isnt known ahead of time use LayeredFile.read() instead 
        which will return the appropriate type
        '''
        ...

    def write(self: LayeredFile_16bit, path: os.PathLike, force_overwrite: bool = ...) -> None:
        '''
        Write the LayeredFile_16bit instance to disk invalidating the data, after this point trying to use the 
        instance is undefined behaviour. 

        :param path: 
            The path of the output file, must have a .psd or .psb extension. Conversion between these two types 
            is taken care of internally
        :type path: 
            os.PathLike
        
        :param force_overwrite: 
            Defaults to True, whether to forcefully overwrite the file if it exists. if False the write-op fails
            and emits an error message
        :type force_overwrite: bool
        '''
        ...

    def __getitem__(self: LayeredFile_16bit, name: str) -> Layer_16bit:
        '''
        Get the specified layer from the root of the layered file. Unlike :func:`find_layer` this does not 
        accept a path but rather a single layer located in the root layer. This is to make chaining of paths
        more pythonic since group layers also implement a __getitem__ function

        .. code-block:: python

            layered_file: LayeredFile_16bit = # Our layered file instance
            nested_img_layer = layered_file["Group"]["Image"]

        :param name: The name of the layer to search for
        :type name: str

        :raises:
            KeyError: If the requested layer is not found

        :return: The requested layer instance
        '''
        ...


class LayeredFile_32bit:
    '''
    This class defines a layered file structure, where each file contains a hierarchy of layers. Layers can be grouped and organized within this structure.

    Attributes
    -------------
    icc : numpy.ndarray
        Property for setting and retrieving the ICC profile attached to the file. This does not do any color conversions
        but simply tells photoshop how to interpret the data. The assignment is overloaded such that you need to pass
        a path to the ICC file you want to load and loading will be done internally.

    compression : psapi.enum.Compression
        Write-only property which sets the compression of all the layers in the LayeredFile

    num_channels : int
        Read-only property to retrieve the number of channels from the file (excludes mask channels)

    bit_depth : psapi.enum.BitDepth
        Read-only property to retrieve the bit-depth

    layers : list[Layer_32bit]
        Read-only property to retrieve a list of all the layers in the root of the file

    dpi : int
        The document DPI settings

    width : int
        The width of the document, must not exceed 30,000 for PSD or 300,000 for PSB

    height : int
        The height of the document, must not exceed 30,000 for PSD or 300,000 for PSB
    '''

    @property
    def icc(self: LayeredFile_32bit) -> numpy.ndarray:
        ...

    @icc.setter
    def icc(self: LayeredFile_32bit, path: os.PathLike) -> None:
        ...

    # We put it here as its property-like
    def compression(self: LayeredFile_32bit, compression: psapi.enum.Compression) -> None:
        ...

    @property
    def num_channels(self: LayeredFile_32bit) -> int:
        ...

    @property
    def layers(self: LayeredFile_32bit) -> list[Layer_32bit]:
        ...

    @property
    def bit_depth(self: LayeredFile_32bit) -> psapi.enum.BitDepth:
        ...

    @property
    def dpi(self: LayeredFile_32bit) -> float:
        ...

    @dpi.setter
    def dpi(self: LayeredFile_32bit, value: float) -> None:
        ...

    @property
    def width(self: LayeredFile_32bit) -> int:
        ...

    @width.setter
    def width(self: LayeredFile_32bit, value: int) -> None:
        ...
    
    @property
    def height(self: LayeredFile_32bit) -> int:
        ...

    @height.setter
    def height(self: LayeredFile_32bit, value: int) -> None:
        ...
    
    @overload
    def __init__(self: LayeredFile_32bit) -> None:
        ...

    @overload
    def __init__(self: LayeredFile_32bit, color_mode: psapi.enum.ColorMode, width: int, height: int) -> None:
        ...

    def find_layer(self: LayeredFile_32bit, path: str) -> Layer_32bit:
        '''
        Find a layer based on the given path

        :param path: The path to the requested layer
        :type path: str

        :return: The requested layer

        :raises:
            ValueError: If the path is not a valid path to a layer
        '''
        ...

    def add_layer(self: LayeredFile_32bit, layer: Layer_32bit) -> None:
        ...

    @overload
    def move_layer(self: LayeredFile_32bit, child: Layer_32bit, parent: Optional[Layer_32bit]) -> None:
        '''
        Move the child layer to the provided parent layer, if none is provided we move to scene root
        instead
        '''
        ...

    @overload
    def move_layer(self: LayeredFile_32bit, child: str, parent: Optional[str]) -> None:
        '''
        Move the child layer to the provided parent layer, if none is provided we move to scene root
        instead
        '''
        ...

    @overload
    def remove_layer(self: LayeredFile_32bit, layer: Layer_32bit) -> None:
        ''' 
        Remove the specified layer from root of the layered_file, if you instead wish to remove 
        from a group call remove_layer on a GroupLayer_32bit instance instead
        '''
        ...

    @overload
    def remove_layer(self: LayeredFile_32bit, layer: str) -> None:
        ''' 
        Remove the specified layer from root of the layered_file, if you instead wish to remove 
        from a group call remove_layer on a GroupLayer_32bit instance instead
        '''
        ...

    def is_layer_in_document(self: LayeredFile_32bit, layer: Layer_32bit) -> bool:
        '''
        Check if the layer already exists in the LayeredFile at any level of nesting,
        this check is done internally on add_layer().
        '''
        ...

    @staticmethod
    def read(path: os.PathLike) -> LayeredFile_32bit:
        '''
        Read and create a LayeredFile from disk. If the bit depth isnt known ahead of time use LayeredFile.read() instead 
        which will return the appropriate type
        '''
        ...

    def write(self: LayeredFile_32bit, path: os.PathLike, force_overwrite: bool = ...) -> None:
        '''
        Write the LayeredFile_32bit instance to disk invalidating the data, after this point trying to use the 
        instance is undefined behaviour. 

        :param path: 
            The path of the output file, must have a .psd or .psb extension. Conversion between these two types 
            is taken care of internally
        :type path: 
            os.PathLike
        
        :param force_overwrite: 
            Defaults to True, whether to forcefully overwrite the file if it exists. if False the write-op fails
            and emits an error message
        :type force_overwrite: bool
        '''
        ...

    def __getitem__(self: LayeredFile_32bit, name: str) -> Layer_32bit:
        '''
        Get the specified layer from the root of the layered file. Unlike :func:`find_layer` this does not 
        accept a path but rather a single layer located in the root layer. This is to make chaining of paths
        more pythonic since group layers also implement a __getitem__ function

        .. code-block:: python

            layered_file: LayeredFile_32bit = # Our layered file instance
            nested_img_layer = layered_file["Group"]["Image"]

        :param name: The name of the layer to search for
        :type name: str

        :raises:
            KeyError: If the requested layer is not found

        :return: The requested layer instance
        '''
        ...
