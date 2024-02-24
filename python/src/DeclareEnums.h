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
	py::enum_<Enum::BitDepth> bitDepth(m, "BitDepth");

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
	py::enum_<Enum::ColorMode> colorMode(m, "ColorMode");
	colorMode.value("rgb", Enum::ColorMode::RGB);
	colorMode.value("cmyk", Enum::ColorMode::CMYK);
	colorMode.value("grayscale", Enum::ColorMode::Grayscale);
	// For now we dont document the others either

	colorMode.export_values();
}


void declareChannelIDEnum(py::module& m)
{
	py::enum_<Enum::ChannelID> channelID(m, "ChannelID");
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


void declareCompressionEnums(py::module& m)
{
	py::enum_<Enum::Compression> compression(m, "Compression");
	compression.value("raw", Enum::Compression::Raw);
	compression.value("rle", Enum::Compression::Rle);
	compression.value("zip", Enum::Compression::Zip);
	compression.value("zipprediction", Enum::Compression::ZipPrediction);

	compression.export_values();

}


void declareBlendModeEnum(py::module& m)
{
	py::enum_<Enum::BlendMode> blendMode(m, "BlendMode");
	blendMode.value("passthrough", Enum::BlendMode::Passthrough);
	blendMode.value("normal", Enum::BlendMode::Normal);
	blendMode.value("dissolve", Enum::BlendMode::Dissolve);
	blendMode.value("darken", Enum::BlendMode::Darken);
	blendMode.value("multiply", Enum::BlendMode::Multiply);
	blendMode.value("colorburn", Enum::BlendMode::ColorBurn);
	blendMode.value("linearburn", Enum::BlendMode::LinearBurn);
	blendMode.value("darkercolor", Enum::BlendMode::DarkerColor);
	blendMode.value("lighten", Enum::BlendMode::Lighten);
	blendMode.value("screen", Enum::BlendMode::Screen);
	blendMode.value("colordodge", Enum::BlendMode::ColorDodge);
	blendMode.value("lineardodge", Enum::BlendMode::LinearDodge);
	blendMode.value("lightercolor", Enum::BlendMode::LighterColor);
	blendMode.value("overlay", Enum::BlendMode::Overlay);
	blendMode.value("softlight", Enum::BlendMode::SoftLight);
	blendMode.value("hardlight", Enum::BlendMode::HardLight);
	blendMode.value("vividlight", Enum::BlendMode::VividLight);
	blendMode.value("linearlight", Enum::BlendMode::LinearLight);
	blendMode.value("pinlight", Enum::BlendMode::PinLight);
	blendMode.value("hardmix", Enum::BlendMode::HardMix);
	blendMode.value("difference", Enum::BlendMode::Difference);
	blendMode.value("exclusion", Enum::BlendMode::Exclusion);
	blendMode.value("subtract", Enum::BlendMode::Subtract);
	blendMode.value("divide", Enum::BlendMode::Divide);
	blendMode.value("hue", Enum::BlendMode::Hue);
	blendMode.value("saturation", Enum::BlendMode::Saturation);
	blendMode.value("color", Enum::BlendMode::Color);
	blendMode.value("luminosity", Enum::BlendMode::Luminosity);

	blendMode.export_values();

}