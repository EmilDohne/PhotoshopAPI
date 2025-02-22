.. _extract_image_data:

Example: `Extract image data`
====================================

This example covers reading of a LayeredFile (A Photoshop file) after which we access the image layers that contain image data.
We additionally cover how we can then extract that image data using the API as `std::vector<T>` or `numpy.ndarray` depending 
on the language

Relevant documentation links:

- :class:`psapi.ImageLayer_8bit`
- :cpp:struct:`ImageLayer` 

.. tab:: C++

	.. literalinclude:: ../../../PhotoshopExamples/ExtractImageData/main.cpp
	   :language: cpp

.. tab:: Python

	.. literalinclude:: ../../../PhotoshopExamples/ExtractImageData/extract_image_data.py
	   :language: python