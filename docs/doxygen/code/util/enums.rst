Enumerators
============


Logging Enums
--------------

.. doxygenenum:: Enum::Severity


Photoshop File Header Enums
---------------------------

While Photoshop calls this "Version", this simply describes what data we are writing out and
internally changes some offsets and sizes accordingly.

.. doxygenenum:: Enum::Version

All the Bit depths of Photoshop, when using the API you will rarely need to interact with these
unless modifying the PhotoshopFile struct directly. Usually you will use template arguments
with :ref:`bitdepth` template arguments

.. doxygenenum:: Enum::BitDepth

The color modes Photoshop is able to read, currently only RGB is fully supported

.. doxygenenum:: Enum::ColorMode


Layer Enums
-----------

Channel IDs for channels of an individual layer. These, or logical indices, can be used to uniquely identify a layer. For more
information visit :cpp:struct:`ImageLayer`

|

.. doxygenenum:: Enum::ChannelID

|

The Blend Modes we are able to write out using the PhotoshopAPI, these match the ones present in Photoshop

|

.. doxygenenum:: Enum::BlendMode

|

A listing of all the available compression types in the PhotoshopAPI.
For more information on how these compression codecs work internally please visit :ref:`compression`

.. doxygenenum:: Enum::Compression