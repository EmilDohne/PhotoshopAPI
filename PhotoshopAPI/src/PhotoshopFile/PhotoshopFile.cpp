#include "PhotoshopFile.h"

#include "FileHeader.h"
#include "ColorModeData.h"
#include "ImageResources.h"
#include "LayerAndMaskInformation.h"
#include "ImageData.h"


#include "Profiling/Perf/Instrumentor.h"

PSAPI_NAMESPACE_BEGIN


// Read our PhotoshopFile section by section
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PhotoshopFile::read(File& document, ProgressCallback& callback)
{
	PSAPI_PROFILE_FUNCTION();
	callback.resetCount();
	// These three sections are trivial in terms of read performance so we simply ignore 
	// incrementing the callback on them
	m_Header.read(document);
	m_ColorModeData.read(document);
	m_ImageResources.read(document, m_ColorModeData.offset() + m_ColorModeData.size());

	m_LayerMaskInfo.read(document, m_Header, callback, m_ImageResources.offset() + m_ImageResources.size());
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PhotoshopFile::write(File& document, ProgressCallback& callback)
{
	PSAPI_PROFILE_FUNCTION();

	callback.resetCount();
	// These three sections are trivial in terms of write performance so we simply ignore 
	// incrementing the callback on them
	m_Header.write(document);
	m_ColorModeData.write(document, m_Header);
	m_ImageResources.write(document);

	m_LayerMaskInfo.write(document, m_Header, callback);
	// This unfortunately appears to be required which inflates files by quite a bit
	// but still significantly less than photoshop itself
	callback.setTask("Writing ImageData section");
	m_ImageData.write(document, m_Header);
	callback.increment();
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
Enum::BitDepth PhotoshopFile::findBitdepth(std::filesystem::path file)
{
	if (file.extension() != ".psb" && file.extension() != ".psd")
	{
		PSAPI_LOG_ERROR("PhotoshopFile", "Invalid file extension '%s' encountered. Only '.psd' and '.psb' are supported", file.extension().string().c_str());
	}

	File document(file);
	FileHeader header;
	header.read(document);
	return header.m_Depth;
}

PSAPI_NAMESPACE_END