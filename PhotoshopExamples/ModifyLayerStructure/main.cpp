/*
Example of loading a PhotoshopFile, reordering the layer structure and saving it back out. This uses the simplified signature to access the data, 
if you instead want to use the "extended" signature which gives lower level access to the PhotoshopFile please look at the ExtendedSignature example.
*/

#include "PhotoshopAPI.h"

#include <unordered_map>
#include <vector>


int main()
{
	using namespace NAMESPACE_PSAPI;
		
	// In this case we already know the bit depth but otherwise one could use the PhotoshopFile.m_Header.m_Depth
	// variable on the PhotoshopFile to figure it out programmatically. This would need to be done using the 
	// "extended" read signature.
	LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read("LayeredFile.psd");

	// The Structure of the photoshop file we just opened is 
	// 'Group'
	//    'NestedGroup'
	//       'NestedImageLayer'
	//    'ImageLayer'
	//
	// and we now want to move the 'NestedGroup' and all of its children to the scene root
	// as well as delete 'ImageLayer'. It is advised to do the restructuring in steps 
	// as shown below to avoid trying to access a layer which no longer exists

	// By not specifying a second parameter to move_layer() we tell the function to move it to the scene root
	// we could however also move it under another group by passing that group as a second parameter
	layeredFile.move_layer("Group/NestedGroup");
	layeredFile.remove_layer("Group/ImageLayer");

	// One could also export as *.psb and PhotoshopAPI would take care of all the conversions internally
	LayeredFile<bpp8_t>::write(std::move(layeredFile), "RearrangedFile.psd");
}