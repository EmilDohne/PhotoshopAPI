#include "PhotoshopFile.h"

#include "FileHeader.h"
#include "ColorModeData.h"
#include "ImageResources.h"
#include "LayerAndMaskInformation.h"
#include "ImageData.h"

PSAPI_NAMESPACE_BEGIN


bool PhotoshopFile::read(File& document)
{
	bool wasSuccessful = true;

	wasSuccessful = wasSuccessful && m_Header.read(document);
	wasSuccessful = wasSuccessful && m_ColorModeData.read(document);
	wasSuccessful = wasSuccessful && m_ImageResources.read(document, m_ColorModeData.m_Offset + m_ColorModeData.m_Size);
	wasSuccessful = wasSuccessful && m_LayerMaskInfo.read(document, m_Header, m_ImageResources.m_Offset + m_ImageResources.m_Size);

	return wasSuccessful;
}


PSAPI_NAMESPACE_END