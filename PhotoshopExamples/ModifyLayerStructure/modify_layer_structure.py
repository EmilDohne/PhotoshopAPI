# Example of creating a simple document with a single layer using the PhotoshopAPI.
import os
import numpy as np
import psapi


def main() -> None:

    # In the python bindings we expose a wrapper which reads a LayeredFile instance with the
    # correct bit-depth
    layered_file = psapi.LayeredFile.read(os.path.join(os.path.dirname(__file__), "LayeredFile.psd"))

    # We can now perform any operations we want to, here we move the NestedGroup to the root of the scene by not specifying 
    # a second parameter and then remove the ImageLayer (this does not remove the Group!)
    layered_file.move_layer("Group/NestedGroup")
    layered_file.remove_layer("Group/ImageLayer")

    # We can also use objects to perform these operations. Note how the indexing reflects
    # what we did in the last operation
    nested_img_layer = layered_file["NestedGroup"]["NestedImageLayer"]
    group_layer = layered_file["Group"]

    # We now want to move the NestedImageLayer back under Group 
    layered_file.move_layer(nested_img_layer, group_layer)

    # Note the implicit conversion to PSB works smoothly without any additional work
    layered_file.write(os.path.join(os.path.dirname(__file__), "RearrangedFile.psb"))

if __name__ == "__main__":
    main()