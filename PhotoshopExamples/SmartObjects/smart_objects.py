# Example of replacing image data on a layer
import os
from typing import Union

import numpy as np
import cv2
import psapi


def main() -> None:

    # Reading the file works as normal, as well as accessing the layers.
    layered_file = psapi.LayeredFile.read(os.path.join(os.path.dirname(__file__), "SmartObject.psd"))
    warped_layer: psapi.SmartObjectLayer_8bit = layered_file["warped"]

    # If we want to add an additional smart object layer we can do this just like you would with a normal layer.
    # The only exception is that we have to provide a path for the image file we wish to link. We can additionally provide a warp
    # object which will reapply the given warp to the new layer. This is convenient if you wish to create a copy of a layer without
    # removing the other layer. These images are read using OpenImageIO so all of the file formats supported by it are supported
    # here
    layer_new = psapi.SmartObjectLayer_8bit(
        layered_file,
        path = os.path.join(os.path.dirname(__file__), "uv_grid.jpg"),
        layer_name = "uv_grid",
        link_type = psapi.enum.LinkedLayerType.external,
        warp = warped_layer.warp
    )

    # Smart objects also have more freedom in their transformations which we expose in the API:
    layer_new.rotate(45, layer_new.center_x, layer_new.center_y)
    layer_new.scale(.65, .65, layer_new.center_x, layer_new.center_y)
    layer_new.move(50, 50)

    # We can additionally also apply a perspective transformation using a 3x3 projective matrix.
    # This could also be used to skew or to directly apply all the above operations. This matrix e.g.
    # applies a perspective transform where the vanishing point is at 2048, 0 (keeping in mind that 0 
    # in this case is at the top of the canvas). 
    # If you wish to know more about how these 3x3 transformation matrices work this is a great video/channel:
    # https://www.youtube.com/watch?v=B8kMB6Hv2eI.
    matrix = np.zeros((3, 3), dtype=np.double)
    matrix[0][0] = 1
    matrix[1][1] = 1
    matrix[0][2] = 1 / 2048
    matrix[2][2] = 1
    layer_new.transform(matrix)

    # If you want to be a bit more descriptive of how these matrices are to be built you can create one using a source
    # and destination quad like this. Keep in mind that these have to be in the coordinate space of the image itself:
    source_transform = psapi.geometry.create_quad(2048, 2048)
    destination_transform = psapi.geometry.create_quad(2048, 2048)
    # Squash together the top of the image
    destination_transform[0].x = 512
    destination_transform[1].x = 1536

    # Create a 3x3 homography mapping the source_transform quad to the destination_transform quad.
    homography = psapi.geometry.create_homography(source_transform, destination_transform)
    layer_new.transform(homography)

    # If now we wanted to clear the transformation and/or warp that could be done as well. This will now just
    # be a smart object layer with width and height of 2048 (like the original image data).
    layer_new.reset_transform()
    layer_new.reset_warp()

    # For modifying the warp directly (bezier) we can push the individual points, although this is a bit more
    # advanced and we don't currently have a very convenient interface for it. This code right here 
    # pushes the top left corner to the 50, 50 position which will create a slightly rounded corner
    warp_pts = layer_new.warp.points
    warp_pts[0].x = 50
    warp_pts[0].y = 50
    layer_new.warp.points = warp_pts

    # adding these works just as with any other layers, writing also works as usual
    layered_file.add_layer(layer_new)
    layered_file.write(os.path.join(os.path.dirname(__file__), "SmartObjectOut.psd"))


if __name__ == "__main__":
    main()