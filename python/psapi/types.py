import sys

import psapi


# Convenience type abbreviations for type hints in functions expecting or returning psapi.Layer types
# Example:
#
#   def update_image_layer(layer: psapi.types.ImageLayer):
#       ...
#
if sys.version_info >= (3, 10):
    # Use | syntax for Python 3.10+
    Layer       = psapi.Layer_8bit | psapi.Layer_16bit | psapi.Layer_32bit
    ImageLayer  = psapi.ImageLayer_8bit | psapi.ImageLayer_16bit | psapi.ImageLayer_32bit
    GroupLayer  = psapi.GroupLayer_8bit | psapi.GroupLayer_16bit | psapi.GroupLayer_32bit

    LayeredFile = psapi.LayeredFile_8bit | psapi.LayeredFile_16bit | psapi.LayeredFile_32bit
else:
    # Fallback to Union syntax for older Python versions
    from typing import Union
    Layer       = Union[psapi.Layer_8bit, psapi.Layer_16bit, psapi.Layer_32bit]
    ImageLayer  = Union[psapi.ImageLayer_8bit, psapi.ImageLayer_16bit, psapi.ImageLayer_32bit]
    GroupLayer  = Union[psapi.GroupLayer_8bit, psapi.GroupLayer_16bit, psapi.GroupLayer_32bit]

    LayeredFile = Union[psapi.LayeredFile_8bit, psapi.LayeredFile_16bit, psapi.LayeredFile_32bit]
