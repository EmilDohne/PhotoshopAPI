LayeredFile
-----------------

About
=======

The LayeredFile class is the main class for interacting with photoshop files as it abstracts away
the implementation of the underlying data structures and provides a simple to use interface over 
a generic image file with layers. Below we only document the ``LayeredFile_8bit`` as well as the helper class
for construction ``LayeredFile`` but there is one class for each bit-depth noted in :class:`psapi.enum.BitDepth` 
(``LayeredFile_16bit`` and ``LayeredFile_32bit`` respectively). 

The function signature for all of these is the same as with the 8-bit instance.

To get started with using the LayeredFile instance for reading/writing files check the two methods below.
The "Simple" example allows for the reading of a LayeredFile with a single line of code and will return 
one of the three bit depths types for files. 

The "Extended" example on the other hand shows essentially what the LayeredFile.read() method is doing 
under the hood.


.. tab:: Simple

	.. code-block:: python

		import photoshopapi as psapi

		file_path = "Path/To/File.psb"
		# This is a wrapper over the different LayeredFile_*bit types and will actually return the 
		# appropriate type depending on the file itself
		layered_file = psapi.LayeredFile.read(file_path)

		# it is however important to note that the layered_file variable will be one of 3 types
		# LayeredFile_8bit | LayeredFile_16bit | LayeredFile_32bit 
		
		# modify the layered_file...

		layered_file.write("Path/To/Out.psb")


.. tab:: Extended

	.. code-block:: python
	
		import photoshopapi as psapi

		file_path = "Path/To/File.psb"
		bit_depth: psapi.enum.BitDepth = psapi.PhotoshopFile.find_bitdepth(file_path)
		layered_file = None

		if bit_depth == psapi.enum.BitDepth.bd_8:
		   layered_file = psapi.LayeredFile_8bit.read(file_path)
		elif bit_depth == psapi.enum.BitDepth.bd_16:
		   layered_file = psapi.LayeredFile_16bit.read(file_path)
		elif bit_depth == psapi.enum.BitDepth.bd_32:
		   layered_file = psapi.LayeredFile_32bit.read(file_path)

		if not layered_file:
		   raise RuntimeError("Unable to deduce LayeredFile bit-depth")

		# modify the layered_file...

		layered_file.write("Path/To/Out.psb")

The reasoning behind the LayeredFile being split up between bit-depths is due to the templated
nature of the C++ classes. We try to provide convenience wrappers such as :class:`psapi.LayeredFile` to 
simplify the usage and typing of these as much as possible




Layer Type Derivatives
----------------------


Below you can find a list of layers that one is able to add to the LayeredFile instance.


.. toctree::
   :maxdepth: 1
   :caption: Layer Types:

   layers/layer.rst
   layers/image.rst
   layers/group.rst
   layers/smart_object.rst


Class Reference LayeredFile
============================

|

.. autoclass:: psapi.LayeredFile
	:members:

|

.. autoclass:: psapi.LayeredFile_8bit
	:members:

	.. automethod:: __getitem__

