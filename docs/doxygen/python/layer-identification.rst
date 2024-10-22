Layer Identification Functions
===============================

The PhotoshopAPI Python bindings include utility functions to help identify specific types of layers in a `LayeredFile`. These functions are provided to help you determine whether a layer is an image layer or a group layer.

Functions
----------

- **is_image_layer**: Check whether the provided layer is an image layer.

  .. code-block:: python

     def is_image_layer(layer: psapi.types.Layer) -> bool:
         """
         Check whether the provided layer is an image layer.
         """
         return isinstance(layer, (psapi.ImageLayer_8bit, psapi.ImageLayer_16bit, psapi.ImageLayer_32bit))

  **Arguments:**
  
  - `layer (psapi.types.Layer)`: The layer to be checked, which could be of any layer type.

  **Returns:**
  
  - `bool`: Returns `True` if the provided layer is an image layer (either 8-bit, 16-bit, or 32-bit), otherwise returns `False`.

  **Usage Example**:

  .. code-block:: python

     layer = psapi.Layer_8bit()  # Example layer
     if psapi.layer_util.is_image_layer(layer):
         print("This is an image layer.")
     else:
         print("This is not an image layer.")

- **is_group_layer**: Check whether the provided layer is a group layer.

  .. code-block:: python

     def is_group_layer(layer: psapi.types.Layer) -> bool:
         """
         Check whether the provided layer is a group layer.
         """
         return isinstance(layer, (psapi.GroupLayer_8bit, psapi.GroupLayer_16bit, psapi.GroupLayer_32bit))

  **Arguments:**
  
  - `layer (psapi.types.Layer)`: The layer to be checked, which could be of any layer type.

  **Returns:**
  
  - `bool`: Returns `True` if the provided layer is a group layer (either 8-bit, 16-bit, or 32-bit), otherwise returns `False`.

  **Usage Example**:

  .. code-block:: python

     layer = psapi.GroupLayer_16bit()  # Example group layer
     if psapi.layer_util.is_group_layer(layer):
         print("This is a group layer.")
     else:
         print("This is not a group layer.")

These utility functions simplify the process of checking layer types when working with complex `LayeredFile` structures. They ensure that you can identify and handle specific types of layers efficiently in your code.