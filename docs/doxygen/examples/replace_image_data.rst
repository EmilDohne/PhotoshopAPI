.. _replace_image_data:

Example: `Replace image data`
====================================

This example covers reading of a LayeredFile (A Photoshop file) after which we access the image layers that contain image data.
We additionally cover how we can then extract, modify and replace that image data after which we save out the file again.

Relevant documentation links:

- :class:`psapi.ImageLayer_8bit`
- :cpp:struct:`ImageLayer` 

.. tab:: C++

	.. literalinclude:: ../../../PhotoshopExamples/ReplaceImageData/main.cpp
	   :language: cpp

.. tab:: Python

	.. literalinclude:: ../../../PhotoshopExamples/ReplaceImageData/replace_image_data.py
	   :language: python