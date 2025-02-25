# Pybind11 doesnt actually inherit from enum.Enum as seen here https://github.com/pybind/pybind11/issues/2332
class BitDepth:
	bd_8: int
	bd_16: int
	bd_32: int