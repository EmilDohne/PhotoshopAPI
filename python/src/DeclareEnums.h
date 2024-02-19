#include "Enum.h"
#include "Macros.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
#include <pybind11/iostream.h>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


void declareBitDepthEnum(py::module& m)
{
	py::enum_<Enum::BitDepth> bitDepth(m, "BitDepth", 
		R"pbdoc(
		Enum representing the bit depth of an image.

        The supported bit depths are:
			- ``bd_8``: 8 bits per channel.
			- ``bd_16``: 16 bits per channel.
			- ``bd_32``: 32 bits per channel.
        )pbdoc");
	bitDepth.value("bd_8", Enum::BitDepth::BD_8,
		R"pbdoc(
		8-bits per channel, equivalent to a numpy.uint8
		)pbdoc");
	bitDepth.value("bd_16", Enum::BitDepth::BD_16,
		R"pbdoc(
		16-bits per channel, equivalent to a numpy.uint16
		)pbdoc");
	bitDepth.value("bd_32", Enum::BitDepth::BD_32, 
		R"pbdoc(
		32-bits per channel, equivalent to a numpy.float32
		)pbdoc");
	// Note that we do not expose the bd_1 enum
	// for now as it isnt within the currently supported
	// bit-depths

	bitDepth.export_values();
}


void declareColorModeEnum(py::module& m)
{
	py::enum_<Enum::ColorMode> colorMode(m, "ColorMode",
		R"pbdoc(
		Enum representing the color mode of an file.

        The supported color modes are:
			- ``rgb``: rgb color mode

		more will be added before the 1.0.0 release
        )pbdoc");
	colorMode.value("rgb", Enum::ColorMode::RGB,
		R"pbdoc(
		rgb color mode (supports channels R, G, B and A)
		)pbdoc");
	colorMode.value("cmyk", Enum::ColorMode::CMYK,
		R"pbdoc(
		cmyk color mode (supports channels C, M, Y, K and A)
		)pbdoc");
	colorMode.value("grayscale", Enum::ColorMode::Grayscale,
		R"pbdoc(
		grayscale color mode (supports channels Gray, A)
		)pbdoc");
	// For now we dont document the others either

	colorMode.export_values();
}


void declareChannelIDEnum(py::module& m)
{
	py::enum_<Enum::ChannelID> channelID(m, "ChannelID",
		R"pbdoc(
		Enum representation of all the different channel ids found in a file.
		)pbdoc");

	channelID.value("red", Enum::ChannelID::Red);
	channelID.value("green", Enum::ChannelID::Green);
	channelID.value("blue", Enum::ChannelID::Blue);
	channelID.value("cyan", Enum::ChannelID::Cyan);
	channelID.value("magenta", Enum::ChannelID::Magenta);
	channelID.value("yellow", Enum::ChannelID::Yellow);
	channelID.value("black", Enum::ChannelID::Black);
	channelID.value("gray", Enum::ChannelID::Gray);
	channelID.value("custom", Enum::ChannelID::Custom);
	channelID.value("mask", Enum::ChannelID::UserSuppliedLayerMask);
	channelID.value("alpha", Enum::ChannelID::Alpha);
}