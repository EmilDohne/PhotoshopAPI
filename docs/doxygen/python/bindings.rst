.. _bindings:

Python Bindings
===============

Below you can find a view of the main classes you will be interacting with when using the PhotoshopAPI python bindings.

You will find that these do not match the C++ functions 100% in an attempt to make the python bindings pythonic,
therefore not all functions can be translated directly 

.. caution::

	The python bindings, much like the PhotoshopAPI itself are still in early development and therefore
	subject to change. If you have any suggestions on how to improve them feel free to leave an issue on
	the github repo


.. toctree::
	:maxdepth: 1

	layeredfile.rst
	photoshopfile.rst
	enums.rst
	utility.rst


To get started with using these bindings, simply ``pip install PhotoshopAPI`` and then follow along with the examples in PhotoshopExamples
on the github page. 

Below you can find an example of extracting a layer from a 16-bit psb file and inserting it into 
an 8-bit psd file with minimal effort.

.. code-block:: python

	import photoshopapi as psapi
	import numpy as np
	import os


	def main():
		# Read both our files, they can be open at the same time or we can also read one file,
		# extract the layer and return just that layer if we want to save on RAM.
		file_src = psapi.LayeredFile.read("GraftSource_16.psb")
		file_dest = psapi.LayeredFile.read("GraftDestination_8.psd")

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
			blend_mode=lr_src.blend_mode, 
			opacity=lr_src.opacity
			)

		# add the layer and write out to file!
		file_dest.add_layer(img_layer_8bit)
		file_dest.write("GraftDestination_8_Edited.psd")


	if __name__ == "__main__":
		main()

