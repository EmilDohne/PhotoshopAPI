# Pybind11 doesnt actually inherit from enum.Enum as seen here https://github.com/pybind/pybind11/issues/2332
class ChannelID:
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