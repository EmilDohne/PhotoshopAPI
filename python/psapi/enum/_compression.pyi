# Pybind11 doesnt actually inherit from enum.Enum as seend here https://github.com/pybind/pybind11/issues/2332
class Compression:
    '''
    Enum representation of all the different Compression codecs supported by Photoshop (and PSAPI).

    Attributes
    -------------

    raw : int
        encode as raw bytes (no compression)
    rle : int
        encode with run-length-encoding for fastest write speeds at the cost of lower compression ratios (especially for 16- and 32-bit)
    zip : int
        encode with zip (deflate) compression, usually the best compression codec choice as well as zipprediction
    zipprediction : int
        encode with zip (deflate) compression but additionally 'prediction' encode the data which takes the difference between the last and 
        the current pixel per scanline and stores that (for 32-bit files it interleaves the bytes).
	'''
    raw: int
    rle: int
    zip: int
    zipprediction: int