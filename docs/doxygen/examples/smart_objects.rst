.. _smart_objects:

Example: `Smart Object Layer`
====================================

This example covers modifying a SmartObject layer by both creating a new one from a file on disk, as well as modifying the warp and 
applying the layer's warp to another different layer.
It additionally covers using transformations on the layer.

Relevant documentation links:

- :class:`psapi.SmartObjectLayer_8bit`
- :cpp:struct:`SmartObjectLayer` 

.. tab:: C++

	.. literalinclude:: ../../../PhotoshopExamples/SmartObjects/main.cpp
	   :language: cpp

.. tab:: Python

	.. literalinclude:: ../../../PhotoshopExamples/SmartObjects/smart_objects.py
	   :language: python