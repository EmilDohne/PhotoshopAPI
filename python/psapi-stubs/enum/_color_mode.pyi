# Pybind11 doesnt actually inherit from enum.Enum as seend here https://github.com/pybind/pybind11/issues/2332
class ColorMode:
    rgb: int
    cmyk: int
    grayscale: int