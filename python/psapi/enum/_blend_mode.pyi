# Pybind11 doesnt actually inherit from enum.Enum as seend here https://github.com/pybind/pybind11/issues/2332
class BlendMode:
    '''
    Enum representation of all the different blendmodes found in a file.

    Attributes
    -----------

    passthrough: int
        Reserved for Group layers only
    normal: int

    dissolve: int

    darken: int

    multiply: int

    colorburn: int

    linearburn: int

    darkercolor: int

    lighten: int

    screen: int

    colordodge: int

    lineardodge: int

    lightercolor: int

    overlay: int

    softlight: int

    hardlight: int

    vividlight: int

    linearlight: int

    pinlight: int

    hardmix: int

    difference: int

    exclusion: int

    subtract: int

    divide: int

    hue: int

    saturation: int

    color: int

    luminosity: int
    '''
    passthrough: int
    normal: int
    dissolve: int
    darken: int
    multiply: int
    colorburn: int
    linearburn: int
    darkercolor: int
    lighten: int
    screen: int
    colordodge: int
    lineardodge: int
    lightercolor: int
    overlay: int
    softlight: int
    hardlight: int
    vividlight: int
    linearlight: int
    pinlight: int
    hardmix: int
    difference: int
    exclusion: int
    subtract: int
    divide: int
    hue: int
    saturation: int
    color: int
    luminosity: int