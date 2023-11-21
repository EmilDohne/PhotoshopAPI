# Compression test cases

These files provide test cases and insight into how Photoshop handles compression at different bit-depths and for different image files as well. 

## 8-bit Files
It appears from testing that **8-bit** files will check if the compressed size of RLE will be smaller than RAW data and if thats not the case, keep the data as RAW encoded. This check, from tests seems to be happening at any given file size (was tested up to 16384 x 16384 pixels). This also holds true for the merged image data section (present when maximize compatibility is turned on).

## 16- and 32-bit Files

For **16-** and **32-bit** files however, it seems that they get saved with prediction encoded zip compression always, no matter the data. Even if the compressed size is larger than the uncompressed data. 

The merged image data section on the other hand appears to always be uncompressed which is unfortunate as Zip Prediction often outperforms RLE, even for bigger datasets. Yet, due to the uncompressed image data section these files are often much bigger.

> [!NOTE]
> This "bug" may very well be because older versions of Photoshop do not support the Zip with Prediction compression mode. It is usually best to disable Maximize compatibility to minimize file sizes