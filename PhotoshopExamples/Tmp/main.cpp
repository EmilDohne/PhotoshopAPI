/*
Example of loading a PhotoshopFile and extracting the image data, this can be freely used with any other operations present on the LayeredFile
*/
#include <OpenImageIO/imageio.h>
#include "PhotoshopAPI.h"
#include "Core/Render/Render.h"
#include "Core/Render/Interleave.h"

#include <unordered_map>
#include <vector>
#include <string>
#include <span>
#include <format>

using namespace NAMESPACE_PSAPI;


int main()
{
	using namespace NAMESPACE_PSAPI;

	Instrumentor::Get().BeginSession("Tmp", "Tmp.json");

	LayeredFile<bpp8_t> file = LayeredFile<bpp8_t>::read("C:/repos/PhotoshopAPI/bin-int/PhotoshopAPI/x64-release/python/image_data/out.psb");
	auto layer_ptr = find_layer_as<bpp8_t, SmartObjectLayer>("SmartObjectLayer", file);

	Instrumentor::Get().EndSession();
}