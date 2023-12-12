#include "PhotoshopFile.h"

#include "FileHeader.h"
#include "ColorModeData.h"
#include "ImageResources.h"
#include "LayerAndMaskInformation.h"
#include "ImageData.h"

#include "Profiling/Perf/Instrumentor.h"

PSAPI_NAMESPACE_BEGIN


// Read our PhotoshopFile layer by layer
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
bool PhotoshopFile::read(File& document)
{
	PROFILE_FUNCTION();

	bool wasSuccessful = true;

	wasSuccessful = wasSuccessful && m_Header.read(document);
	wasSuccessful = wasSuccessful && m_ColorModeData.read(document);
	wasSuccessful = wasSuccessful && m_ImageResources.read(document, m_ColorModeData.m_Offset + m_ColorModeData.m_Size);
	wasSuccessful = wasSuccessful && m_LayerMaskInfo.read(document, m_Header, m_ImageResources.m_Offset + m_ImageResources.m_Size);

	return wasSuccessful;
}


void PhotoshopFile::write(File& document)
{
	PROFILE_FUNCTION();

	m_Header.write(document);
	m_ColorModeData.write(document, m_Header);
	m_ImageResources.write(document);
	//m_LayerMaskInfo.write(document);
}


PSAPI_NAMESPACE_END