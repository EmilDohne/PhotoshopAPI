# Example of creating a document with groups using the PhotoshopAPI.
import os
import numpy as np
import psapi


def main() -> None:
    # Initialize some constants that we will need throughout the program
    width = 64
    height = 64
    color_mode = psapi.enum.ColorMode.rgb

    # Generate our LayeredFile instance
    document = psapi.LayeredFile_8bit(color_mode, width, height)

    # Create an empty group layer, width and height are only relevant if we want to add a mask channel
    group_layer = psapi.GroupLayer_8bit("Group")
    document.add_layer(group_layer)

    img_data = np.zeros((3, height, width), np.uint8)
    img_data[0] = 255

    # When creating an image layer the width and height parameter are required if its not a zero sized layer
    img_layer = psapi.ImageLayer_8bit(img_data, "Layer Red", width=width, height=height)
    # In this example we add the image layer under the group instead which requires us to pass the document as first
    # argument. This is required for runtime checks that a layer isnt added twice
    group_layer.add_layer(document, img_layer)

    document.write(os.path.join(os.path.dirname(__file__), "WriteGroupedFile.psd"))


if __name__ == "__main__":
    main()