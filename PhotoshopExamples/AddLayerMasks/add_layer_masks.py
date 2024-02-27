# These are just for running these in the repository but is not necessary when the files are pip installed
import os, sys
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__),"../../bin-int/PhotoshopAPI/x64-release/python")))

# Example of creating a simple document with a single layer and a mask using the PhotoshopAPI.
import psapi
import numpy as np


def main() -> None:
    # Initialize some constants that we will need throughout the program
    width = 64
    height = 64
    color_mode = psapi.enum.ColorMode.rgb

    # Generate our LayeredFile instance
    document = psapi.LayeredFile_8bit(color_mode, width, height)

    img_data = np.zeros((3, height, width), np.uint8)
    img_data[0] = 255
    mask = np.full((height, width), 128, np.uint8)

    # When creating an image layer the width and height parameter are required if its not a zero sized layer
    img_layer = psapi.ImageLayer_8bit(img_data, "Layer Red", layer_mask=mask, width=width, height=height)

    # Add the layer and write to disk
    document.add_layer(img_layer)
    document.write(os.path.join(os.path.dirname(__file__), "WriteLayerMasks.psd"))


if __name__ == "__main__":
    main()