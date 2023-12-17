# PhotoshopFileFormat

> [!IMPORTANT]
> This section is still under heavy development and will likely be completed after the initial planned release in Jan 2024

Photoshop Documents are always stored in Big Endian byte order (most significant byte at the lowest memory address).

The Photoshop File Formats (*.psd and *.psb) are composed of 5 different file sections. These are:

- [File Header Section](./FileHeader.md)
    - This contains information on the size, bit depth and type of the document as a whole
- [Color Mode Data Section](./ColorModeData.md)
    - This section contains information for indexed and duotone color modes (the latter being undocumented) as well as some potential HDR toning information for 32-bit files
- [Image Resources Section](./ImageResources.md)
    - Here, some general information on the appearance, color spaces, and more pertaining to the document as a whole are stored. This does not include any global information about the layers themselves. Many of these Resource Blocks are undocumented and/or deprecated.
- [Layer and Mask Information Section](./LayerAndMaskInformation.md)
    - This is where we store layer information such as layer structure and image data
- [Image Data Section](./ImageData.md)
    - Despite what the name might imply, we simply store the Merged Image Data result here, rather than the image data for all the layers. This is also only relevant if the document is saved with "Maximize Compatibility" on, to enable third-party applications to visualize the contents of the photoshop file.