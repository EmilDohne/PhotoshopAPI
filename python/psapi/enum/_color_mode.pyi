# Pybind11 doesnt actually inherit from enum.Enum as seend here https://github.com/pybind/pybind11/issues/2332
class ColorMode:
    '''
    Enum representing the color mode of an file.

    Attributes
    -------------

    rgb : int
        rgb color mode (supports channels R, G, B and A)
    cmyk : int
        cmyk color mode (supports channels C, M, Y, K and A)
    grayscale : int
        grayscale color mode (supports channels Gray, A)
    '''
    rgb: int
    cmyk: int
    grayscale: int