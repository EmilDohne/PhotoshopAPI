# Example of creating a simple document with a single layer and a mask using the PhotoshopAPI.
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
    mask = np.full((height, width), 128, np.uint8)

    img_layer = psapi.ImageLayer_8bit(img_data, "Layer Red", layer_mask=mask, width=width, height=height)

    # Add the layer and write to disk
    document.add_layer(img_layer)
    document.write(os.path.join(os.path.dirname(__file__), "WriteLayerMasks.psd"))


if __name__ == "__main__":
    main()