/*
Example of creating a simple document with a single layer and a mask using the PhotoshopAPI.
*/

#include "PhotoshopAPI.h"

#include <unordered_map>
#include <vector>


int main()
{
	using namespace NAMESPACE_PSAPI;

	LayeredFile<bpp8_t>::read("C:/repos/PhotoshopAPI/PhotoshopExamples/AdjustmentLayers/adjustment_layers.psd");
}