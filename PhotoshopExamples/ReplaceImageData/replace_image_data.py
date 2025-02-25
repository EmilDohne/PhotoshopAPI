# Example of replacing image data on a layer
import os
import numpy as np
import psapi


def srgb_to_linear(srgb: np.ndarray) -> np.ndarray:
    # Normalize the uint8 values to range [0, 1]
    normalized = srgb / 255.0

    # Apply the sRGB to linear transformation
    linear = np.where(normalized <= 0.04045, 
                      normalized / 12.92, 
                      np.power((normalized + 0.055) / 1.055, 2.4))
    
    # Convert back to uint8 by scaling to [0, 255]
    return np.round(linear * 255).astype(np.uint8)


def main() -> None:

    # In the python bindings we expose a wrapper which reads a LayeredFile instance with the
    # correct bit-depth
    layered_file = psapi.LayeredFile.read(os.path.join(os.path.dirname(__file__), "ImageData.psb"))

    # We can verify this by printing the type of layered_file
    print(type(layered_file))   # <--- Will print psapi.LayeredFile_8bit

    # One massive advantage is that we can use dict-like indexing to retrieve the layers
    img_layer: psapi.ImageLayer_8bit = layered_file["Blue_Lagoon"]["Blue_Lagoon.exr"]
    img_data: dict[int, np.ndarray] = img_layer.get_image_data()
    
    # Apply the sRGB to linear conversion for each channel in the image data, this could be any sort of operation
    # we could also resize the image data
    for channel, data in img_data.items():
        print(f"Processing channel: {channel}")
        img_data[channel] = srgb_to_linear(data)
        
    # Now we set the image data again and can write out the file!
    # Note that if we had rescaled the image data using opencv or similar we would have to 
    # pass the new width and height as the second and third argument!
    img_layer.set_image_data(img_data)
    layered_file.write(os.path.join(os.path.dirname(__file__), "ModifiedImageData.psb"))



if __name__ == "__main__":
    main()