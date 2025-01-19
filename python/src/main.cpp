#include <pybind11/pybind11.h>
#include "Macros.h"

#include "DeclareLayeredFile.h"
#include "DeclarePhotoshopFile.h"
#include "DeclareLayer.h"
#include "DeclareImageLayer.h"
#include "DeclareGroupLayer.h"
#include "DeclareSmartObjectLayer.h"
#include "Declare_ImageDataLayerType.h"
#include "DeclareSmartObjectWarp.h"
#include "DeclareEnums.h"
#include "DeclareGeometry.h"
#include "DeclareUtil.h"

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;

PYBIND11_MODULE(psapi, m) 
{
	auto enum_module = m.def_submodule("enum", "A collection of enumerators used throughout the project.");
	declare_linkedlayertype_enums(enum_module);
	declare_bitdepth_enums(enum_module);
	declare_colormode_enums(enum_module);
	declare_channelid_enums(enum_module);
	declare_compression_enums(enum_module);
	declare_blendmode_enums(enum_module);

	auto util_module = m.def_submodule("util", "Utility functions and structures to support the creation/interaction with LayeredFile or PhotoshopFile");
	declare_file_struct(util_module);
	declare_channelidinfo(util_module);

	auto geometry_module = m.def_submodule("geometry");
	declare_point2d(geometry_module);
	declare_geometry_operations(geometry_module);

	declare_layer<bpp8_t>(m, "_8bit");
	declare_layer<bpp16_t>(m, "_16bit");
	declare_layer<bpp32_t>(m, "_32bit");

	declare_image_data_layer_type<bpp8_t>(m, "_8bit");
	declare_image_data_layer_type<bpp16_t>(m, "_16bit");
	declare_image_data_layer_type<bpp32_t>(m, "_32bit");

	declare_layered_file<bpp8_t>(m, "_8bit");
	declare_layered_file<bpp16_t>(m, "_16bit");
	declare_layered_file<bpp32_t>(m, "_32bit");
	declare_layered_file_wrapper(m);

	declare_image_layer<bpp8_t>(m, "_8bit");
	declare_image_layer<bpp16_t>(m, "_16bit");
	declare_image_layer<bpp32_t>(m, "_32bit");

	declare_group_layer<bpp8_t>(m, "_8bit");
	declare_group_layer<bpp16_t>(m, "_16bit");
	declare_group_layer<bpp32_t>(m, "_32bit");

	declare_smart_object_layer<bpp8_t>(m, "_8bit");
	declare_smart_object_layer<bpp16_t>(m, "_16bit");
	declare_smart_object_layer<bpp32_t>(m, "_32bit");

	declare_smart_object_warp(m);
	
	declare_photoshop_file(m);
}