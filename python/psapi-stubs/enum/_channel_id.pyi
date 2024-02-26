# Pybind11 doesnt actually inherit from enum.Enum as seend here https://github.com/pybind/pybind11/issues/2332
class ChannelID:
    '''
    Enum representation of all the different channel ids found in a file.

    Attributes
    -----------

    red: int

    green: int

    blue: int

    cyan: int

    magenta: int

    yellow: int

    black: int

    gray: int

    custom: int

    mask: int

    alpha: int
    '''
    red: int
    green: int
    blue: int
    cyan: int
    magenta: int
    yellow: int
    black: int
    gray: int
    custom: int
    mask: int
    alpha: int