#include "doctest.h"

#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"

#include <cmath>
#include <filesystem>
#include <memory>
#include <string>

using namespace NAMESPACE_PSAPI;

namespace
{
	template <typename T>
	std::shared_ptr<TextLayer<T>> find_text_layer(LayeredFile<T>& file, const std::string& name)
	{
		for (auto& layer : file.flat_layers())
		{
			auto tl = std::dynamic_pointer_cast<TextLayer<T>>(layer);
			if (tl && tl->name() == name)
				return tl;
		}
		return nullptr;
	}
}


// ── Warp fixture: TextLayers_Warp.psd ──────────────────────────────────
// Contains:
//   "WarpArc"   – warpStyle: ARC, bend 28, hDistort 0, vDistort 0
//   "Secondary"  – no warp (warpNone)

TEST_CASE("Warp: has_warp returns true for warped layer")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Warp.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "WarpArc");
	REQUIRE(layer != nullptr);

	CHECK(layer->has_warp() == true);
}


TEST_CASE("Warp: has_warp returns false for non-warped layer")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Warp.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "Secondary");
	REQUIRE(layer != nullptr);

	CHECK(layer->has_warp() == false);
}


TEST_CASE("Warp: warp_style returns correct value for ARC warp")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Warp.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "WarpArc");
	REQUIRE(layer != nullptr);

	auto style = layer->warp_style();
	REQUIRE(style.has_value());
	CHECK((style.value() == TextLayerEnum::WarpStyle::Arc));
}


TEST_CASE("Warp: warp_style returns warpNone for non-warped layer")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Warp.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "Secondary");
	REQUIRE(layer != nullptr);

	auto style = layer->warp_style();
	REQUIRE(style.has_value());
	CHECK((style.value() == TextLayerEnum::WarpStyle::NoWarp));
}


TEST_CASE("Warp: warp_value returns bend amount")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Warp.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "WarpArc");
	REQUIRE(layer != nullptr);

	auto value = layer->warp_value();
	REQUIRE(value.has_value());
	// The fixture was generated with warpBend: 28
	CHECK(std::abs(value.value() - 28.0) < 0.01);
}


TEST_CASE("Warp: warp_value returns 0 for non-warped layer")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Warp.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "Secondary");
	REQUIRE(layer != nullptr);

	auto value = layer->warp_value();
	REQUIRE(value.has_value());
	CHECK(std::abs(value.value()) < 0.01);
}


TEST_CASE("Warp: warp_horizontal_distortion returns 0 for ARC fixture")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Warp.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "WarpArc");
	REQUIRE(layer != nullptr);

	auto hd = layer->warp_horizontal_distortion();
	REQUIRE(hd.has_value());
	CHECK(std::abs(hd.value()) < 0.01);
}


TEST_CASE("Warp: warp_vertical_distortion returns 0 for ARC fixture")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Warp.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "WarpArc");
	REQUIRE(layer != nullptr);

	auto vd = layer->warp_vertical_distortion();
	REQUIRE(vd.has_value());
	CHECK(std::abs(vd.value()) < 0.01);
}


TEST_CASE("Warp: warp_rotation returns 0 (Horizontal) for ARC fixture")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Warp.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "WarpArc");
	REQUIRE(layer != nullptr);

	auto rot = layer->warp_rotation();
	REQUIRE(rot.has_value());
	CHECK((rot.value() == TextLayerEnum::WarpRotation::Horizontal));
}


TEST_CASE("Warp: warp APIs still work after text roundtrip")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Warp.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "WarpArc");
	REQUIRE(layer != nullptr);

	// Verify text is readable
	auto text = layer->text();
	REQUIRE(text.has_value());
	CHECK(text.value() == "Warped Text");

	// Write and re-read
	auto tmp = std::filesystem::temp_directory_path() / "psapi_warp_roundtrip.psd";
	LayeredFile<bpp8_t>::write(std::move(file), tmp);
	auto reread = LayeredFile<bpp8_t>::read(tmp);
	std::filesystem::remove(tmp);

	auto rl = find_text_layer(reread, "WarpArc");
	REQUIRE(rl != nullptr);

	// Warp data should be preserved through roundtrip
	auto style = rl->warp_style();
	REQUIRE(style.has_value());
	CHECK((style.value() == TextLayerEnum::WarpStyle::Arc));

	auto value = rl->warp_value();
	REQUIRE(value.has_value());
	CHECK(std::abs(value.value() - 28.0) < 0.01);

	CHECK(rl->has_warp() == true);
}


TEST_CASE("Warp: set APIs mutate and survive roundtrip")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Warp.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "WarpArc");
	REQUIRE(layer != nullptr);

	layer->set_warp_style(TextLayerEnum::WarpStyle::Wave);
	layer->set_warp_value(-22.5);
	layer->set_warp_horizontal_distortion(15.0);
	layer->set_warp_vertical_distortion(-8.25);
	layer->set_warp_rotation(TextLayerEnum::WarpRotation::Vertical);

	auto style = layer->warp_style();
	REQUIRE(style.has_value());
	CHECK(style.value() == TextLayerEnum::WarpStyle::Wave);

	auto value = layer->warp_value();
	REQUIRE(value.has_value());
	CHECK(std::abs(value.value() - (-22.5)) < 0.01);

	auto hd = layer->warp_horizontal_distortion();
	REQUIRE(hd.has_value());
	CHECK(std::abs(hd.value() - 15.0) < 0.01);

	auto vd = layer->warp_vertical_distortion();
	REQUIRE(vd.has_value());
	CHECK(std::abs(vd.value() - (-8.25)) < 0.01);

	auto rotation = layer->warp_rotation();
	REQUIRE(rotation.has_value());
	CHECK(rotation.value() == TextLayerEnum::WarpRotation::Vertical);
	CHECK(layer->has_warp());

	auto tmp = std::filesystem::temp_directory_path() / "psapi_warp_set_roundtrip.psd";
	LayeredFile<bpp8_t>::write(std::move(file), tmp);
	auto reread = LayeredFile<bpp8_t>::read(tmp);
	std::filesystem::remove(tmp);

	auto rl = find_text_layer(reread, "WarpArc");
	REQUIRE(rl != nullptr);

	style = rl->warp_style();
	REQUIRE(style.has_value());
	CHECK(style.value() == TextLayerEnum::WarpStyle::Wave);

	value = rl->warp_value();
	REQUIRE(value.has_value());
	CHECK(std::abs(value.value() - (-22.5)) < 0.01);

	hd = rl->warp_horizontal_distortion();
	REQUIRE(hd.has_value());
	CHECK(std::abs(hd.value() - 15.0) < 0.01);

	vd = rl->warp_vertical_distortion();
	REQUIRE(vd.has_value());
	CHECK(std::abs(vd.value() - (-8.25)) < 0.01);

	rotation = rl->warp_rotation();
	REQUIRE(rotation.has_value());
	CHECK(rotation.value() == TextLayerEnum::WarpRotation::Vertical);
	CHECK(rl->has_warp());
}


TEST_CASE("Warp: Basic fixture layers have warpNone")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);

	for (auto& layer : file.flat_layers())
	{
		auto tl = std::dynamic_pointer_cast<TextLayer<bpp8_t>>(layer);
		if (!tl) continue;

		// All basic fixture layers should have no warp
		CHECK_FALSE(tl->has_warp());
		auto style = tl->warp_style();
		if (style.has_value())
		{
			CHECK((style.value() == TextLayerEnum::WarpStyle::NoWarp));
		}
	}
}
