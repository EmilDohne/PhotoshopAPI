Structure Description: `LayeredFile`
====================================

The LayeredFile struct is the primary interface for editing Photoshop Files. It creates an abstraction over the actual implementation and provides more 
high level control over the Photoshop binary file format. It makes modifying a layer structure very convenient and as we are just shuffling pointers
around internally also very efficient


Usage
-----

The code snippets below show some examples of how one would generally use a LayeredFile. For more detailed examples please visit the PhotoshopExamples/ directory
on the github 

.. code-block:: cpp
	:caption: Initialize from PhotoshopFile
	
	using namespace PhotoshopAPI;

	// Assuming we already have a unique_ptr to a PhotoshopFile named psDocumentPtr to use
	// as well as knowing we have a 16-bit document. If we wouldnt know that we could figure that 
	// out using the psDocumentPtr->m_Header.m_Depth variable
	LayeredFile<bpp16_t> layeredFile = { std::move(psDocumentPtr) };

	// Move a layer one level up in the hierarchy
	layeredFile.moveLayer("Group/NestedGroup/Image", "Group");

	// Delete the now empty group from the document
	layeredFile.removeLayer("Group/NestedGroup");

	// We can now convert the LayeredFile to a PhotoshopFile and write it out
	File::FileParams params = { .doRead = false, .forceOverwrite = true };
	auto outputFile = File("./OutFile.psd", params);
	auto psOutDocumentPtr = LayeredToPhotoshopFile(std::move(layeredFile));

	psOutDocumentPtr->write(outputFile);


Alternatively we could initialize from scratch and add layers into the document at will

.. code-block:: cpp
	:caption: Initialize and add some layers

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

	// We could now do the same as the example above to write the file to disk



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


   layers/adjustment.rst
   layers/artboard.rst
   layers/smartobject.rst
   layers/textlayer.rst



Conversion Functions
---------------------

.. doxygenfunction:: LayeredToPhotoshopFile

LayeredFile Struct
------------------

.. doxygenstruct:: LayeredFile
	:members:


Layer Order Enum
-----------------

.. doxygenenum:: LayerOrder