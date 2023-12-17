# File Header

> [!NOTE]
> The size is always expressed in bytes unless specifically stated otherwise while the offset is from the start of the section

## Structure

| Offset| Size | Type | Variable | Description |
|---|---|---|---|---|
| **0** | **4** | char[] | Signature | Is always '8BPS' |
| **4** | **2** | uint16_t| Version| If the document is PSD this is `1`, otherwise it is `2` |
| **6** | **6** | void| Padding| All these bytes must be 0, otherwise it is invalid|
| **12** | **2** | uint16_t | NumChannels | Total number of channels in the document, valid range is 1 to 56 |
| **14** | **4** | uint32_t | Height | Height of the file itself, ranges from 1 to 30,000(**PSD**) / 300,000(**PSB**) 
| **18** | **4** | uint32_t | Width | Width of the file itself, ranges from 1 to 30,000(**PSD**) / 300,000(**PSB**) 
| **22** | **2** | uint16_t | BitDepth | Either 1-bit (uint8_t with bitmasks), 8-bit (uint8_t), 16-bit (uint16_t), 32-bit ([IEEE 754 Single Float](https://en.wikipedia.org/wiki/Single-precision_floating-point_format)) |
| **24** | **2** | uint16_t | ColorMode | For a mapping of values see [ColorModeMapping](#color-mode-mapping)




## Color Mode Mapping

| Value | Type |
|---|---|
| **0** | [Bitmap](#bitmap) |
| **1** | [Grayscale](#grayscale) |
| **2** | [Indexed](#indexed) |
| **3** | [RGB](#rgb) |
| **4** | [CMYK](#cmyk) |
| **7** | [Multichannel](#multichannel) |
| **8** | [Duotone](#duotone) |
| **9** | [Lab](#lab) |

### Color Modes

#### Bitmap
- Bitmap is Photoshops way of describing 1-bit [Dithered](https://en.wikipedia.org/wiki/Dither) images (black or white), therefore 8 pixels are 1 byte. This colour mode only supports a single layer.
#### Grayscale
- Grayscale stores only a single channel describing the value of the pixel, this colour mode works in 8-, 16- and 32-bit mode. Despite it only storing a single colour channel, it also always stores an alpha channel alongside the data.
#### Indexed
- Indexed colour mode stores 256 distinct colour values in a colour lookup table that is stored in the [ColorModeData](./ColorModeData.md) section of the file. We then map the pixel value to one of these colours, producing the final image. This mode only supports 8-bit depth
#### RGB
- RGB is the most common colour mode and is a representation of Colours as three colour components working together additively (a value of R = 1, G = 1, B = 1 will produce a white pixel). R stands for Red, G stands for Green, B stands for Blue. This mode supports all 8-, 16- and 32-bit depths.
#### CMYK
- CMYK is a print media colour mode working subtractively (if all values are 0 we get a white image). The individual components stand for Cyan, Magenta, Yellow and Key (black). This colour mode is only supported for 8- and 16-bit files.
#### Multichannel
- Multichannel is an further specialization for print media and stores each channel in either 8- or 16-bit mode.
#### Duotone
- A way to create the duotone effect based on a greyscale image. This mode is only supported in 8-bit mode and stores some information on how to map the grey values to RGB in the [ColorModeData](./ColorModeData.md) section.
#### Lab
- The [CIELAB](https://en.wikipedia.org/wiki/CIELAB_color_space) color space