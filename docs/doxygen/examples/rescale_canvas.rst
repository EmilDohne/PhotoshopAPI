.. _rescale_canvas:

Example: `Rescale File (Python only)`
=====================================

This example covers reading of a LayeredFile (A Photoshop file) after which we rescale all of the image data inside of it as well 
as any masks. Finally, we save out the file again

Relevant documentation links:

- :class:`psapi.ImageLayer_8bit`
- :class:`psapi.LayeredFile_8bit`

.. tab:: Python

	.. literalinclude:: ../../../PhotoshopExamples/RescaleCanvas/rescale_canvas.py
	   :language: python