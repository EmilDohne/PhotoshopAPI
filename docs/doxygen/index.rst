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
	- Smart Objects (replacing, warping, extracting)
	- Pixel Masks
	- Modifying layer attributes (name, blend mode etc.)
	- Setting the Display ICC Profile
	- 8-, 16- and 32-bit files
	- RGB, CMYK and Grayscale color modes
	- All compression modes known to Photoshop

Planned:
	- Support for Adjustment Layers
	- Support for Vector Masks
	- Support for Text Layers
	- Indexed, Duotone Color Modes

Not Supported:
	- Files written by the PhotoshopAPI do not contain a valid merged image in order to save size meaning they will not behave properly when opened in
	  third party apps requiring these (such as Lightroom)
	- Lab and Multichannel Color Modes 

If something is missing from this list or you would like it added, please open an issue on the github page for it as most things are already parsed by the :cpp:struct:`PhotoshopFile` and
may only need forwarding to the :cpp:struct:`LayeredFile`


Requirements
=============

This goes over requirements for usage, for development requirements please visit the :ref:`building` section.

- A CPU with AVX2 support (this is most CPUs after 2014) will greatly increase performance, if we detect this to not be there we disable this optimization
- C++ Library: **Linux**, **Windows** or **MacOS** 
- Python Library :sup:`1`: **Linux**, **Windows**, **MacOS**

The python bindings support python >=3.7 (except for ARM-based MacOS machines which raise this to >=3.10)


.. note:: 
	
	:sup:`1` Currently Linux is supported only as manylinux build and has some features disabled such as timestamps on logging.

Performance
===========

The PhotoshopAPI is built with performance as one of its foremost concerns. Using it should enable you to optimize your pipeline rather than slow it down. It runs fully multithreaded with 
SIMD instructions to leverage all the computing power your computer can afford. 

As the feature set increases this will keep being one of the key requirements.
For detailed benchmarks running on a variety of different configurations please visit the :ref:`benchmarks` section.


Python Wrapper
==============

The PhotoshopAPI comes with Python bindings which can be installed using

.. code-block:: none

	$ py -m pip install PhotoshopAPI

alternatively the wheels can be downloaded from the Releases page. For examples on how to use the python bindings please refer to :ref:`examples`
or :ref:`bindings`

Quickstart
==========

The primary struct to familiarize yourself with when using the PhotoshopAPI is the :cpp:struct:`LayeredFile` as well as all its Layer derivatives (such as :cpp:struct:`ImageLayer` and 
:cpp:struct:`GroupLayer`), all of these are template structs for each of the available bit depths. 

To get a feel of what is possible with the API as well as how to use it please refer to :ref:`examples`. To familiarize
yourself with the main concepts, as well as recommended workflows check out :ref:`concepts`

If more fine grained control over the binary structure is necessary, one can modify the :cpp:struct:`PhotoshopFile` which is what is parsed by the API internally.
Do keep in mind that this requires a deep understanding of how the Photoshop File Format works. An example of this can be found under :ref:`extended_signature`

Below is a minimal example to get started with opening a PhotoshopFile, removing some layer, and writing the file back out to disk:

.. tab:: C++

	.. literalinclude:: ../../PhotoshopExamples/CreateSimpleDocument/main.cpp
	   :language: cpp

.. tab:: Python

	.. literalinclude:: ../../PhotoshopExamples/CreateSimpleDocument/create_simple_document.py
	   :language: python

The same code for reading and writing can also be used to for example :cpp:func:`LayeredFile::move_layer` or :cpp:func:`LayeredFile::add_layer`.

Contents
========

.. toctree::
   :maxdepth: 2

   code/codestructures.rst
   python/bindings.rst
   concepts/index.rst
   examples/index.rst
   benchmarks.rst
   building.rst
  

:ref:`genindex`

License
=======

.. code-block:: none
	
	BSD 3-Clause License

	Copyright (c) 2025, Emil Dohne

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

