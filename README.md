# PhotoshopAPI

A C++ Library for reading and writing of Photoshop Files (*.psd and *.psb) based on previous works from [psd_sdk](https://github.com/MolecularMatters/psd_sdk), [pytoshop](https://github.com/mdboom/pytoshop) and [psd-tools](https://github.com/psd-tools/psd-tools/). As well as the official [Photoshop File Format Specification](https://web.archive.org/web/20231122064257/https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/), where applicable.

This project aims to create a more unified, modern C++ approach to parsing Photoshop files with thorough testing of all the core aspects to ensure verifiability and correctness.

> [!IMPORTANT]
> This repository is still under heavy development and is not meant to be used for production code yet. The expected date for an early release is January 2024. Contributions at this time are welcome, but expect the project to change.

## Structure

The **PhotoshopAPI** is divided into two main projects *PhotoshopAPI* and *PhotoshopTest*

### PhotoshopAPI

This represents the core library for reading, writing and parsing Photoshop files. It consists of the following 3 subfolders (under src/)

- LayeredFile/
    - An abstraction over a photoshop file which is the intended way to use this library as this represents a layer hierarchy in a more simple way abstracting away implementation details of the underlying structure
- PhotoshopFile/
    - An in-memory representation of a Photoshop file. This section makes no assumptions about what the data represents. It does however heavily check if the data is correct (according to the specification) for further parsing.
- Util/
    - Utility functionality pertaining to both LayeredFile/ and PhotoshopFile/


### PhotoshopTest

Unit and Integration testing of the PhotoshopAPI that builds to an executable, primarily of the PhotoshopFile. This contains mostly tests performed on full files where we parse many different types of Photoshop files. For further information, please refer to [this](./PhotoshopTest/documents/README.md) document.

## Usage

> [!WARNING]
> This section will likely heavily change in the near future

The intended usage of the PhotoshopAPI is through the LayeredFile struct. See a minimal reproducible example below.

For more detailed documentation on design decisions as well as general architecture and examples of how to use please refer to the [docs/](/docs/DOCS.md) directory of this repository

### Reading files
```cpp
#include "PhotoshopAPI.h"

int main()
{
    // Initialize a File object for our path
    std::filesystem::path myFile = "C:/Some/Path/To/Document.psd";
    NAMESPACE_PSAPI::File file(myFile);

    // Initialize our document and read the File object
    std::unique_ptr<NAMESPACE_PSAPI::PhotoshopFile> document;
    bool didParse = document->read(file);
    
    // Check if the file parsed correctly and only proceed if thats the case
    if (!didParse)
    {
        return 0;
    }

    // Create a LayeredFile from our PhotoshopFile.
    // Note that we move the document, the constructor of LayeredFile expects a unique_ptr
    // and will extract the relevant data after which it destroys the PhotoshopFile
    NAMESPACE_PSAPI::LayeredFile layeredFile(std::move(document));

    // Use our extracted Image Data
}
```

### Writing files

TODO