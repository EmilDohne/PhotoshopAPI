#include "doctest.h"

#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"

#include <atomic>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace
{
	std::filesystem::path temp_psd_path()
	{
		static std::atomic<uint64_t> counter{ 0u };
		const auto stamp = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
		const auto idx = counter.fetch_add(1u, std::memory_order_relaxed);
		return std::filesystem::temp_directory_path() / ("psapi_text_builder_" + std::to_string(stamp) + "_" + std::to_string(idx) + ".psd");
	}

	template <typename T>
	std::shared_ptr<NAMESPACE_PSAPI::TextLayer<T>> find_text_layer_by_name(
		NAMESPACE_PSAPI::LayeredFile<T>& file,
		const std::string& name)
	{
		for (const auto& layer : file.flat_layers())
		{
			auto text_layer = std::dynamic_pointer_cast<NAMESPACE_PSAPI::TextLayer<T>>(layer);
			if (text_layer && text_layer->name() == name)
			{
				return text_layer;
			}
		}
		return nullptr;
	}
}


// =============================================================================
//  Basic create() factory
// =============================================================================

TEST_CASE("TextLayer::create produces a layer with correct text")
{
	using namespace NAMESPACE_PSAPI;

	auto layer = TextLayer<bpp8_t>::create("TestLayer", "Hello World");
	REQUIRE(layer != nullptr);
	CHECK(layer->name() == "TestLayer");

	auto text = layer->text();
	REQUIRE(text.has_value());
	// create() appends trailing \r internally
	CHECK(text.value() == "Hello World");
}


TEST_CASE("TextLayer::create with default parameters produces valid single run")
{
	using namespace NAMESPACE_PSAPI;

	auto layer = TextLayer<bpp8_t>::create("Defaults", "ABCDEF");

	auto lengths = layer->style_run_lengths();
	REQUIRE(lengths.has_value());
	// 6 chars + trailing \r = 7
	REQUIRE(lengths->size() == 1u);
	CHECK(lengths->at(0) == 7);

	CHECK(layer->style_run_count() == 1u);
	auto fs = layer->style_run_font_size(0u);
	REQUIRE(fs.has_value());
	CHECK(doctest::Approx(fs.value()).epsilon(0.01) == 24.0);  // default font size

	auto fill = layer->style_run_fill_color(0u);
	REQUIRE(fill.has_value());
	REQUIRE(fill->size() == 4u);
	CHECK(doctest::Approx((*fill)[0]).epsilon(0.001) == 1.0);  // alpha
	CHECK(doctest::Approx((*fill)[1]).epsilon(0.001) == 0.0);  // R
	CHECK(doctest::Approx((*fill)[2]).epsilon(0.001) == 0.0);  // G
	CHECK(doctest::Approx((*fill)[3]).epsilon(0.001) == 0.0);  // B
}


TEST_CASE("TextLayer::create with custom font size and fill color")
{
	using namespace NAMESPACE_PSAPI;

	auto layer = TextLayer<bpp8_t>::create(
		"Custom", "Test",
		"ArialMT", 36.0,
		{ 1.0, 0.5, 0.3, 0.8 });

	auto fs = layer->style_run_font_size(0u);
	REQUIRE(fs.has_value());
	CHECK(doctest::Approx(fs.value()).epsilon(0.01) == 36.0);

	auto fill = layer->style_run_fill_color(0u);
	REQUIRE(fill.has_value());
	REQUIRE(fill->size() == 4u);
	CHECK(doctest::Approx((*fill)[0]).epsilon(0.001) == 1.0);
	CHECK(doctest::Approx((*fill)[1]).epsilon(0.001) == 0.5);
	CHECK(doctest::Approx((*fill)[2]).epsilon(0.001) == 0.3);
	CHECK(doctest::Approx((*fill)[3]).epsilon(0.001) == 0.8);
}


// =============================================================================
//  Newline handling
// =============================================================================

TEST_CASE("TextLayer::create converts newlines to carriage returns")
{
	using namespace NAMESPACE_PSAPI;

	auto layer = TextLayer<bpp8_t>::create("Multiline", "Line1\nLine2\nLine3");

	auto text = layer->text();
	REQUIRE(text.has_value());
	// \n should be converted to \r in the EngineData
	CHECK(text.value() == "Line1\rLine2\rLine3");
}


// =============================================================================
//  Write/read roundtrip
// =============================================================================

TEST_CASE("TextLayer::create roundtrips through file write and read")
{
	using namespace NAMESPACE_PSAPI;

	auto doc = LayeredFile<bpp8_t>(Enum::ColorMode::RGB, 800, 600);
	auto layer = TextLayer<bpp8_t>::create("RoundTrip", "Hello World", "ArialMT", 28.0);

	doc.add_layer(layer);

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(doc), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_by_name(reread, std::string("RoundTrip"));
	REQUIRE(reread_layer != nullptr);

	auto text = reread_layer->text();
	REQUIRE(text.has_value());
	CHECK(text.value() == "Hello World");

	auto fs = reread_layer->style_run_font_size(0u);
	REQUIRE(fs.has_value());
	CHECK(doctest::Approx(fs.value()).epsilon(0.01) == 28.0);

	std::filesystem::remove(out_path);
}


TEST_CASE("TextLayer::create multiline roundtrips correctly")
{
	using namespace NAMESPACE_PSAPI;

	auto doc = LayeredFile<bpp8_t>(Enum::ColorMode::RGB, 800, 600);
	auto layer = TextLayer<bpp8_t>::create("MultiRT", "First\nSecond\nThird", "ArialMT", 20.0);
	doc.add_layer(layer);

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(doc), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_by_name(reread, std::string("MultiRT"));
	REQUIRE(reread_layer != nullptr);

	auto text = reread_layer->text();
	REQUIRE(text.has_value());
	CHECK(text.value() == "First\rSecond\rThird");

	std::filesystem::remove(out_path);
}


// =============================================================================
//  Style run splitting
// =============================================================================

TEST_CASE("TextLayer::create then split_style_run produces correct run lengths")
{
	using namespace NAMESPACE_PSAPI;

	// "Hello Bold World\r" → 17 code units
	auto layer = TextLayer<bpp8_t>::create("Split", "Hello Bold World");

	auto lengths = layer->style_run_lengths();
	REQUIRE(lengths.has_value());
	REQUIRE(lengths->size() == 1u);
	CHECK(lengths->at(0) == 17);  // 16 chars + trailing \r

	// Split at offset 6 → "Hello " | "Bold World\r"
	CHECK_NOTHROW(layer->split_style_run(0, 6));
	lengths = layer->style_run_lengths();
	REQUIRE(lengths.has_value());
	REQUIRE(lengths->size() == 2u);
	CHECK(lengths->at(0) == 6);
	CHECK(lengths->at(1) == 11);

	// Split "Bold World\r" at offset 4 → "Bold" | " World\r"
	CHECK_NOTHROW(layer->split_style_run(1, 4));
	lengths = layer->style_run_lengths();
	REQUIRE(lengths.has_value());
	REQUIRE(lengths->size() == 3u);
	CHECK(lengths->at(0) == 6);
	CHECK(lengths->at(1) == 4);
	CHECK(lengths->at(2) == 7);
}


TEST_CASE("split_style_run with invalid run index returns false")
{
	using namespace NAMESPACE_PSAPI;

	auto layer = TextLayer<bpp8_t>::create("Invalid", "Short");
	CHECK_THROWS(layer->split_style_run(5, 2));  // run index 5 doesn't exist
}


TEST_CASE("split_style_run with zero offset returns false")
{
	using namespace NAMESPACE_PSAPI;

	auto layer = TextLayer<bpp8_t>::create("ZeroOff", "Hello");
	CHECK_THROWS(layer->split_style_run(0, 0));  // can't split at start
}


// =============================================================================
//  Style run mutation after split
// =============================================================================

TEST_CASE("Style properties can be set on split runs and roundtrip")
{
	using namespace NAMESPACE_PSAPI;

	auto layer = TextLayer<bpp8_t>::create("StyleMut", "Hello Bold World", "ArialMT", 28.0);

	// Split into: "Hello " (6) | "Bold" (4) | " World\r" (7)
	CHECK_NOTHROW(layer->split_style_run(0, 6));
	CHECK_NOTHROW(layer->split_style_run(1, 4));

	// Modify run 1 ("Bold")
	CHECK_NOTHROW(layer->set_style_run_faux_bold(1, true));
	CHECK_NOTHROW(layer->set_style_run_font_size(1, 32.0));
	CHECK_NOTHROW(layer->set_style_run_fill_color(1, { 1.0, 1.0, 0.0, 0.0 }));  // red

	// Modify run 2 (" World\r")
	CHECK_NOTHROW(layer->set_style_run_faux_italic(2, true));
	CHECK_NOTHROW(layer->set_style_run_underline(2, true));

	// Verify in-memory
	auto bold = layer->style_run_faux_bold(1);
	REQUIRE(bold.has_value());
	CHECK(bold.value() == true);

	auto italic = layer->style_run_faux_italic(2);
	REQUIRE(italic.has_value());
	CHECK(italic.value() == true);

	auto underline = layer->style_run_underline(2);
	REQUIRE(underline.has_value());
	CHECK(underline.value() == true);

	// Roundtrip
	auto doc = LayeredFile<bpp8_t>(Enum::ColorMode::RGB, 800, 600);
	doc.add_layer(layer);

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(doc), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_by_name(reread, std::string("StyleMut"));
	REQUIRE(reread_layer != nullptr);

	// Check run structure
	auto rlens = reread_layer->style_run_lengths();
	REQUIRE(rlens.has_value());
	REQUIRE(rlens->size() == 3u);
	CHECK(rlens->at(0) == 6);
	CHECK(rlens->at(1) == 4);
	CHECK(rlens->at(2) == 7);

	// Check run 1 ("Bold") properties
	auto r1_bold = reread_layer->style_run_faux_bold(1);
	REQUIRE(r1_bold.has_value());
	CHECK(r1_bold.value() == true);

	auto r1_fs = reread_layer->style_run_font_size(1);
	REQUIRE(r1_fs.has_value());
	CHECK(doctest::Approx(r1_fs.value()).epsilon(0.01) == 32.0);

	auto r1_fill = reread_layer->style_run_fill_color(1);
	REQUIRE(r1_fill.has_value());
	REQUIRE(r1_fill->size() == 4u);
	CHECK(doctest::Approx((*r1_fill)[1]).epsilon(0.001) == 1.0);  // red=1.0

	// Check run 2 (" World\r") properties
	auto r2_italic = reread_layer->style_run_faux_italic(2);
	REQUIRE(r2_italic.has_value());
	CHECK(r2_italic.value() == true);

	auto r2_under = reread_layer->style_run_underline(2);
	REQUIRE(r2_under.has_value());
	CHECK(r2_under.value() == true);

	// Check run 0 ("Hello ") retains original settings (not bold)
	auto r0_bold = reread_layer->style_run_faux_bold(0);
	REQUIRE(r0_bold.has_value());
	CHECK(r0_bold.value() == false);

	std::filesystem::remove(out_path);
}


// =============================================================================
//  Stroke properties
// =============================================================================

TEST_CASE("Stroke flag, color, and outline width roundtrip on created layer")
{
	using namespace NAMESPACE_PSAPI;

	auto layer = TextLayer<bpp8_t>::create("Stroke", "Outlined Text", "ArialMT", 28.0);

	CHECK_NOTHROW(layer->set_style_run_stroke_flag(0, true));
	CHECK_NOTHROW(layer->set_style_run_stroke_color(0, { 1.0, 0.0, 0.0, 1.0 }));  // blue
	CHECK_NOTHROW(layer->set_style_run_outline_width(0, 3.0));

	// Verify in-memory
	auto sflag = layer->style_run_stroke_flag(0);
	REQUIRE(sflag.has_value());
	CHECK(sflag.value() == true);

	// Roundtrip
	auto doc = LayeredFile<bpp8_t>(Enum::ColorMode::RGB, 800, 600);
	doc.add_layer(layer);

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(doc), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_by_name(reread, std::string("Stroke"));
	REQUIRE(reread_layer != nullptr);

	auto r_sflag = reread_layer->style_run_stroke_flag(0);
	REQUIRE(r_sflag.has_value());
	CHECK(r_sflag.value() == true);

	auto r_scol = reread_layer->style_run_stroke_color(0);
	REQUIRE(r_scol.has_value());
	REQUIRE(r_scol->size() == 4u);
	CHECK(doctest::Approx((*r_scol)[0]).epsilon(0.001) == 1.0);
	CHECK(doctest::Approx((*r_scol)[1]).epsilon(0.001) == 0.0);
	CHECK(doctest::Approx((*r_scol)[2]).epsilon(0.001) == 0.0);
	CHECK(doctest::Approx((*r_scol)[3]).epsilon(0.001) == 1.0);

	auto r_ow = reread_layer->style_run_outline_width(0);
	REQUIRE(r_ow.has_value());
	CHECK(doctest::Approx(r_ow.value()).epsilon(0.01) == 3.0);

	std::filesystem::remove(out_path);
}


TEST_CASE("FillFirst defaults to false on created layers")
{
	using namespace NAMESPACE_PSAPI;

	auto layer = TextLayer<bpp8_t>::create("FillFirstCheck", "Test");

	auto ff = layer->style_run_fill_first(0);
	REQUIRE(ff.has_value());
	CHECK(ff.value() == false);
}


// =============================================================================
//  Font size type preservation (float stays float, not integer)
// =============================================================================

TEST_CASE("Font size remains float after set_style_run_font_size with whole number")
{
	using namespace NAMESPACE_PSAPI;

	auto layer = TextLayer<bpp8_t>::create("FontFloat", "Test", "ArialMT", 28.0);

	// Set a whole-number font size
	CHECK_NOTHROW(layer->set_style_run_font_size(0, 20.0));

	auto fs = layer->style_run_font_size(0);
	REQUIRE(fs.has_value());
	CHECK(doctest::Approx(fs.value()).epsilon(0.01) == 20.0);

	// Roundtrip to verify the float type is preserved in EngineData
	auto doc = LayeredFile<bpp8_t>(Enum::ColorMode::RGB, 800, 600);
	doc.add_layer(layer);

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(doc), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_by_name(reread, std::string("FontFloat"));
	REQUIRE(reread_layer != nullptr);

	auto rt_fs = reread_layer->style_run_font_size(0);
	REQUIRE(rt_fs.has_value());
	CHECK(doctest::Approx(rt_fs.value()).epsilon(0.01) == 20.0);

	std::filesystem::remove(out_path);
}


// =============================================================================
//  Multiple layers from scratch in one document
// =============================================================================

TEST_CASE("Multiple created TextLayers coexist in a single document")
{
	using namespace NAMESPACE_PSAPI;

	auto doc = LayeredFile<bpp8_t>(Enum::ColorMode::RGB, 800, 600);

	auto layer1 = TextLayer<bpp8_t>::create("Layer1", "First", "ArialMT", 24.0);
	auto layer2 = TextLayer<bpp8_t>::create("Layer2", "Second", "ArialMT", 32.0);
	auto layer3 = TextLayer<bpp8_t>::create("Layer3", "Third", "ArialMT", 16.0);

	doc.add_layer(layer1);
	doc.add_layer(layer2);
	doc.add_layer(layer3);

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(doc), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);

	auto r1 = find_text_layer_by_name(reread, std::string("Layer1"));
	auto r2 = find_text_layer_by_name(reread, std::string("Layer2"));
	auto r3 = find_text_layer_by_name(reread, std::string("Layer3"));

	REQUIRE(r1 != nullptr);
	REQUIRE(r2 != nullptr);
	REQUIRE(r3 != nullptr);

	CHECK(r1->text().value() == "First");
	CHECK(r2->text().value() == "Second");
	CHECK(r3->text().value() == "Third");

	auto fs1 = r1->style_run_font_size(0);
	auto fs2 = r2->style_run_font_size(0);
	auto fs3 = r3->style_run_font_size(0);
	REQUIRE(fs1.has_value());
	REQUIRE(fs2.has_value());
	REQUIRE(fs3.has_value());
	CHECK(doctest::Approx(fs1.value()).epsilon(0.01) == 24.0);
	CHECK(doctest::Approx(fs2.value()).epsilon(0.01) == 32.0);
	CHECK(doctest::Approx(fs3.value()).epsilon(0.01) == 16.0);

	std::filesystem::remove(out_path);
}


// =============================================================================
//  Complex multi-run styled layer roundtrip
// =============================================================================

TEST_CASE("Complex multi-run styled layer roundtrips all properties")
{
	using namespace NAMESPACE_PSAPI;

	// Mimics the demo_text_from_scratch script
	auto layer = TextLayer<bpp8_t>::create(
		"Complex", "Hello Bold World\nUnderline here",
		"ArialMT", 28.0, { 1.0, 0.0, 0.0, 0.0 });

	// Split into 5 runs:
	//   run 0: "Hello "       (6)
	//   run 1: "Bold"         (4)
	//   run 2: " World\r"     (7)  — \n converted to \r
	//   run 3: "Underline"    (9)
	//   run 4: " here\r"      (6)
	CHECK_NOTHROW(layer->split_style_run(0, 6));
	CHECK_NOTHROW(layer->split_style_run(1, 4));
	CHECK_NOTHROW(layer->split_style_run(2, 7));
	CHECK_NOTHROW(layer->split_style_run(3, 9));

	auto lengths = layer->style_run_lengths();
	REQUIRE(lengths.has_value());
	REQUIRE(lengths->size() == 5u);
	CHECK(lengths->at(0) == 6);
	CHECK(lengths->at(1) == 4);
	CHECK(lengths->at(2) == 7);
	CHECK(lengths->at(3) == 9);
	CHECK(lengths->at(4) == 6);

	// Style run 1: bold
	CHECK_NOTHROW(layer->set_style_run_faux_bold(1, true));

	// Style run 2: red + blue stroke
	CHECK_NOTHROW(layer->set_style_run_fill_color(2, { 1.0, 1.0, 0.0, 0.0 }));
	CHECK_NOTHROW(layer->set_style_run_stroke_flag(2, true));
	CHECK_NOTHROW(layer->set_style_run_stroke_color(2, { 1.0, 0.0, 0.0, 1.0 }));
	CHECK_NOTHROW(layer->set_style_run_outline_width(2, 2.0));

	// Style run 3: underline
	CHECK_NOTHROW(layer->set_style_run_underline(3, true));

	// Style run 4: italic + different size
	CHECK_NOTHROW(layer->set_style_run_faux_italic(4, true));
	CHECK_NOTHROW(layer->set_style_run_font_size(4, 20.0));

	// Roundtrip
	auto doc = LayeredFile<bpp8_t>(Enum::ColorMode::RGB, 800, 600);
	doc.add_layer(layer);

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(doc), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto rl = find_text_layer_by_name(reread, std::string("Complex"));
	REQUIRE(rl != nullptr);

	// Text and run structure
	auto text = rl->text();
	REQUIRE(text.has_value());
	CHECK(text.value() == "Hello Bold World\rUnderline here");

	auto rlens = rl->style_run_lengths();
	REQUIRE(rlens.has_value());
	REQUIRE(rlens->size() == 5u);
	CHECK(rlens->at(0) == 6);
	CHECK(rlens->at(1) == 4);
	CHECK(rlens->at(2) == 7);
	CHECK(rlens->at(3) == 9);
	CHECK(rlens->at(4) == 6);

	// Run 0: default (not bold, not italic)
	CHECK(rl->style_run_faux_bold(0).value_or(true) == false);
	CHECK(rl->style_run_faux_italic(0).value_or(true) == false);

	// Run 1: bold
	CHECK(rl->style_run_faux_bold(1).value_or(false) == true);

	// Run 2: red fill, stroke enabled, blue stroke
	auto r2_fill = rl->style_run_fill_color(2);
	REQUIRE(r2_fill.has_value());
	CHECK(doctest::Approx((*r2_fill)[1]).epsilon(0.001) == 1.0);  // red
	CHECK(rl->style_run_stroke_flag(2).value_or(false) == true);
	auto r2_sc = rl->style_run_stroke_color(2);
	REQUIRE(r2_sc.has_value());
	CHECK(doctest::Approx((*r2_sc)[3]).epsilon(0.001) == 1.0);  // blue

	auto r2_ow = rl->style_run_outline_width(2);
	REQUIRE(r2_ow.has_value());
	CHECK(doctest::Approx(r2_ow.value()).epsilon(0.01) == 2.0);

	// Run 3: underline
	CHECK(rl->style_run_underline(3).value_or(false) == true);

	// Run 4: italic, font size 20
	CHECK(rl->style_run_faux_italic(4).value_or(false) == true);
	auto r4_fs = rl->style_run_font_size(4);
	REQUIRE(r4_fs.has_value());
	CHECK(doctest::Approx(r4_fs.value()).epsilon(0.01) == 20.0);

	std::filesystem::remove(out_path);
}


// =============================================================================
//  Edge cases
// =============================================================================

TEST_CASE("TextLayer::create with single character")
{
	using namespace NAMESPACE_PSAPI;

	auto layer = TextLayer<bpp8_t>::create("Single", "X");
	auto text = layer->text();
	REQUIRE(text.has_value());
	CHECK(text.value() == "X");

	auto lengths = layer->style_run_lengths();
	REQUIRE(lengths.has_value());
	CHECK(lengths->at(0) == 2);  // "X" + trailing \r
}


TEST_CASE("TextLayer::create with box_width and box_height roundtrips")
{
	using namespace NAMESPACE_PSAPI;

	auto doc = LayeredFile<bpp8_t>(Enum::ColorMode::RGB, 800, 600);
	auto layer = TextLayer<bpp8_t>::create(
		"BoxSize", "Box text here",
		"ArialMT", 24.0,
		{ 1.0, 0.0, 0.0, 0.0 },
		20.0, 50.0,   // position
		500.0, 200.0); // explicit box size

	doc.add_layer(layer);

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(doc), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_by_name(reread, std::string("BoxSize"));
	REQUIRE(reread_layer != nullptr);

	auto text = reread_layer->text();
	REQUIRE(text.has_value());
	CHECK(text.value() == "Box text here");

	std::filesystem::remove(out_path);
}


TEST_CASE("TextLayer::create 16bit variant roundtrips")
{
	using namespace NAMESPACE_PSAPI;

	auto doc = LayeredFile<bpp16_t>(Enum::ColorMode::RGB, 400, 300);
	auto layer = TextLayer<bpp16_t>::create("Layer16", "Sixteen Bit");

	doc.add_layer(layer);

	const auto out_path = temp_psd_path();
	LayeredFile<bpp16_t>::write(std::move(doc), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp16_t>::read(out_path);
	auto rl = find_text_layer_by_name(reread, std::string("Layer16"));
	REQUIRE(rl != nullptr);

	CHECK(rl->text().value() == "Sixteen Bit");

	std::filesystem::remove(out_path);
}


TEST_CASE("TextLayer::create 32bit variant roundtrips")
{
	using namespace NAMESPACE_PSAPI;

	auto doc = LayeredFile<bpp32_t>(Enum::ColorMode::RGB, 400, 300);
	auto layer = TextLayer<bpp32_t>::create("Layer32", "ThirtyTwo Bit");

	doc.add_layer(layer);

	const auto out_path = temp_psd_path();
	LayeredFile<bpp32_t>::write(std::move(doc), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp32_t>::read(out_path);
	auto rl = find_text_layer_by_name(reread, std::string("Layer32"));
	REQUIRE(rl != nullptr);

	CHECK(rl->text().value() == "ThirtyTwo Bit");

	std::filesystem::remove(out_path);
}
