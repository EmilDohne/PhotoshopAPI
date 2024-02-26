import os

import psapi.util
import psapi.enum

class PhotoshopFile:
    '''
    This class represents the low-level File Structure of the Photoshop document itself.
    In the python bindings we explicitly do not expose all of its sub-classes as the implementation
    details are currently not meant to be accessed
    '''

    def __init__(self: PhotoshopFile) -> None:
        ...

    def read(self: PhotoshopFile, document: psapi.util.File) -> None:
        '''
        Read the PhotoshopFile class from a File instance, this file must be a valid .psd or .psb file.

        :param document: The file object used for reading
        :type document: :class:`psapi.util.File`

        :rtype: None
        '''
        ...

    def write(self: PhotoshopFile, document: psapi.util.File) -> None:
        '''
        Write the PhotoshopFile class to disk using a instance, this file must be a valid .psd or .psb file.

        :param document: The file object used for writing
        :type document: :class:`psapi.util.File`

        :rtype: None
        '''
        ...
    
    @staticmethod
    def find_bitdepth(path: os.PathLike) -> psapi.enum.BitDepth:
        '''
        Find the bit depth of a Photoshop file from the given filepath.
        This function has basically no runtime cost as it simply reads the first 26 bytes of the document
        and uses that to extract the bit depth. The intention of this function is to provide an interface
        to quickly check which psapi.LayeredFile instance to construct. For example

        .. code-block:: python

            depth = psapi.PhotoshopFile.find_bitdepth("SomeFile.psb")
            if (depth == psapi.enum.BitDepth.bd_8):
                layered_file = psapi.LayeredFile_8bit.read("SomeFile.psb")
            # etc...

        :param filepath: The path to the Photoshop file.
        :type filepath: str

        :return: The bit depth of the Photoshop file as an Enum::BitDepth.
        :rtype: :class:`psapi.enum.BitDepth`
        '''
        ...
