.. _progress_callbacks:

Example: `Progress Callbacks (C++ only)`
=========================================

This example covers attaching a progress callback to the read/write operations of the LayeredFile allowing you to query the state
of those operations asynchronously which is especially helpful when reading heavy files

Relevant documentation links:

- :cpp:struct:`LayeredFile` 

.. tab:: C++

	.. literalinclude:: ../../../PhotoshopExamples/ProgressCallbacks/main.cpp
	   :language: cpp