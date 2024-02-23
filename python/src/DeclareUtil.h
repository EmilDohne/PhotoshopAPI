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
	py::class_<File> fileStruct(m, "File", R"pbdoc(
		An abstraction for a generic file structure. The implementation of which is not defined
		in the python bindings
	)pbdoc");
}


void declareChannelIDInfo(py::module& m)
{
	py::class_<Enum::ChannelIDInfo> channelIDInfo(m, "ChannelIDInfo", py::dynamic_attr(), R"pbdoc(
		Utility class which stores both the ID of the channel as well its logical index.
		This is done to allow for custom channels which have will have :class:`psapi.enum.ChannelID.Custom`
		as ID and then the corresponding index from 0-56.
	)pbdoc");

	channelIDInfo.def_property("id",
		[](const Enum::ChannelIDInfo& self) { return self.id; },
		[](Enum::ChannelIDInfo& self, const Enum::ChannelID& other, const Enum::ColorMode& color_mode)
		{
			auto result = Enum::channelIDToChannelIDInfo(other, color_mode);
			self.id = result.id;
			self.index = result.index;
		}, R"pbdoc(		
			When setting this property the ``index`` property is updated automatically to reflect this change
		)pbdoc"
	);
	channelIDInfo.def_property("index",
		[](const Enum::ChannelIDInfo& self) { return self.index; },
		[](Enum::ChannelIDInfo& self, const int other, const Enum::ColorMode& color_mode)
		{
			auto result = Enum::intToChannelIDInfo(static_cast<int16_t>(other), color_mode);
			self.id = result.id;
			self.index = result.index;
		}, R"pbdoc(		
			When setting this property the ``id`` property is updated automatically to reflect this change
		)pbdoc"
	);
	channelIDInfo.def("__eq__", &Enum::ChannelIDInfo::operator ==, py::arg("other"));
}