GroupLayer
-----------------

As with LayeredFile, we only provide documentation for the 8bit type here but 16- and 32-bit are both equally supported, all the classes can be seen below.

``GroupLayer_8bit`` 

``GroupLayer_16bit`` 

``GroupLayer_32bit``


Similar to the LayeredFile, we can access child layers using the ``__getitem__`` function with dict-like indexing ``[]``. As long as the names of these layers 
are valid we can chain as many of these as we want.

.. code-block:: python

	group_layer = # group_layer instance with some child nodes
	image_layer = group_layer["NestedGroup"]["MoreNested"]["ImageLayer"]

	# We could also combine this with the channel indexing of the ImageLayer:
	channel_r = group_layer["NestedGroup"]["MoreNested"]["ImageLayer"][0]
	# or
	channel_r = group_layer["NestedGroup"]["MoreNested"]["ImageLayer"][psapi.enum.ChannelID.red]



Class Reference GroupLayer
============================

|

.. autoclass:: psapi.GroupLayer_8bit
	:members:
	:inherited-members:

	.. automethod:: __init__
	.. automethod:: __getitem__


