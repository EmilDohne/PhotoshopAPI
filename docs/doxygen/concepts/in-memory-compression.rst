.. _in-memory-compression:

Concept: `In Memory Compression`
====================================

While Photoshop stores its data using the compression codecs listed in :ref:`compression`, it is impractical for us to store the Image Data in the same compressed fashion since
the algorithms used by Photoshop are not nearly fast enough to compress and decompress on the fly. 

Storing our images uncompressed is however also unpractical as it requires a huge memory overhead which is especially unfortunate if we wont use most of the parsed channels.
To circumvent this, PhotoshopAPI uses the excellent `c-blosc2 <https://github.com/Blosc/c-blosc2>`_ library to compress and decompress on the fly as we need it. 

Somewhat counterintuitively, this approach not only reduces the memory footprint significantly, it also speeds up the passing of data as it is faster than a straight ``memcpy()``.

To illustrate this point with an example (You can find more in the benchmarks section), a 16-bit Photoshop PSB file which took up ~6GB of data on disk with 170 Image Layers 
each of size 9124x6082 took up 82GB of system memory on decompression. After enabling in-memory compression the same data took up only 6GB of memory while simultaneously
speeding up overall parsing of the whole file by 2x. 

This, in addition to the fast parsing speeds makes it more than feasible to read the file as whole and worry about which data we actually want later on. An important concept when working with the
PhotoshopAPI is passing of data to the :cpp:class:`LayeredFile` as quickly as possible since it internally will handle this compression. So rather than creating the struct only at the end and 
passing all the data in as a whole, it is advised to intermittently add layers to it.

The internal structure (which is used by both the :cpp:class:`LayeredFile` and the :cpp:class:`PhotoshopFile`) responsible for compressing and decompressing data in-memory is the 
:cpp:class:`ImageChannel`