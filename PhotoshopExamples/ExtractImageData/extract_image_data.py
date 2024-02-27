# These are just for running these in the repository but is not necessary when the files are pip installed
import os, sys
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__),"../../bin-int/PhotoshopAPI/x64-release/python")))

# Example of creating a simple document with a single layer using the PhotoshopAPI.
import psapi
import numpy as np


def main() -> None:

    # In the python bindings we expose a wrapper which reads a LayeredFile instance with the
    # correct bit-depth
    layered_file = psapi.LayeredFile.read(os.path.join(os.path.dirname(__file__), "ImageData.psb"))

    # We can verify this by printing the type of layered_file
    print(type(layered_file))   # <--- Will print psapi.LayeredFile_8bit

    # One massive advantage is that we can use dict-like indexing to retrieve the layers
    img_layer: psapi.ImageLayer_8bit = layered_file["RedLayer"]    

    # and then again for channels
    channel_r = img_layer[0]
    channel_g = img_layer[1]
    channel_b = img_layer[2]
    print(f"Extracted image data: {channel_r}, {channel_g}, {channel_b}")

    # or if we just want all the data as dict 
    img_data: dict[int, np.ndarray] = img_layer.get_image_data()
    print(f"Image data retrieved as dict: {img_data}")

    # Similarly we can also extract masks in many different ways
    mask_layer: psapi.ImageLayer_8bit = layered_file["Group"]["EmptyLayerWithMask"]
    mask_data = mask_layer[-2]  # -2 is the index for mask channels
    mask_data = mask_layer.get_channel_by_id(psapi.enum.ChannelID.mask) # If we want to be more explicit
    mask_data = mask_layer.get_mask_data() 
    # Dont worry if the mask data shows up as empty since photoshop has optimized this mask channel away due to it being full white

    # There is also no reason to actually store the mask layer, we can simply chain indexing calls
    mask_data = layered_file["Group"]["EmptyLayerWithMask"][-2]

    # Now we already knew all the layers' names but if we wish to iterate we can do that too
    for layer in layered_file.layers:
        if isinstance(layer, psapi.GroupLayer_8bit):
            # We could now recurse down to get all the levels 
            print(f"Found group layer: {layer.name}")
            print(f"And its sub-layers: {layer.layers}")

    # Throughout the example so far we omitted a parameter on channel extraction which is
    # the 'do_copy' parameter. Setting it to false means image data isnt copied out but rather
    # extracted which means it is no longer accessible. For an example:
    red_channel = img_layer.get_channel_by_index(0, do_copy=False)
    
    try:
        # We use another way of indexing for simplicity here to show that all the methods
        # are compatible with one another
        red_channel_again = img_layer[0]    # This implicitly enables do_copy
    except RuntimeError:
        pass    # The PhotoshopAPI will already complain here


if __name__ == "__main__":
    main()