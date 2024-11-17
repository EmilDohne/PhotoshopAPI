/*
Example of loading a PhotoshopFile and extracting the image data, this can be freely used with any other operations present on the LayeredFile
*/

#include "PhotoshopAPI.h"

#include <unordered_map>
#include <vector>


int main()
{
	using namespace NAMESPACE_PSAPI;

	Instrumentor::Get().BeginSession("Tmp", "Tmp.json");

	//LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read("C:/Users/emild/Desktop/linkedlayers/smartobject_unlinked.psd");
	LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read("C:/Users/emild/Desktop/linkedlayers/warp/warptest.psd");

	Instrumentor::Get().EndSession();
}