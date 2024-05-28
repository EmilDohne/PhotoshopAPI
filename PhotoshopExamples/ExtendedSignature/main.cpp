/*
This is the ModifyLayerStructure example but instead of using the simplified read and write signature we use the extend
one to give us direct access to the PhotoshopFile. This is a lot more verbose than the alternative presented in the other examples
The preferred method is always to interact through the LayeredFile
struct rather than directly with the low level PhotoshopAPI::PhotoshopFile although that is also possible but requires a deep understanding of the
underlying structure whereas LayeredFile abstracts the implementation details.
*/

#include "PhotoshopAPI.h"

#include <unordered_map>
#include <vector>


int main()
{
	using namespace NAMESPACE_PSAPI;

	auto inputFile = File("./LayeredFile.psd");
	auto psDocumentPtr = std::make_unique<PhotoshopFile>();
	// The PhotoshopFile struct requires a progress callback to be passed which will be 
	// edited by the write function, to see an example of how to interact with this callback please refer
	// to the "ProgressCallbacks" example
	ProgressCallback callback{};
	psDocumentPtr->read(inputFile, callback);


	// In this case we already know the bit depth but otherwise one could use the PhotoshopFile.m_Header.m_Depth
	// variable on the PhotoshopFile to figure it out programmatically
	LayeredFile<bpp8_t> layeredFile = { std::move(psDocumentPtr) };

	// The Structure of the photoshop file we just opened is 
	// 'Group'
	//    'NestedGroup'
	//       'NestedImageLayer'
	//    'ImageLayer'
	//
	// and we now want to move the 'NestedGroup' and all of its children to the scene root
	// as well as delete 'ImageLayer'. It is advised to do the restructuring in steps 
	// as shown below to avoid trying to access a layer which no longer exists

	// By not specifying a second parameter to moveLayer() we tell the function to move it to the scene root
	// we could however also move it under another group by passing that group as a second parameter
	layeredFile.moveLayer("Group/NestedGroup");
	layeredFile.removeLayer("Group/ImageLayer");

	// This is the alternative signature which expects a layer ptr instead of a path.
	// It is useful if we already have the layer ptr from another operation like extracting data and then want to move
	// it etc.
	/*
	auto nestedGroupLayer = layeredFile.findLayer("Group/NestedGroup");
	if (nestedGroupLayer)
	{

		layeredFile.moveLayer(nestedGroupLayer);
	}
	auto imageLayer = layeredFile.findLayer();
	if (imageLayer)
	{
		layeredFile.removeLayer(imageLayer);
	}
	*/

	// We can now convert back to a PhotoshopFile and write out to disk
	File::FileParams params = File::FileParams();
	params.doRead = false;
	params.forceOverwrite = true;
	auto outputFile = File("./RearrangedFile.psd", params);
	auto psOutDocumentPtr = LayeredToPhotoshopFile(std::move(layeredFile));

	// We pass the same callback into the write function, clearing of data will 
	// be handled internally
	psOutDocumentPtr->write(outputFile, callback);
}