Base Layer Structure
---------------------

.. note::
	This struct is not to be used directly but holds all the virtual base functions which all the other layers may use. It additionally defines the 
	``Layer<T>::Params`` struct which is expected to be passed into layer initialization for its subclassed types

.. doxygenstruct:: Layer
	:members: 
	:undoc-members: