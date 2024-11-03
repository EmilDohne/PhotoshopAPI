#include "LayeredFile.h"

#include "PhotoshopFile/PhotoshopFile.h"
#include "Macros.h"
#include "StringUtil.h"
#include "Core/Struct/TaggedBlock.h"
#include "LayerTypes/Layer.h"
#include "LayerTypes/ImageLayer.h"
#include "LayerTypes/GroupLayer.h"
#include "LayerTypes/AdjustmentLayer.h"
#include "LayerTypes/ArtboardLayer.h"
#include "LayerTypes/SectionDividerLayer.h"
#include "LayerTypes/ShapeLayer.h"
#include "LayerTypes/SmartObjectLayer.h"
#include "LayerTypes/TextLayer.h"

#include "LayeredFile/Util/GenerateHeader.h"
#include "LayeredFile/Util/GenerateColorModeData.h"
#include "LayeredFile/Util/GenerateImageResources.h"
#include "LayeredFile/Util/GenerateLayerMaskInfo.h"

#include <vector>


#include <variant>
#include <memory>
#include <filesystem>
#include <algorithm>


PSAPI_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
ICCProfile::ICCProfile(const std::filesystem::path& pathToICCFile)
{
	if (pathToICCFile.extension() != ".icc") [[unlikely]]
	{
		PSAPI_LOG_ERROR("ICCProfile", "Must pass a valid .icc file into the ctor. Got a %s", pathToICCFile.extension().string().c_str());
	}
	// Open a File object and read the raw bytes of the ICC file
	File iccFile = { pathToICCFile };
	m_Data = ReadBinaryArray<uint8_t>(iccFile, iccFile.getSize());
}

PSAPI_NAMESPACE_END