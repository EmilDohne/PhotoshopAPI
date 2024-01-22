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
void PhotoshopFile::read(File& document)
{
	PROFILE_FUNCTION();
	m_Header.read(document);
	m_ColorModeData.read(document);
	m_ImageResources.read(document, m_ColorModeData.m_Offset + m_ColorModeData.m_Size);
	m_LayerMaskInfo.read(document, m_Header, m_ImageResources.m_Offset + m_ImageResources.m_Size);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PhotoshopFile::write(File& document)
{
	PROFILE_FUNCTION();

	m_Header.write(document);
	m_ColorModeData.write(document, m_Header);
	m_ImageResources.write(document);
	m_LayerMaskInfo.write(document, m_Header);
	// This unfortunately appears to be required which inflates files by quite a bit
	m_ImageData.write(document, m_Header);
}


PSAPI_NAMESPACE_END