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
	LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read("C:/Users/emild/Desktop/ocio/custom_ocio.psb");
}