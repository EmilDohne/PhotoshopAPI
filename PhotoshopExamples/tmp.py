import numpy as np
import psapi


width = 64
height = 64
color_mode = psapi.enum.ColorMode.rgb
print(color_mode)

# Generate our LayeredFile instance
document = psapi.LayeredFile_8bit(color_mode, width, height)

img_data = np.zeros((3, height, width), np.uint8)
img_data[0] = 255
mask = np.full((height, width), 128, np.uint8)

# # When creating an image layer the width and height parameter are required if its not a zero sized layer
img_layer = psapi.ImageLayer_8bit(img_data, "Layer Red", layer_mask=mask, width=width, height=height)