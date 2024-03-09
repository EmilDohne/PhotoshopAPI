# PhotoshopAPI

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/Q5Q4TYALW)


[![CPP Version](https://img.shields.io/badge/language-C%2B%2B20-blue.svg)](https://isocpp.org/)
[![PyPI - Version](https://img.shields.io/pypi/v/PhotoshopAPI?label=pip&color=blue)](https://pypi.org/project/PhotoshopAPI/)
[![PyPi - Downloads](https://img.shields.io/pypi/dm/PhotoshopAPI?label=pip%20downloads&color=4dc81f)](https://pypi.org/project/PhotoshopAPI/)
[![Documentation Status](https://readthedocs.org/projects/photoshopapi/badge/?version=latest)](https://photoshopapi.readthedocs.io/en/latest/?badge=latest)
[![CI Status](https://github.com/EmilDohne/PhotoshopAPI/actions/workflows/cmake-build.yml/badge.svg)](https://github.com/EmilDohne/PhotoshopAPI/actions/workflows/cmake-build.yml)
[![Test Status](https://github.com/EmilDohne/PhotoshopAPI/actions/workflows/cmake-test.yml/badge.svg)](https://github.com/EmilDohne/PhotoshopAPI/actions/workflows/cmake-test.yml)
[![Python Wheels](https://github.com/EmilDohne/PhotoshopAPI/actions/workflows/build-wheels.yml/badge.svg)](https://github.com/EmilDohne/PhotoshopAPI/actions/workflows/build-wheels.yml)




> [!NOTE]
> The PhotoshopAPI is still in early development status which means it is subject to change and will likely include bugs. If you find any please report them to the issues page

About
=========

**PhotoshopAPI** is a C++20 Library with Python bindings for reading and writing of Photoshop Files (*.psd and *.psb) based on previous works from [psd_sdk](https://github.com/MolecularMatters/psd_sdk>),
[pytoshop](https://github.com/mdboom/pytoshop) and [psd-tools](https://github.com/psd-tools/psd-tools>). As well as the official 
[Photoshop File Format Specification](https://web.archive.org/web/20231122064257/https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/), where applicable.
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
is required. It is roughly 5-10x faster in reads and 20x faster in writes than photoshop while producing files that are consistently 20-50% lower in size (see the benchmarks section on readthedocs for details).
The cost of parsing is paid up front either on read or on write so modifying the layer structure itself is almost instantaneous (except for adding new layers).


Features
=========

Supported:
- Read and write of \*.psd and \*.psb files
- Creating and modifying simple and complex nested layer structures
- Pixel Masks
- Modifying layer attributes (name, blend mode etc.)
- Setting the Display ICC Profile
- Setting the DPI of the document
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


Python
==============

The PhotoshopAPI comes with fully fledged Python bindings which can be simply installed using
```
$ py -m pip install PhotoshopAPI
```

alternatively the wheels can be downloaded from the Releases page. For examples on how to use the python bindings please refer to the Python Bindings section on [Readthedocs](https://photoshopapi.readthedocs.io/en/latest/index.html) or check out the PhotoshopExamples/ directory on the github page which includes examples for Python as well as C++.

For an even quicker way of getting started check out the [Quickstart](#quickstart) section!

Documentation
===============

The full documentation with benchmarks, build instructions and code reference is hosted on the [PhotoshopAPI readthedocs page](https://photoshopapi.readthedocs.io/).


Requirements
=============

This goes over requirements for usage, for development requirements please visit the [docs](https://photoshopapi.readthedocs.io/).

- A CPU with AVX2 support (this is most CPUs after 2014) will greatly increase performance, if we detect this to not be there we disable this optimization
- A 64-bit system
- C++ Library: **Linux**, **Windows** or **MacOS**
- Python Library<sup>1</sup>: **Linux**, **Windows**, **MacOS**

The python bindings support python >=3.7 (except for ARM-based MacOS machines which raise this to >=3.10)

> <sup>1</sup> Currently Linux is supported only as manylinux build and has some features disabled such as timestamps on logging.

Performance
===========

The PhotoshopAPI is built with performance as one of its foremost concerns. Using it should enable you to optimize your pipeline rather than slow it down. It runs fully multithreaded with 
SIMD instructions to leverage all the computing power your computer can afford. 

As the feature set increases this will keep being one of the key requirements.
For detailed benchmarks running on a variety of different configurations please visit the [docs](https://photoshopapi.readthedocs.io/)

Below you can find some of the benchmarks comparing the PhotoshopAPI ('PSAPI') against Photoshop in read/write performance

[![8-bit](https://github.com/EmilDohne/PhotoshopAPI/blob/master/docs/doxygen/images/benchmarks/Ryzen_9_5950x/8-bit_graphs.png)](https://photoshopapi.readthedocs.io/en/latest/benchmarks.html)


<img src="https://github.com/EmilDohne/PhotoshopAPI/blob/master/docs/doxygen/images/benchmarks/Ryzen_9_5950x/16-bit_graphs.png" width="49%"/>
<img src="https://github.com/EmilDohne/PhotoshopAPI/blob/master/docs/doxygen/images/benchmarks/Ryzen_9_5950x/32-bit_graphs.png" width="49%"/>

Quickstart
==========

The primary struct to familiarize yourself with when using the PhotoshopAPI is the LayeredFile as well as all its Layer derivatives (such as ImageLayer and 
GroupLayer), all of these are template structs for each of the available bit depths. 

To get a feel of what is possible with the API as well as how to use it please refer to ``PhotoshopExample/`` directory. To familiarize
yourself with the main concepts, as well as recommended workflows check out the [docs](https://photoshopapi.readthedocs.io/) or the [examples](https://github.com/EmilDohne/PhotoshopAPI/tree/master/PhotoshopExamples).

If more fine grained control over the binary structure is necessary, one can modify the PhotoshopFile which is what is parsed by the API internally.
Do keep in mind that this requires a deep understanding of how the Photoshop File Format works. 

Below is a minimal example to get started with opening a PhotoshopFile, removing some layer, and writing the file back out to disk:

### C++ 

```cpp	
using namespace PhotoshopAPI;

// Initialize an 8-bit layeredFile. This must match the bit depth of the PhotoshopFile.
// To initialize this programmatically please refer to the ExtendedSignature example
LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read("InputFile.psd");

// Do some operation, in this case delete
layeredFile.removeLayer("SomeGroup/SomeNestedLayer");	

// One could write out to .psb instead if wanted and the PhotoshopAPI will take 
// care of any conversion internally
LayeredFile<bpp8_t>::write(std::move(layeredFile), "OutputFile.psd");
```


The same code for reading and writing can also be used to for example `LayeredFile::moveLayer` or `LayeredFile::addLayer` as well as extracting any image data

### Python

```py
import psapi

# Read the layered_file using the LayeredFile helper class, this returns a 
# psapi.LayeredFile_*bit object with the appropriate bit-depth
layered_file = psapi.LayeredFile.read("InputFile.psd")

# Do some operation, in this case delete
layered_file.remove_layer()

# Write back out to disk
layered_file.write("OutFile.psd")
```

We can also do much more advanced things such as taking image data from one file and transferring 
it to another file, this can be across file sizes, psd/psb and even bit-depth!

```py
import psapi
import numpy as np
import os


def main() -> None:
    # Read both our files, they can be open at the same time or we can also read one file,
    # extract the layer and return just that layer if we want to save on RAM.
    file_src = psapi.LayeredFile.read("GraftSource_16.psb")
    file_dest = psapi.LayeredFile.read("GraftDestination_8.psd")

    # Extract the image data and convert to 8-bit.
    lr_src: psapi.ImageLayer_16bit = file_src["GraftSource"]
    img_data_src = lr_src.get_image_data()
    img_data_8bit = {}
    for key, value in img_data_src.items():
        value = value / 256 # Convert from 0-65535 -> 0-255
        img_data_8bit[key] = value.astype(np.uint8)

    # Reconstruct an 8bit converted layer
    img_layer_8bit = psapi.ImageLayer_8bit(
        img_data_8bit, 
        layer_name=lr_src.name, 
        width=lr_src.width, 
        height=lr_src.height, 
        blend_mode=lr_src.blend_mode, 
        opacity=lr_src.opacity
        )

    # add the layer and write out to file!
    file_dest.add_layer(img_layer_8bit)
    file_dest.write("GraftDestination_8_Edited.psd")


if __name__ == "__main__":
    main()
```
