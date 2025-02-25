.. _graft_layer:

Example: `Graft Layer (Python only)`
====================================

This example is likely one of the more realistic scenarious of usage for the PhotoshopAPI. It shows how you can take a layer 
from one file and 'graft' it into another layer. In this example we also go from 16-bit -> 8-bit as well as mixing PSD and 
PSB files. 

All of this is handled by the PhotoshopAPI.

Relevant documentation links:

- :class:`psapi.ImageLayer_8bit`

.. tab:: Python

	.. literalinclude:: ../../../PhotoshopExamples/GraftLayer/graft_files.py
	   :language: python