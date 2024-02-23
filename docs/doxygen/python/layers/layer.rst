Layer
--------

Similar to the C++ bindings this is the base class for all layer types, unlike C++ there is no reason to actually use this type as casting down the 
inheritance chain is done implicitly. We can however use this to indicate a return type for type hinting.

.. code-block:: python

	def some_function() -> psapi.Layer_8bit:
		''' 
		Return a layer type
		'''
		pass


This is why we dont actually specify any constructors for the `Layer` class. If you try and construct a layer object you will get an error

.. code-block:: python

	layer = psapi.Layer_8bit() # -> TypeError: psapi.Layer_8bit: No constructor defined!

Similarly the Layer<>::Params class from C++ is not implemented here but rather the arguments are implemented as kwargs for the individual constructors

Class Reference Layer
============================

|

.. autoclass:: psapi.Layer_8bit
	:members:

