ImageLayer
-----------------

As with LayeredFile, we only provide documentation for the 8bit type here but 16- and 32-bit are both equally supported, all the classes can be seen below.

``ImageLayer_8bit`` 

``ImageLayer_16bit`` 

``ImageLayer_32bit``

|

One major change in the python bindings compared to the C++ code is that we allow for dict-like indexing of channels through the
``__getitem__`` method. This is very convenient as it significantly reduces the amount of code that has to be written. 

We currently support two methods of accessing channel data this way, by logical index or by channel id. Both of these methods 
are documented below

.. important::

	When accessing image data this way the image data is always copied out of the underlying structure meaning 
	we can keep accessing the same channel. 
	
	While this does mirror more pythonic behaviour, when wanting to explicitly discard a channels information 
	we have to revert to the individual :func:`psapi.LayeredFile_8bit.get_channel_by_id` and :func:`psapi.LayeredFile_8bit.get_channel_by_index` methods
	with the ``do_copy`` parameter set to false.

.. tab:: Get channel by logical index

	.. code-block:: python

		import psapi
		import numpy as np

		layered_file = psapi.LayeredFile.read("File.psb")
		layer_red = layered_file["Layer_Red"]
		channel_r = layer_red[0] # 0-based indexing, raises KeyError if index is out of range

.. tab:: Get channel by ID

	.. code-block:: python

		import psapi
		import numpy as np

		layered_file = psapi.LayeredFile.read("File.psb")
		layer_red = layered_file["Layer_Red"]
		channel_r = layer_red[psapi.enum.ChannelID.red] # raises KeyError if index doesnt exist

Class Reference ImageLayer
============================

|

.. autoclass:: psapi.ImageLayer_8bit
	:members:

	.. automethod:: __getitem__


