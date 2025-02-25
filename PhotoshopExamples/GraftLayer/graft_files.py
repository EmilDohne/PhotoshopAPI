'''
A more involved example showing how to 'graft' a layer from one file to another. We are using a worst-case scenario here
going from 16-bit -> 8-bit as well as going from PSB -> PSD and going from a 128x64px file -> 256x256px file but numpy
with the psapi make easy work of this.
'''
import psapi
import numpy as np
import os

def main() -> None:
    # Read both our files, they can be open at the same time or we can also read one file, extract the layer
    # and return just that layer if we want to save on RAM.
    file_src = psapi.LayeredFile.read(os.path.join(os.path.dirname(__file__), "GraftSource_16.psb"))
    file_dest = psapi.LayeredFile.read(os.path.join(os.path.dirname(__file__), "GraftDestination_8.psd"))

    # Extract the image data and convert to 8-bit.
    lr_src: psapi.ImageLayer_16bit = file_src["GraftSource"]
    img_data_src = lr_src.get_image_data()
    img_data_8bit = {}
    for key, value in img_data_src.items():
        value = value / 256 # Convert from 0-65535 -> 0-255
        img_data_8bit[key] = value.astype(np.uint8)

    # Reconstruct an 8bit converted layer
    img_layer_8bit = psapi.ImageLayer_8bit(
        img_data_8bit, 
        layer_name=lr_src.name, 
        width=lr_src.width, 
        height=lr_src.height, 
        pos_x=lr_src.center_x,
        pos_y=lr_src.center_y,
        blend_mode=lr_src.blend_mode, 
        opacity=lr_src.opacity
        )

    # add the layer and write out to file!
    file_dest.add_layer(img_layer_8bit)
    file_dest.write(os.path.join(os.path.dirname(__file__), "GraftDestination_8_Edited.psd"))

if __name__ == "__main__":
    main()