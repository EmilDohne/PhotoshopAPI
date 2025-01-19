# Pybind11 doesnt actually inherit from enum.Enum as seen here https://github.com/pybind/pybind11/issues/2332
class LinkedLayerType:
    data: int
    external: int