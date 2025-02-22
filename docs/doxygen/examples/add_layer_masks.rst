.. _add_layer_masks:

Example: `Add masks to layers`
====================================

This example covers creation of a LayeredFile (A Photoshop file) as well as that of an image layer while adding a pixel mask channel to
said layer.

Relevant documentation links:

- :class:`psapi.Layer_8bit`
- :cpp:struct:`Layer` 

.. tab:: C++

	.. literalinclude:: ../../../PhotoshopExamples/AddLayerMasks/main.cpp
	   :language: cpp

.. tab:: Python

	.. literalinclude:: ../../../PhotoshopExamples/AddLayerMasks/add_layer_masks.py
	   :language: python