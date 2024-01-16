# Pascal String

Pascal strings are Photoshops way of describing char[] which have a pre-fixed size and are often padded to a specific byte count. 

**Total Size** : 1-255 bytes
| Size | Type | Variable | Description |
|---|---|---|---|
| **1** | uint8_t | Size | The size of the data to follow, note that this does not include any padding information! This must be deduced from the documentation
| **Size** | char[] | String | The string encoded as chars.
| **Variable** | void | Padding | Potential padding bytes, padding applies to the size of the string + the uint8_t size marker. E.g. a 62 long string (with size set to 62) and a padding of 2-bytes will end up having a total memory length of 64 as the combined size is 63-bytes not 62.