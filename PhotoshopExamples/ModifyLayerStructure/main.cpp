/*
Example of loading a PhotoshopFile, reordering the layer structure and saving it back out. The preferred method is always to interact through the LayeredFile
struct rather than directly with the low level PhotoshopAPI::PhotoshopFile although that is also possible but requires a deep understanding of the
underlying structure whereas LayeredFile abstracts the implementation details.
*/

#include "PhotoshopAPI.h"

#include <unordered_map>
#include <vector>


int main()
{
	auto inputFile = PhotoshopAPI::File("./LayeredFile.psd");
	auto psDocumentPtr = std::make_unique<PhotoshopAPI::PhotoshopFile>();
	psDocumentPtr->read(inputFile);


	// In this case we already know the bit depth but otherwise one could use the PhotoshopFile.m_Header.m_Depth
	// variable on the PhotoshopFile to figure it out programmatically
	PhotoshopAPI::LayeredFile<PhotoshopAPI::bpp8_t> layeredFile = { std::move(psDocumentPtr) };

	// The Structure of the photoshop file we just opened is 
	// 'Group'
	//    'NestedGroup'
	//       'NestedImageLayer'
	//    'ImageLayer'
	//
	// and we now want to move the 'NestedGroup' and all of its children to the scene root
	// as well as delete 'ImageLayer'. It is advised to do the restructuring in steps 
	// as shown below to avoid trying to access a layer which no longer exists

	auto nestedGroupLayer = layeredFile.findLayer("Group/NestedGroup");
	if (nestedGroupLayer)
	{
		// By not specifying a second parameter we tell the function to move it to the scene root
		// we could however also move it under another group by passing that group as a parameter
		layeredFile.moveLayer(nestedGroupLayer);
	}

	auto imageLayer = layeredFile.findLayer("Group/ImageLayer");
	if (imageLayer)
	{
		layeredFile.removeLayer(imageLayer);
	}

	// We can now convert back to a PhotoshopFile and write out to disk
	PhotoshopAPI::File::FileParams params = { .doRead = false, .forceOverwrite = true };
	auto outputFile = PhotoshopAPI::File("./RearrangedFile.psd", params);
	auto psOutDocumentPtr = PhotoshopAPI::LayeredToPhotoshopFile(std::move(layeredFile));

	psOutDocumentPtr->write(outputFile);
}