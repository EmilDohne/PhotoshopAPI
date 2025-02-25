.. _modify_structure:

Example: `Modify layer structure`
====================================

This example covers reading of a LayeredFile (A Photoshop file) after which we modify the layer structure by moving layers,
removing layers etc. The PhotoshopAPI gives you full control over the layer hierarchy.

Relevant documentation links:

- :class:`psapi.GroupLayer_8bit`
- :class:`psapi.LayeredFile_8bit`
- :cpp:struct:`GroupLayer` 
- :cpp:struct:`LayeredFile` 

.. tab:: C++

	.. literalinclude:: ../../../PhotoshopExamples/ModifyLayerStructure/main.cpp
	   :language: cpp

.. tab:: Python

	.. literalinclude:: ../../../PhotoshopExamples/ModifyLayerStructure/modify_layer_structure.py
	   :language: python