Type hinting
===============

The PhotoshopAPI Python bindings come with additional type hints to simplify and abbreviate return types and function arguments.
These type hints are designed to enhance code readability and streamline the development process by providing clear indications of the expected data types.

We provide these for the Layer types, including specific layer bit depths (8-bit, 16-bit, and 32-bit), as well as for `LayeredFile`.

Layer Types
-------------

For convenience, the following type hints are available:

- **Layer**: Represents any base layer that can be 8-bit, 16-bit, or 32-bit.
  
  .. code-block:: python

     Layer = psapi.Layer_8bit | psapi.Layer_16bit | psapi.Layer_32bit
     
  This allows function signatures to accept or return any of these layer types, reducing verbosity in type hinting.

- **ImageLayer**: Specifically represents image layers in 8-bit, 16-bit, or 32-bit.

  .. code-block:: python

     ImageLayer = psapi.ImageLayer_8bit | psapi.ImageLayer_16bit | psapi.ImageLayer_32bit
     
  Useful when you want to distinguish image layers from other types of layers such as groups.

- **GroupLayer**: Used to reference group layers in 8-bit, 16-bit, or 32-bit.

  .. code-block:: python

     GroupLayer = psapi.GroupLayer_8bit | psapi.GroupLayer_16bit | psapi.GroupLayer_32bit

- **LayeredFile**: Represents files that contain layers, in 8-bit, 16-bit, or 32-bit formats.

  .. code-block:: python

     LayeredFile = psapi.LayeredFile_8bit | psapi.LayeredFile_16bit | psapi.LayeredFile_32bit

Examples
---------

Here are some examples demonstrating how you can use these type hints in your code:

.. code-block:: python

   def process_layer(layer: Layer) -> None:
       ...
   
   def get_image_layer() -> ImageLayer:
       # Your logic to return an image layer
       pass
   
   def organize_group_layer(layer: GroupLayer) -> None:
       # Handle the group layer in the layered file
       pass

   def open_layered_file(file: LayeredFile) -> None:
       # Open and process a layered file
       pass
