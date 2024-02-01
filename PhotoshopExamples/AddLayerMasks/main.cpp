/*
Example of creating a simple document with a single layer using the PhotoshopAPI. The preferred method is always to interact through the LayeredFile
struct rather than directly with the low level PhotoshopAPI::PhotoshopFile although that is also possible but requires a deep understanding of the
underlying structure whereas LayeredFile abstracts the implementation details.
*/

#include "PhotoshopAPI.h"

#include <unordered_map>
#include <vector>


int main()
{
	NAMESPACE_PSAPI::Instrumentor::Get().BeginSession("PSAPI_Profile", "LargeReadWrite.json");

	auto inputFile = PhotoshopAPI::File("./large_file.psb");
	auto psDocumentPtr = std::make_unique<PhotoshopAPI::PhotoshopFile>();
	psDocumentPtr->read(inputFile);


	// In this case we already know the bit depth but otherwise one could use the PhotoshopFile.m_Header.m_Depth
	// variable on the PhotoshopFile to figure it out programmatically
	PhotoshopAPI::LayeredFile<PhotoshopAPI::bpp16_t> layeredFile = { std::move(psDocumentPtr) };

	// We can now convert back to a PhotoshopFile and write out to disk
	PhotoshopAPI::File::FileParams params = { .doRead = false, .forceOverwrite = true };
	auto outputFile = PhotoshopAPI::File("./large_file_api.psb", params);
	auto psOutDocumentPtr = PhotoshopAPI::LayeredToPhotoshopFile(std::move(layeredFile));

	psOutDocumentPtr->write(outputFile);

	NAMESPACE_PSAPI::Instrumentor::Get().EndSession();

}