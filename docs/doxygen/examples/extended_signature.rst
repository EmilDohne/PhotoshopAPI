.. _extended_signature:

Example: `Extended Signature (C++ only)`
==========================================

This example covers creation of a LayeredFile (A Photoshop file) using the 'extended' signature, do note that this is almost never necessary 
and :ref:`create_simple_document` is the preferred (and encouraged) way to create files.

Here we read the `PhotoshopFile` structure directly instead of through the `LayeredFile` which gives us a view into the objects as they are
parsed from photoshop. This is primarily for debugging or if you wish to understand how the structures are parsed

Relevant documentation links:

- :cpp:struct:`LayeredFile` 
- :cpp:struct:`PhotoshopFile` 

.. tab:: C++

	.. literalinclude:: ../../../PhotoshopExamples/ExtendedSignature/main.cpp
	   :language: cpp