PhotoshopAPI
########################################

.. raw:: html

	<script type='text/javascript' src='https://storage.ko-fi.com/cdn/widget/Widget_2.js'></script><script type='text/javascript'>kofiwidget2.init('Support Me on Ko-fi', '#29abe0', 'Q5Q4TYALW');kofiwidget2.draw();</script> 

.. note::

	The PhotoshopAPI is still in early development status which means it is subject to change and will likely include bugs. If you find any please report them to the issues page

About
=========

**PhotoshopAPI** is a C++20 Library with Python bindings for reading and writing of Photoshop Files (\*.psd and \*.psb) based on previous works from `psd_sdk <https://github.com/MolecularMatters/psd_sdk>`_,
`pytoshop <https://github.com/mdboom/pytoshop>`_ and `psd-tools <https://github.com/psd-tools/psd-tools>`_. As well as the official 
`Photoshop File Format Specification <https://web.archive.org/web/20231122064257/https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/>`_, where applicable.
The library is continuously tested for correctness in its core functionality. If you do find a bug
please submit an issue to the github page.

The motivation to create another library despite all the other works present is that there isn't a library which has layer editing as a first class citizen while also supporting 
all bit-depths known to Photoshop (``8-bits``, ``16-bits``, ``32-bits``). This Library aims to create an abstraction between the raw binary file format and the structure that the user interfaces
against to provide a more intuitive approach to the editing of Photoshop Files. 

Why should you care?
====================

Photoshop itself is unfortunately often slow to read/write files and the built-in tools for automatically/programmatically modifying files suffer this same issue. On top of this, due to the 
extensive history of the Photoshop File Format, Photoshop files written out by Photoshop itself are often unnecessarily bloated to add backwards compatibility or cross-software compatibility.

The PhotoshopAPI tries to address these issue by allowing the user to read/write/modify Photoshop Files without ever having to enter Photoshop itself which additionally means, no license 
is required. It is roughly 5-10x faster in reads and 20x faster in writes than photoshop while producing files that are consistently 20-50% lower in size (see :ref:`benchmarks` for details).
The cost of parsing is paid up front either on read or on write so modifying the layer structure itself is almost instantaneous (except for adding new layers).


Features
=========

Supported:
	- Read and write of \*.psd and \*.psb files
	- Creating and modifying simple and complex nested layer structures
	- Pixel Masks
	- Modifying layer attributes (name, blend mode etc.)
	- Setting the Display ICC Profile
	- 8-, 16- and 32-bit files
	- RGB Color Mode
	- All compression modes known to Photoshop

Planned:
	- Support for Adjustment Layers
	- Support for Vector Masks
	- Support for Text Layers
	- Support for Smart Object Layers
	- CMYK, Indexed, Duotone and Greyscale Color Modes

Not Supported:
	- Files written by the PhotoshopAPI do not contain a valid merged image in order to save size meaning they will not behave properly when opened in
	  third party apps requiring these (such as Lightroom)
	- Lab and Multichannel Color Modes 

If something is missing from this list or you would like it added, please open an issue on the github page for it as most things are already parsed by the :cpp:struct:`PhotoshopFile` and
may only need forwarding to the :cpp:struct:`LayeredFile`


Requirements
=============

This goes over requirements for usage, for development requirements please visit the :ref:`building` section.

- A CPU with AVX2 support (this is most CPUs after 2014). If you are unsure, please refer to your CPUs specification
- A 64-bit system
- C++ Library: **Linux**, **Windows** or **MacOS** (M-Chips are not tested)
- Python Library :sup:`1`: **Windows**, **MacOS** (M-Chips are not tested)

.. note:: 
	
	:sup:`1` Currently Linux is not supported as the manylinux image for cibuildwheels does not yet support C++20

Performance
===========

The PhotoshopAPI is built with performance as one of its foremost concerns. Using it should enable you to optimize your pipeline rather than slow it down. It runs fully multithreaded with 
SIMD instructions to leverage all the computing power your computer can afford. 

As the feature set increases this will keep being one of the key requirements.
For detailed benchmarks running on a variety of different configurations please visit the :ref:`benchmarks` section.


Python Wrapper
==============

The PhotoshopAPI comes with fully fledged Python bindings which can be simply installed using

.. code-block:: none

	$ py -m pip install PhotoshopAPI

alternatively the wheels can be downloaded from the Releases page. For examples on how to use the python bindings please refer to the Python Bindings section or check out the PhotoshopExamples/ directory on
the github page which includes fully fledged python examples.

Quickstart
==========

The primary struct to familiarize yourself with when using the PhotoshopAPI is the :cpp:struct:`LayeredFile` as well as all its Layer derivatives (such as :cpp:struct:`ImageLayer` and 
:cpp:struct:`GroupLayer`), all of these are template structs for each of the available bit depths. 

To get a feel of what is possible with the API as well as how to use it please refer to ``PhotoshopExample/`` directory in the root directory of the github page. To familiarize
yourself with the main concepts, as well as recommended workflows check out :ref:`concepts`

If more fine grained control over the binary structure is necessary, one can modify the :cpp:struct:`PhotoshopFile` which is what is parsed by the API internally.
Do keep in mind that this requires a deep understanding of how the Photoshop File Format works. 

Below is a minimal example to get started with opening a PhotoshopFile, removing some layer, and writing the file back out to disk:

C++
--------

.. code-block:: cpp
	
	using namespace PhotoshopAPI;

	auto inputFile = File("./InputFile.psd");
	auto psDocumentPtr = std::make_unique<PhotoshopFile>();
	psDocumentPtr->read(inputFile);

	// Initialize an 8-bit layeredFile. This must match the bit depth of the PhotoshopFile.
	// To find the bit depth programatically one can use the psDocumentPtr->m_Header.m_Depth
	// variable
	LayeredFile<bpp8_t> layeredFile = { std::move(psDocumentPtr) };
	layeredFile.removeLayer("SomeGroup/SomeNestedLayer");	// This will also delete any child layers

	// We can now convert back to a PhotoshopFile and write out to disk
	File::FileParams params = { .doRead = false};
	// One could write out to .psb instead if wanted and the PhotoshopAPI will take 
	// care of any conversion internally
	auto outputFile = File("./OutputFile.psd", params);
	auto psOutDocumentPtr = LayeredToPhotoshopFile(std::move(layeredFile));

	psOutDocumentPtr->write(outputFile);


Python
---------

.. code-block:: python

	import psapi

	# Read the layered_file using the LayeredFile helper class, this returns a 
	# psapi.LayeredFile_*bit object with the appropriate bit-depth
	layered_file = psapi.LayeredFile.read("InputFile.psd")

	# Do some operation, in this case delete
	layered_file.remove_layer()

	# Write back out to disk
	layered_file.write("OutFile.psd")

The same code for reading and writing can also be used to for example :cpp:func:`LayeredFile::moveLayer` or :cpp:func:`LayeredFile::addLayer`.

Contents
========

.. toctree::
   :maxdepth: 2

   code/codestructures.rst
   python/bindings.rst
   concepts/index.rst
   benchmarks.rst
   building.rst
  

:ref:`genindex`

License
=======

.. code-block:: none
	
	BSD 3-Clause License

	Copyright (c) 2024, Emil Dohne

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its
	   contributors may be used to endorse or promote products derived from
	   this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
	FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

