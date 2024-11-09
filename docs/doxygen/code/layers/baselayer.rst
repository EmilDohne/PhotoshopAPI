Base Layer Structure
---------------------

.. note::
	This struct is not to be used directly but holds all the virtual base functions which all the other layers may use. It additionally defines the 
	``Layer<T>::Params`` struct which is expected to be passed into layer initialization for its subclassed types


Layer Mask Type
=======================

.. doxygenstruct:: LayerMask
	:members:

Base Layer Type
=======================

This is the base definition for all layer types which holds generic data such as name, mask data, opacity etc.
This class is not supposed to be used directly but instead through any of its subclassed layers as this layer type
doesn't exist in Photoshop itself. There is a further specialization `_ImageDataLayerType<T>` which acts as a generic 
interface for ImageData types such as `SmartObjectLayer<T>` and `ImageLayer<T>`

.. doxygenstruct:: Layer
	:members: 
	:undoc-members: