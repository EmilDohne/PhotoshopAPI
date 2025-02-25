.. _create_groups:

Example: `Create groups`
====================================

This example covers creation of a LayeredFile (A Photoshop file) as well as that of a group layer which itself is added to the file and 
contains child layers.

Relevant documentation links:

- :class:`psapi.GroupLayer_8bit`
- :cpp:struct:`GroupLayer` 

.. tab:: C++

	.. literalinclude:: ../../../PhotoshopExamples/CreateGroups/main.cpp
	   :language: cpp

.. tab:: Python

	.. literalinclude:: ../../../PhotoshopExamples/CreateGroups/create_groups.py
	   :language: python