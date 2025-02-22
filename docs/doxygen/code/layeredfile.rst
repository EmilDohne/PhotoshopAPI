Structure Description: `LayeredFile`
====================================

The LayeredFile struct is the primary interface for editing Photoshop Files. It creates an abstraction over the actual implementation and provides more 
high level control over the Photoshop binary file format. It makes modifying a layer structure very convenient and as we are just shuffling pointers
around internally also very efficient


Usage
-----

The code snippets below show some examples of how one would generally use a LayeredFile. For more detailed examples please visit the PhotoshopExamples/ directory
on the github page

.. tab:: Read and Modify

	.. code-block:: cpp
		:caption: Read from disk and modify
	
		using namespace NAMESPACE_PSAPI;

		LayeredFile<bpp16_t> layeredFile = LayeredFile<bpp16_t>::read("InFile.psd");

		// Move a layer one level up in the hierarchy
		layeredFile.moveLayer("Group/NestedGroup/Image", "Group");

		// Delete the now empty group from the document
		layeredFile.removeLayer("Group/NestedGroup");

		// We can now convert the LayeredFile to a PhotoshopFile and write it out (this is done internally
		// but can be exposed, see the ExtendedSignature example for more information)
		LayeredFile<bpp16_t>::write(std::move(layeredFile), "OutFile.psd");


.. tab:: Initialize from scratch

	.. code-block:: cpp
		:caption: Initialize, modify and write

		using namespace PhotoshopAPI;

		const static uint32_t width = 64u;
		const static uint32_t height = 64u;

		LayeredFile<bpp8_t> document = { Enum::ColorMode::RGB, width, height };

		// Create our individual channels to add to our image layer. Keep in mind that all these 3 channels
		// need to be specified for RGB mode
		std::unordered_map <Enum::ChannelID, std::vector<bpp8_t>> channelMap;
		channelMap[Enum::ChannelID::Red] = std::vector<bpp8_t>(width * height, 255u);
		channelMap[Enum::ChannelID::Green] = std::vector<bpp8_t>(width * height, 0u);
		channelMap[Enum::ChannelID::Blue] = std::vector<bpp8_t>(width * height, 0u);

		ImageLayer<bpp8_t>::Params layerParams = {};
		layerParams.layerName = "Layer Red";
		layerParams.width = width;
		layerParams.height = height;

		auto layer = std::make_shared<ImageLayer<bpp8_t>>(std::move(channelMap), layerParams);
		document.addLayer(layer);

		LayeredFile<bpp8_t>::write(std::move(layeredFile), "OutFile.psd");



Layer Type Derivatives
----------------------

Below you can find a list of layers that one is able to add to the LayeredFile instance. Keep in mind that some of these are not fully implemented yet

.. toctree::
   :maxdepth: 2
   :caption: Layer Types:

   layers/baselayer.rst
   layers/group.rst
   layers/image.rst
   layers/sectiondivider.rst
   layers/smartobject.rst



Conversion Functions
---------------------

.. doxygenfunction:: layered_to_photoshop

|

Find Layer as specific type
----------------------------

.. doxygenfunction:: find_layer_as

|

ICC Profile Struct
--------------------

.. doxygenstruct:: ICCProfile
	:members:

|

LayeredFile Struct
------------------

.. doxygenstruct:: LayeredFile
	:members:

|

Layer Order Enum
-----------------

.. doxygenenum:: LayerOrder

|