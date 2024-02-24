#include "Enum.h"
#include "Macros.h"
#include "Struct/File.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
#include <pybind11/iostream.h>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


void declareFileStruct(py::module& m)
{
	py::class_<File> fileStruct(m, "File");
}


void declareChannelIDInfo(py::module& m)
{
	py::class_<Enum::ChannelIDInfo> channelIDInfo(m, "ChannelIDInfo", py::dynamic_attr());

	channelIDInfo.def_property("id",
		[](const Enum::ChannelIDInfo& self) { return self.id; },
		[](Enum::ChannelIDInfo& self, const Enum::ChannelID& other, const Enum::ColorMode& color_mode)
		{
			auto result = Enum::channelIDToChannelIDInfo(other, color_mode);
			self.id = result.id;
			self.index = result.index;
		});
	channelIDInfo.def_property("index",
		[](const Enum::ChannelIDInfo& self) { return self.index; },
		[](Enum::ChannelIDInfo& self, const int other, const Enum::ColorMode& color_mode)
		{
			auto result = Enum::intToChannelIDInfo(static_cast<int16_t>(other), color_mode);
			self.id = result.id;
			self.index = result.index;
		});
	channelIDInfo.def("__eq__", &Enum::ChannelIDInfo::operator ==, py::arg("other"));
}