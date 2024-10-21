# Example of replacing image data on a layer
import os
from typing import Union

import numpy as np
import cv2
import psapi


def is_image_layer(layer: Union[psapi.Layer_8bit, psapi.Layer_16bit, psapi.Layer_32bit]) -> bool:
    return isinstance(layer, (psapi.ImageLayer_8bit, psapi.ImageLayer_16bit, psapi.ImageLayer_32bit))


def scale_dimensions(layer: Union[psapi.Layer_8bit, psapi.Layer_16bit, psapi.Layer_32bit], scaling_factor: float) -> None:
    """
    Apply the scaling factor to width, height, center_x and center_y of the layer
    """
    layer.width    *= scaling_factor 
    layer.height   *= scaling_factor 
    layer.center_x *= scaling_factor 
    layer.center_y *= scaling_factor


def main() -> None:
    layered_file = psapi.LayeredFile.read(os.path.join(os.path.dirname(__file__), "BaseFile.psb"))

    # Scale up our canvas, this will not change any of the image data but only prepare it for later
    scaling_factor = 2
    layered_file.width *= scaling_factor
    layered_file.height *= scaling_factor

    # Loop over all the layers in the file and rescale them
    # Note that we have to do it in the following order:
    #
    # - Extract the image data we want to process
    # - Modify the layers' width and height properties
    # - Set the Image data back on the channel
    #
    # The reason for this is that we internally use the layers' width and height to deduce and 
    # verify the numpy array's shape. 

    for layer in layered_file.flat_layers:
        new_shape = (layer.width * scaling_factor, layer.height * scaling_factor)
        if is_image_layer(layer):
            image_data = layer.image_data
            scale_dimensions(layer, scaling_factor)
            for index, channel in image_data.items():

                rescaled_channel = cv2.resize(channel, new_shape, interpolation=cv2.INTER_LINEAR)
                layer[index] = rescaled_channel
        # As for image layers the image_data property will also return a mask channel we can safely 
        # apply the scaling here regardless
        elif layer.has_mask():
            mask = layer.mask
            scale_dimensions(layer, scaling_factor)
            layer.mask = cv2.resize(mask, new_shape, interpolation=cv2.INTER_LINEAR)
            
    # We can now write the scaled file back out!
    layered_file.write(os.path.join(os.path.dirname(__file__), "RescaledFile.psb"))



if __name__ == "__main__":
    main()