# Example of creating a simple document with a single layer using the PhotoshopAPI.
import os
import numpy as np
import photoshopapi as psapi

def main() -> None:
    # Initialize some constants that we will need throughout the program
    width = 64
    height = 64
    color_mode = psapi.enum.ColorMode.rgb

    # Generate our LayeredFile instance
    document = psapi.LayeredFile_8bit(color_mode, width, height)

    img_data = np.zeros((3, height, width), np.uint8)
    img_data[0] = 255
    # When creating an image layer the width and height parameter are required if its not a zero sized layer
    img_layer = psapi.ImageLayer_8bit(img_data, "Layer Red", width=width, height=height)
    document.add_layer(img_layer)

    # Similar to the C++ version we can adjust parameters of the layer after it has been added to the document
    # as long as it happens before we write to disk
    img_layer.opacity = .5

    document.write(os.path.join(os.path.dirname(__file__), "WriteSimpleFile.psd"))


if __name__ == "__main__":
    main()