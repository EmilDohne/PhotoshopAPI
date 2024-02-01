Layer and Mask Information Structure
-------------------------------------


.. doxygenstruct:: LayerAndMaskInformation
	:members:
	:undoc-members:

Layer Information Structure
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenstruct:: LayerInfo
	:members:
	:undoc-members:


Layer Records and Channel Image Data Structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Both of these sections exist as ``std::vector`` on the LayerInfo struct and they are intrinsically linked.
A LayerRecord always needs a ChannelImageData struct to go with it.

Layer Record
^^^^^^^^^^^^^


.. doxygenstruct:: LayerRecord
	:members:

Channel Image Data
^^^^^^^^^^^^^^^^^^

.. doxygenstruct:: ChannelImageData
	:members:
	:undoc-members: