# Color Structures

Color Structures are photoshops way of storing colour swatches. These are also relevant for the duotone color mode. Described below are some of the types present and how they are to be interpreted. Keep in mind that photoshop seems to choose which type it writes according to some unknown algorithm meaning that one must consider all cases even if one file shows a consistent usage of one type.

We cover the types specified in the Adobe Specification but there might be some other undocumented one

- [Undefined/Disabled](#undefined--disabled)
- [RGB](#rgb)
- [HSB](#hsb)
- [CMYK](#cmyk)
- [LAB](#lab)
- [Greyscale](#greyscale)

## Undefined / Disabled

**Total Size** : 10 bytes
| Size | Type | Variable | Description |
|---|---|---|---|
| **2** | int16_t | ColorMode | The type of data to follow, for undefined/disabled this is -1 |
| **2** | uint16_t | Paddding | Must be 0 |
| **2** | uint16_t | Paddding | Must be 0 |
| **2** | uint16_t | Paddding | Must be 0 |
| **2** | uint16_t | Paddding | Must be 0 |

## RGB

**Total Size** : 10 bytes
| Size | Type | Variable | Description |
|---|---|---|---|
| **2** | int16_t | ColorMode | The type of data to follow, for RGB this is 0 |
| **2** | uint16_t | R value | The red value of the data |
| **2** | uint16_t | G value | The green value of the data |
| **2** | uint16_t | B value | The blue value of the data |
| **2** | uint16_t | Paddding | Must be 0 |



## HSB

**Total Size** : 10 bytes
| Size | Type | Variable | Description |
|---|---|---|---|
| **2** | int16_t | ColorMode | The type of data to follow, for HSB this is 1 |
| **2** | uint16_t | hue value | The hue value of the data |
| **2** | uint16_t | sat value | The sat value of the data |
| **2** | uint16_t | val value | The val value of the data |
| **2** | uint16_t | Paddding | Must be 0 |


## CMYK

**Total Size** : 10 bytes
| Size | Type | Variable | Description |
|---|---|---|---|
| **2** | int16_t | ColorMode | The type of data to follow, for CMYK this is 2 |
| **2** | uint16_t | C value | The C value of the data. Unintuitively, the range is inverse. I.e. 65535 is 0  |
| **2** | uint16_t | M value | The M value of the data. Unintuitively, the range is inverse. I.e. 65535 is 0  |
| **2** | uint16_t | Y value | The Y value of the data. Unintuitively, the range is inverse. I.e. 65535 is 0  |
| **2** | uint16_t | K value | The K value of the data. Unintuitively, the range is inverse. I.e. 65535 is 0  |

## LAB

**Total Size** : 10 bytes
| Size | Type | Variable | Description |
|---|---|---|---|
| **2** | int16_t | ColorMode | The type of data to follow, for LAB this is 7 |
| **2** | uint16_t | Lightness | The Lightness of the data. this value ranges from 0 - 10,000|
| **2** | int16_t | A Chrominance | Chrominance values are in the range of -12,800 - 12,700 with gray being 0  |
| **2** | int16_t | B Chrominance | Chrominance values are in the range of -12,800 - 12,700 with gray being 0  |
| **2** | uint16_t | Paddding | Must be 0 |

## Greyscale

**Total Size** : 10 bytes
| Size | Type | Variable | Description |
|---|---|---|---|
| **2** | int16_t | ColorMode | The type of data to follow, for Greyscale this is 8 |
| **2** | uint16_t | Grey | The Grey intensity of the value, making use of the whole range |
| **2** | uint16_t | Paddding | Must be 0 |
| **2** | uint16_t | Paddding | Must be 0 |
| **2** | uint16_t | Paddding | Must be 0 |