#include <pybind11/pybind11.h>
#include "Macros.h"

#include "DeclareLayeredFile.h"
#include "DeclarePhotoshopFile.h"
#include "DeclareLayer.h"
#include "DeclareImageLayer.h"
#include "DeclareGroupLayer.h"
#include "DeclareEnums.h"
#include "DeclareUtil.h"

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;

PYBIND11_MODULE(psapi, m) {
	/*py::options options;
	options.disable_function_signatures();*/

	auto enum_module = m.def_submodule("enum", "A collection of enumerators used throughout the project.");
	declareBitDepthEnum(enum_module);
	declareColorModeEnum(enum_module);
	declareChannelIDEnum(enum_module);
	declareCompressionEnums(enum_module);
	declareBlendModeEnum(enum_module);

	auto util_module = m.def_submodule("util", "Utility functions and structures to support the creation/interaction with LayeredFile or PhotoshopFile");
	declareFileStruct(util_module);
	declareChannelIDInfo(util_module);

	declareLayer<bpp8_t>(m, "_8bit");
	declareLayer<bpp16_t>(m, "_16bit");
	declareLayer<bpp32_t>(m, "_32bit");

	declareLayeredFile<bpp8_t>(m, "_8bit");
	declareLayeredFile<bpp16_t>(m, "_16bit");
	declareLayeredFile<bpp32_t>(m, "_32bit");
	declareLayeredFileWrapper(m);

	declareImageLayer<bpp8_t>(m, "_8bit");
	declareImageLayer<bpp16_t>(m, "_16bit");
	declareImageLayer<bpp32_t>(m, "_32bit");

	declareGroupLayer<bpp8_t>(m, "_8bit");
	declareGroupLayer<bpp16_t>(m, "_16bit");
	declareGroupLayer<bpp32_t>(m, "_32bit");

	declarePhotoshopFile(m);
}