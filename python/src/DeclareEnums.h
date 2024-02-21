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
	py::enum_<Enum::BitDepth> bitDepth(m, "BitDepth", R"pbdoc(
		Enum representing the bit depth of an image.

        Attributes
		-------------

		bd_8 : int
			8-bits per channel, equivalent to numpy.uint8
		bd_16 : int
			16-bits per channel, equivalent to numpy.uint16
		bd_32 : int
			32-bits per channel, equivalent to numpy.float32

        )pbdoc");
	bitDepth.value("bd_8", Enum::BitDepth::BD_8);
	bitDepth.value("bd_16", Enum::BitDepth::BD_16);
	bitDepth.value("bd_32", Enum::BitDepth::BD_32);
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

		Attributes
		-------------

		rgb : int
			rgb color mode (supports channels R, G, B and A)
		cmyk : int
			cmyk color mode (supports channels C, M, Y, K and A)
		grayscale : int
			grayscale color mode (supports channels Gray, A)

        )pbdoc");
	colorMode.value("rgb", Enum::ColorMode::RGB);
	colorMode.value("cmyk", Enum::ColorMode::CMYK);
	colorMode.value("grayscale", Enum::ColorMode::Grayscale);
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

	channelID.export_values();

}