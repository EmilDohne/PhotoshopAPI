# Pybind11 doesnt actually inherit from enum.Enum as seen here https://github.com/pybind/pybind11/issues/2332
class LayerColor:
    none: int
    red: int
    orange: int
    yellow: int
    green: int
    blue: int
    violet: int
    gray: int
    seafoam: int
    indigo: int
    magenta: int
    fuschia: int