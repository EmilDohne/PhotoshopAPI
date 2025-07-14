ImageLayer
-----------------

About
=======

As with LayeredFile, we only provide documentation for the 8bit type here but 16- and 32-bit are both equally supported, all the classes can be seen below.

``ImageLayer_8bit`` 

``ImageLayer_16bit`` 

``ImageLayer_32bit``


One major change in the python bindings compared to the C++ code is that we allow for dict-like indexing of channels through the
``__getitem__`` method. This is very convenient as it significantly reduces the amount of code that has to be written. 

We currently support two methods of accessing channel data this way, by logical index or by channel id. Both of these methods 
are documented below

Examples
========

.. important::

	When accessing image data this way the image data is always copied out of the underlying structure meaning 
	we can keep accessing the same channel. 
	
	While this does mirror more pythonic behaviour, when wanting to explicitly remove a channels information from the layer (to save on memory)
	we have to revert to the individual :func:`psapi.LayeredFile_8bit.get_channel_by_id` and :func:`psapi.LayeredFile_8bit.get_channel_by_index` methods
	with the ``do_copy`` parameter set to false.

.. tab:: Get channel by logical index

	.. code-block:: python

		import photoshopapi as psapi
		import numpy as np

		layered_file = psapi.LayeredFile.read("File.psb")
		layer_red = layered_file["Layer_Red"]
		channel_r = layer_red[0] # 0-based indexing, raises KeyError if index is out of range
		# We can also extract alpha and masks this way with index -1 and -2 respectively!

.. tab:: Get channel by ID

	.. code-block:: python

		import photoshopapi as psapi
		import numpy as np

		layered_file = psapi.LayeredFile.read("File.psb")
		layer_red = layered_file["Layer_Red"]
		channel_r = layer_red[psapi.enum.ChannelID.red] # raises KeyError if index doesnt exist


Construction of an **ImageLayer** class is also intended to be as easy as possible which is why we support 3 different ways of passing data into the ImageLayer, 

- construction by **dict** with integer indexing
- construction by **dict** with ChannelID enum indexing
- construction by **np.ndarray**

Check out the examples below to find out how to leverage each of these to your advantage.


.. tab:: Construction by np.ndarray

	This method is likely the simplest (but least explicit) way of constructing an image layer. It does make some assumptions to achieve this simplicity. One of these
	assumptions is that a layer (for RGB) must have at least 3, but not more than 4 channels. This is due to us taking the required channels 
	(for RGB this would be R, G, B. For CMYK this would be C, M, Y, K etc.) and allowing for one last optional alpha channel.

	So if we continue our RGB example we could have a ``np.ndarray`` with shape (3, 32, 32) which would tell the PhotoshopAPI that we are dealing with no alpha channel
	(white alpha). If we however specify the following shape (4, 32, 32), the 4th channel is assumed to be the alpha channel and is interpreted as such.

	We support both 2D arrays (flattened image data) or 3D arrays (explicit rows and columns) which work interchangeably as long as the data is in row-major order.

	.. tab:: 2D array

		.. code-block:: python

			import photoshopapi as psapi
			import numpy as np

			document_color_mode = psapi.enum.ColorMode.rgb
			width = 32
			height = 32
			file = psapi.LayeredFile_8bit(document_color_mode, width, height)

			# This method has the image data flattened (each channel is a 1D array)
			img_data_np = np.zeros((4, height * width), np.uint8)	
			img_data_np[0] = 255	# Set the red component 
			img_data_np[3] = 128	# Set the alpha component

			# Construct our layer instance, width and height must be specified for this to work!
			img_lr_np = psapi.ImageLayer_8bit(
				img_data_np, 
				layer_name="img_lr_np", 
				width=width, 
				height=height, 
				color_mode=document_color_mode
			)

			# Add to the file and write out
			file.add_layer(img_lr_np)
			file.write("Out.psd")

	.. tab:: 3D array

		.. code-block:: python

			import photoshopapi as psapi
			import numpy as np

			document_color_mode = psapi.enum.ColorMode.rgb
			width = 32
			height = 32
			file = psapi.LayeredFile_8bit(document_color_mode, width, height)

			# This method has the channel data as 2D array in row-major fashion
			img_data_np = np.zeros((4, height, width), np.uint8)	
			img_data_np[0] = 255	# Set the red component 
			img_data_np[3] = 128	# Set the alpha component

			# Construct our layer instance, width and height must be specified for this to work!
			img_lr_np = psapi.ImageLayer_8bit(
				img_data_np, 
				layer_name="img_lr_np", 
				width=width, 
				height=height, 
				color_mode=document_color_mode
			)

			# Add to the file and write out
			file.add_layer(img_lr_np)
			file.write("Out.psd")

.. tab:: Construction by (int) dict

	Similarly to the numpy example these can also be constructed and passed either as flattened array or 2D array by simply interchanging this ``np.full((height, width), 0, np.uint8)``
	to this ``np.full((height * width), 0, np.uint8)``. 

	.. code-block:: python

		import photoshopapi as psapi
		import numpy as np

		document_color_mode = psapi.enum.ColorMode.rgb
		width = 32
		height = 32
		file = psapi.LayeredFile_8bit(document_color_mode, width, height)

		# We can use logical indices for these as should be familiar from other software
		image_dict = {}
		image_dict[0] = np.full((height, width), 0, np.uint8)		# Red channel
		image_dict[1] = np.full((height, width), 255, np.uint8)		# Green channel
		image_dict[2] = np.full((height, width), 0, np.uint8)		# Blue channel
		image_dict[-1] = np.full((height, width), 128, np.uint8)	# Alpha channel

		# Construct our layer instance, width and height must be specified for this to work!
		img_lr = psapi.ImageLayer_8bit(
			image_dict, 
			layer_name="img_lr_dict", 
			width=width, 
			height=height, 
			color_mode=document_color_mode)

		# Add to the file and write out
		file.add_layer(img_lr)
		file.write("Out.psd")

.. tab:: Construction by (enum) dict

	Similarly to the numpy example these can also be constructed and passed either as flattened array or 2D array by simply interchanging this ``np.full((height, width), 0, np.uint8)``
	to this ``np.full((height * width), 0, np.uint8)``. 

	.. attention::

		This method of construction is unfortunately currently unsupported and **will** result in photoshop files that cannot be opened due to limitations in pybind11
		as seen in this `issue <https://github.com/pybind/pybind11/issues/5020>`_. Support for this is planned however once that is resolved so keep tuned. This message
		will disappear once it is fixed :)

	.. code-block:: python

		import photoshopapi as psapi
		import numpy as np

		document_color_mode = psapi.enum.ColorMode.rgb
		width = 32
		height = 32
		file = psapi.LayeredFile_8bit(document_color_mode, width, height)

		# This method is a bit more explicit about the naming of channels
		image_dict = {}
		image_dict[psapi.enum.ChannelID.red] = np.full((height, width), 0, np.uint8)
		image_dict[psapi.enum.ChannelID.green] = np.full((height, width), 255, np.uint8)
		image_dict[psapi.enum.ChannelID.blue] = np.full((height, width), 0, np.uint8)
		image_dict[psapi.enum.ChannelID.alpha] = np.full((height, width), 128, np.uint8)

		# Construct our layer instance, width and height must be specified for this to work!
		img_lr = psapi.ImageLayer_8bit(
			image_dict, 
			layer_name="img_lr_dict", 
			width=width, 
			height=height, 
			color_mode=document_color_mode
		)

		# Add to the file and write out
		file.add_layer(img_lr)
		file.write("Out.psd")


Class Reference ImageLayer
============================

|

.. autoclass:: photoshopapi.ImageLayer_8bit
	:members:
	:inherited-members:

	.. automethod:: __init__
	.. automethod:: __getitem__


