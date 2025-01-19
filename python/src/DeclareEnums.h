#include "Util/Enum.h"
#include "Macros.h"
#include "LayeredFile/LinkedData/LinkedLayerData.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
#include <pybind11/iostream.h>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;

void declare_linkedlayertype_enums(py::module& m)
{
	py::enum_<LinkedLayerType> linked_layer_type(m, "LinkedLayerType", R"pbdoc(

		Enum representing the bit depth of an image.

		Attributes
		-------------

		data : int
			The original image data is stored directly on the photoshop file
			and is therefore packaged and contained.
		external : int
			The original image data is stored in a file on disk, for packaging
			the file therefore has to be shipped alongside the photoshop file.

	)pbdoc");

	linked_layer_type.value("data", LinkedLayerType::data);
	linked_layer_type.value("external", LinkedLayerType::external);

	linked_layer_type.export_values();
}


void declare_bitdepth_enums(py::module& m)
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


void declare_colormode_enums(py::module& m)
{
	py::enum_<Enum::ColorMode> colorMode(m, "ColorMode", R"pbdoc(
		Enum representing the color mode of an file.

		Attributes
		------------ -

		rgb : int
			rgb color mode(supports channels R, G, B and A)
		cmyk : int
			cmyk color mode(supports channels C, M, Y, K and A)
		grayscale : int
			grayscale color mode(supports channels Gray, A)

	)pbdoc");

	colorMode.value("rgb", Enum::ColorMode::RGB);
	colorMode.value("cmyk", Enum::ColorMode::CMYK);
	colorMode.value("grayscale", Enum::ColorMode::Grayscale);
	// For now we dont document the others either

	colorMode.export_values();
}


void declare_channelid_enums(py::module& m)
{
	py::enum_<Enum::ChannelID> channelID(m, "ChannelID", R"pbdoc(
		Enum representation of all the different channel ids found in a file.

		Attributes
		-----------

		red: int

		green: int

		blue: int

		cyan: int

		magenta: int

		yellow: int

		black: int

		gray: int

		custom: int

		mask: int

		alpha: int

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


void declare_compression_enums(py::module& m)
{
	py::enum_<Enum::Compression> compression(m, "Compression", R"pbdoc(
		Enum representation of all the different Compression codecs supported by Photoshop (and PSAPI).

		Attributes
		-------------

		raw : int
			encode as raw bytes (no compression)
		rle : int
			encode with run-length-encoding for fastest write speeds at the cost of lower compression ratios (especially for 16- and 32-bit)
		zip : int
			encode with zip (deflate) compression, usually the best compression codec choice as well as zipprediction
		zipprediction : int
			encode with zip (deflate) compression but additionally 'prediction' encode the data which takes the difference between the last and 
			the current pixel per scanline and stores that (for 32-bit files it interleaves the bytes).

	)pbdoc");

	compression.value("raw", Enum::Compression::Raw);
	compression.value("rle", Enum::Compression::Rle);
	compression.value("zip", Enum::Compression::Zip);
	compression.value("zipprediction", Enum::Compression::ZipPrediction);

	compression.export_values();

}


void declare_blendmode_enums(py::module& m)
{
	py::enum_<Enum::BlendMode> blendMode(m, "BlendMode", R"pbdoc(
		Enum representation of all the different blendmodes found in a file.

		Attributes
		-----------

		passthrough: int
			Reserved for Group layers only
		normal: int

		dissolve: int

		darken: int

		multiply: int

		colorburn: int

		linearburn: int

		darkercolor: int

		lighten: int

		screen: int

		colordodge: int

		lineardodge: int

		lightercolor: int

		overlay: int

		softlight: int

		hardlight: int

		vividlight: int

		linearlight: int

		pinlight: int

		hardmix: int

		difference: int

		exclusion: int

		subtract: int

		divide: int

		hue: int

		saturation: int

		color: int

		luminosity: int

	)pbdoc");

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