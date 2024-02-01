.. _compression:

Concept: `Compression Codecs`
====================================

Photoshop has four different modes in which it can store its image data which all have their individual benefits/drawbacks. These are ``Raw``, ``Rle``, ``Zip`` and ``ZipPrediction``.


**PhotoshopAPI** defaults to :ref:`ZipPrediction` compression as it usually reaches the best compression ratios and is faster than both :ref:`Raw` and :ref:`Rle` while only being
marginally (~5%) slower than :ref:`Zip`.

.. _raw:

Raw
----

Raw simply stores the bytes as-is in Big Endian order on disk. While one might assume that this is the fastest way to write a PhotoshopFile it actually gets
outperformed by almost all compression codes even on an NVME SDD as the output drive (for more information visit the benchmark section). Therefore it is usually
not recommended to choose this mode of storing data.

.. _rle:

Rle
----

Run-Length-Encoding using the PackBits algorithm compresses each scanline of data separately and essentially always starts with a ``count`` byte that gives us information on whether we are storing 
a run or non-run. Due to this operating on individual bytes the maximum length of each run and non-run are 128 where run bytes are stored as a single value and non-run bytes are just the sequence
of bytes it was unable to compress. For more information on this algorithm please visit the `Wikipedia <https://en.wikipedia.org/wiki/PackBits>`_ page as it holds some examples as well.

Rle is Photoshop's preferred way of encoding ``8-bit`` data as it works best if each byte represents a single pixel. It is valid to compress ``16-bit`` and ``32-bit`` data using this compression scheme
as well but is not usually advised as it would usually lead to fairly bad compression ratios. Despite this, Rle is actually the compression codec of choice for writing out the ImageData section
from the **PhotoshopAPI** as it is the only codec besides Raw supported by Photoshop for that section and by having the bytes be explicit 0s we can reduce file size significantly.


.. _zip:

Zip
----

The Zip Compression Codec is using the ``Inflate`` and ``Deflate`` compression algorithms as seen in the ZIP file format and in most cases offers much greater compression ratios than Run-length encoding
and is also much faster to encode (due to using a much more optimized implementation of the algorithm than our naive PackBits implemention). Internally `zlib-ng <https://github.com/zlib-ng/zlib-ng>`_ 
is used as our way of encoding and decoding zip data. Photoshop itself (tested on Photoshop CC 23.3.2) actually never explicitly writes out this compression coded in favour of :ref:`ZipPrediction`.
In fact, when writing 32-bit files Photoshop will not allow Zip Compressed data and interprets it as ZipPrediction regardless of what compression codec is specified. Due to this the **PhotoshopAPI**
will issue a warning when compressing 32-bit files with Zip and internally force a conversion to ZipPrediction to still write valid files.


.. _zipprediction:

ZipPrediction
--------------

Zip Prediction is an interesting compression codec as it has different ways of encoding the data depending on the context it is called from. While for ``8-bit`` and ``16-bit`` documents
it simply calculated the differences between each item in a scanline, for ``32-bit`` documents it deinterleaves the bytes of each scanline presumably for better compression ratios. This
compression codec is the compression codec of choice for Photoshop if the document is ``16-bit`` or ``32-bit``. For the non-32-bit layers the ZipPrediction codec calculates the difference
between items as illustrated below:


	123456789 // This is our starting numbers

	111111111 // This is what the prediction encoding would look like

After which it would simply use the same ``Inflate`` and ``Deflate`` compression algorithms outlined in :ref:`zip`

For ``32-bit`` files however the byte order is first deinterleaved, after which the data is prediction encoded and zipped as shown above. This means if we imagine an array of 4-byte numbers we group all the 
first, second, third and fourth bytes together into sections of the row.

	1234 1234 1234 1234 ...		// This is our starting bytes

	1111 2222 3333 4444 ...		// This is the result of our deinterleaving

