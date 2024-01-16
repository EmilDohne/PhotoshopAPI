# Color Mode Data

This file section holds mapping information for indexed and duotone colours as well as toning information (speculative) for 32-bit files

> [!NOTE]
> The size is always expressed in bytes unless specifically stated otherwise while the offset is from the start of the section

- [Duotone Colors](#duotone-colors)
- [Indexed Colors](#indexed-colors)
- [32-bit files](#32-bit-files)

## Duotone colors

### Structure

Offset | Size | Type | Variable | Description |
|---|---|---|---|---|
| **0** | **4** | uint32_t | Section Size | Section Size for duotone colours, appears to always be 524
| **4** | **2** | uint16_t | Version | Appears to always be equals to 1
| **6** | **2** | uint16_t | Mode | 1 = Monotone; 2 = Duotone; 3 = Tritone; 4 = Quadtone 
| **8** | **10** | [ColorStruct](./Structures/ColorStructure.md) | Ink1 Colour | The colour of Ink1, if this is not enabled the mode of the struct is -1
| **18** | **10** | [ColorStruct](./Structures/ColorStructure.md) | Ink2 Colour | The colour of Ink2, if this is not enabled the mode of the struct is -1
| **28** | **10** | [ColorStruct](./Structures/ColorStructure.md) | Ink3 Colour | The colour of Ink3, if this is not enabled the mode of the struct is -1
| **38** | **10** | [ColorStruct](./Structures/ColorStructure.md) | Ink4 Colour | The colour of Ink4, if this is not enabled the mode of the struct is -1
| **48** | **64** | [PascalString](./Structures/PascalString.md) | Ink1 Name | The name of the Ink1 colour, padded to 64 bytes. Always present no matter if it set to Monotone, Duotone, Tritone or Quadtone
| **112** | **64** | [PascalString](./Structures/PascalString.md) | Ink2 Name | The name of the Ink2 colour, padded to 64 bytes. Always present no matter if it set to Monotone, Duotone, Tritone or Quadtone
| **176** | **64** | [PascalString](./Structures/PascalString.md) | Ink3 Name | The name of the Ink3 colour, padded to 64 bytes. Always present no matter if it set to Monotone, Duotone, Tritone or Quadtone
| **240** | **64** | [PascalString](./Structures/PascalString.md) | Ink4 Name | The name of the Ink4 colour, padded to 64 bytes. Always present no matter if it set to Monotone, Duotone, Tritone or Quadtone
| **304** | **28** | [TransferFunction](./Structures/TransferFunction.md) | Ink1 Curve | The values describing the transfer curve of ink1. Always present
| **332** | **28** | [TransferFunction](./Structures/TransferFunction.md) | Ink2 Curve | The values describing the transfer curve of ink2. Always present
| **360** | **28** | [TransferFunction](./Structures/TransferFunction.md) | Ink3 Curve | The values describing the transfer curve of ink3. Always present
| **388** | **28** | [TransferFunction](./Structures/TransferFunction.md) | Ink4 Curve | The values describing the transfer curve of ink4. Always present
| **416** | **2** | uint16t | Dot Gain | Kept for compatibility with Photoshop 2.0 but is ignored (must be 20)|
| **418** | **10** | [ColorStruct](./Structures/ColorStructure.md) | Overprint Colour | 1+2 Overprint Colour |
| **428** | **10** | [ColorStruct](./Structures/ColorStructure.md) | Overprint Colour | 1+3 Overprint Colour |
| **438** | **10** | [ColorStruct](./Structures/ColorStructure.md) | Overprint Colour | 2+3 Overprint Colour |
| **448** | **10** | [ColorStruct](./Structures/ColorStructure.md) | Overprint Colour | 1+2+3 Overprint Colour |
| **458** | **10** | [ColorStruct](./Structures/ColorStructure.md) | Overprint Colour | 1+4 Overprint Colour |
| **468** | **10** | [ColorStruct](./Structures/ColorStructure.md) | Overprint Colour | 2+4 Overprint Colour |
| **478** | **10** | [ColorStruct](./Structures/ColorStructure.md) | Overprint Colour | 3+4 Overprint Colour |
| **488** | **10** | [ColorStruct](./Structures/ColorStructure.md) | Overprint Colour | 1+2+4 Overprint Colour |
| **498** | **10** | [ColorStruct](./Structures/ColorStructure.md) | Overprint Colour | 1+3+4 Overprint Colour |
| **508** | **10** | [ColorStruct](./Structures/ColorStructure.md) | Overprint Colour | 2+3+4 Overprint Colour |
| **518** | **10** | [ColorStruct](./Structures/ColorStructure.md) | Overprint Colour | 1+2+3+4 Overprint Colour |


## Indexed colors

### Structure


## 32 bit files

### Structure


32-bit files also have a required but undocumented ColorModeData section as described below:
