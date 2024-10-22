import sys


from . import Layer_8bit, Layer_16bit, Layer_32bit
from . import ImageLayer_8bit, ImageLayer_16bit, ImageLayer_32bit
from . import GroupLayer_8bit, GroupLayer_16bit, GroupLayer_32bit
from . import LayeredFile_8bit, LayeredFile_16bit, LayeredFile_32bit


# Convenience type abbreviations for type hints in functions expecting or returning psapi.Layer types
# Example:
#
#   def update_image_layer(layer: psapi.types.ImageLayer):
#       ...
#
if sys.version_info >= (3, 10):
    # Use | syntax for Python 3.10+
    Layer       = Layer_8bit | Layer_16bit | Layer_32bit
    ImageLayer  = ImageLayer_8bit | ImageLayer_16bit | ImageLayer_32bit
    GroupLayer  = GroupLayer_8bit | GroupLayer_16bit | GroupLayer_32bit
    
    LayeredFile = LayeredFile_8bit | LayeredFile_16bit | LayeredFile_32bit
else:
    # Fallback to Union syntax for older Python versions
    from typing import Union
    Layer       = Union[Layer_8bit, Layer_16bit, Layer_32bit]
    ImageLayer  = Union[ImageLayer_8bit, ImageLayer_16bit, ImageLayer_32bit]
    GroupLayer  = Union[GroupLayer_8bit, GroupLayer_16bit, GroupLayer_32bit]

    LayeredFile = Union[LayeredFile_8bit, LayeredFile_16bit, LayeredFile_32bit]
