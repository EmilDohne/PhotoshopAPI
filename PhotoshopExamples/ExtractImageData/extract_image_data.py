# Example of creating a simple document with a single layer using the PhotoshopAPI.
import os
import numpy as np
import psapi


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
    mask_data = mask_layer.mask 
    # Dont worry if the mask data shows up as empty since photoshop has optimized this mask channel 
    # away due to it being full white. The `mask_default_color` property will hold the pixel value applied
    # outside of the masks bounding box

    # There is also no reason to actually store the mask layer, we can simply chain indexing calls
    mask_data = layered_file["Group"]["EmptyLayerWithMask"][-2]

    # Now we already knew all the layers' names but if we wish to iterate we can do that too
    for layer in layered_file.layers:
        if isinstance(layer, psapi.GroupLayer_8bit):
            # We could now recurse down to get all the levels 
            print(f"Found group layer: {layer.name}")
            print(f"And its sub-layers: {layer.layers}")


if __name__ == "__main__":
    main()