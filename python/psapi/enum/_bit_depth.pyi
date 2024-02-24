# Pybind11 doesnt actually inherit from enum.Enum as seend here https://github.com/pybind/pybind11/issues/2332
class BitDepth:
	'''
    Enum representing the bit depth of an image.

    Attributes
    -------------

    bd_8 : int
        8-bits per channel, equivalent to numpy.uint8
    bd_16 : int
        16-bits per channel, equivalent to numpy.uint16
    bd_32 : int
        32-bits per channel, equivalent to numpy.float32
    '''
	bd_8: int
	bd_16: int
	bd_32: int