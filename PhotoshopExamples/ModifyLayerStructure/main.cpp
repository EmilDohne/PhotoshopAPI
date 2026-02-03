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
	auto file_ocio = LayeredFile<bpp16_t>::read(
		"C:/repos/PhotoshopAPI/PhotoshopTest/documents/ColorManagement/color_checker_ocio_acescg_16.psd"
	);

	auto file_icc = LayeredFile<bpp16_t>::read(
		"C:/repos/PhotoshopAPI/PhotoshopTest/documents/ColorManagement/color_checker_icc_srgb_16.psd"
	);
}