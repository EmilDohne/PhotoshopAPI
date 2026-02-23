#include "doctest.h"
#include "TestTextLayerMutationUtils.h"

// ============================================================
// FontSet read APIs
// ============================================================

TEST_CASE("TextLayer font_count returns the number of fonts in FontSet")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer != nullptr);

	// The style runs fixture has at least 3 fonts; the exact count depends on the document
	CHECK(text_layer->font_count() >= 3u);
}

TEST_CASE("TextLayer font_postscript_name returns correct PostScript names")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer != nullptr);

	const auto count = text_layer->font_count();
	REQUIRE(count >= 1u);

	// Every font should have a non-empty PostScript name
	for (size_t i = 0u; i < count; ++i)
	{
		const auto name = text_layer->font_postscript_name(i);
		REQUIRE(name.has_value());
		CHECK_FALSE(name.value().empty());
	}

	// font_name is an alias for font_postscript_name
	const auto name0 = text_layer->font_postscript_name(0);
	CHECK(text_layer->font_name(0).value() == name0.value());

	// Verify the AdobeInvisFont is present somewhere (Photoshop always adds it)
	bool found_invis = false;
	for (size_t i = 0u; i < count; ++i)
	{
		if (text_layer->font_postscript_name(i).value() == "AdobeInvisFont")
		{
			found_invis = true;
			break;
		}
	}
	CHECK(found_invis);
}

TEST_CASE("TextLayer font_postscript_name returns nullopt for out-of-range index")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer != nullptr);

	CHECK_FALSE(text_layer->font_postscript_name(100).has_value());
}

TEST_CASE("TextLayer font_script returns script codes for each font")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer != nullptr);

	const auto script0 = text_layer->font_script(0);
	REQUIRE(script0.has_value());
	CHECK(script0.value() == TextLayerEnum::FontScript::Roman);
}

TEST_CASE("TextLayer font_type returns font type codes for each font")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer != nullptr);

	const auto count = text_layer->font_count();
	REQUIRE(count >= 1u);

	// Every font should have a valid font type (0 or 1)
	for (size_t i = 0u; i < count; ++i)
	{
		const auto ft = text_layer->font_type(i);
		REQUIRE(ft.has_value());
		CHECK((ft.value() == TextLayerEnum::FontType::OpenType || ft.value() == TextLayerEnum::FontType::TrueType));
	}
}

TEST_CASE("TextLayer font_synthetic returns synthetic flag for each font")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer != nullptr);

	const auto synthetic0 = text_layer->font_synthetic(0);
	REQUIRE(synthetic0.has_value());
	CHECK(synthetic0.value() >= 0);
}

TEST_CASE("TextLayer font index from style_run_font maps to font_postscript_name")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer != nullptr);

	// Get the font index used by style run 0
	const auto run_font_idx = text_layer->style_run_font(0);
	REQUIRE(run_font_idx.has_value());

	// Resolve the index to a name
	const auto psname = text_layer->font_postscript_name(static_cast<size_t>(run_font_idx.value()));
	REQUIRE(psname.has_value());

	// The name should be one of the known fonts
	CHECK((psname.value() == "ArialMT" || psname.value() == "MyriadPro-Regular" || psname.value() == "AdobeInvisFont"));
}

// ---------------------------------------------------------------------------
//  Orientation / WritingDirection tests
// ---------------------------------------------------------------------------

TEST_CASE("TextLayer orientation returns 0 for horizontal text")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer != nullptr);

	const auto wd = text_layer->orientation();
	REQUIRE(wd.has_value());
	CHECK(wd.value() == TextLayerEnum::WritingDirection::Horizontal);
	CHECK_FALSE(text_layer->is_vertical());
}

TEST_CASE("TextLayer orientation returns 2 for vertical text")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Vertical.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "VERTICAL");
	REQUIRE(text_layer != nullptr);

	const auto wd = text_layer->orientation();
	REQUIRE(wd.has_value());
	CHECK(wd.value() == TextLayerEnum::WritingDirection::Vertical);
	CHECK(text_layer->is_vertical());
}

TEST_CASE("TextLayer set_orientation changes horizontal to vertical")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer != nullptr);

	// Starts horizontal
	REQUIRE(text_layer->orientation().value() == TextLayerEnum::WritingDirection::Horizontal);

	// Change to vertical
	CHECK_NOTHROW(text_layer->set_orientation(TextLayerEnum::WritingDirection::Vertical));

	// Verify in-memory
	CHECK(text_layer->orientation().value() == TextLayerEnum::WritingDirection::Vertical);
	CHECK(text_layer->is_vertical());
}

TEST_CASE("TextLayer set_orientation writes Procession=1 for vertical text")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer != nullptr);
	REQUIRE(text_layer->orientation().value() == TextLayerEnum::WritingDirection::Horizontal);

	CHECK_NOTHROW(text_layer->set_orientation(TextLayerEnum::WritingDirection::Vertical));

	auto [record, _channel_data] = text_layer->to_photoshop();
	REQUIRE(record.m_AdditionalLayerInfo.has_value());
	const auto tysh = record.m_AdditionalLayerInfo->getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrTypeTool);
	REQUIRE(tysh.has_value());

	const auto procession = extract_engine_int_value(*tysh.value(), "/Procession");
	REQUIRE(procession.has_value());
	CHECK(procession.value() == 1);
}

TEST_CASE("TextLayer set_orientation changes vertical to horizontal")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Vertical.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "VERTICAL");
	REQUIRE(text_layer != nullptr);

	// Starts vertical
	REQUIRE(text_layer->orientation().value() == TextLayerEnum::WritingDirection::Vertical);

	// Change to horizontal
	CHECK_NOTHROW(text_layer->set_orientation(TextLayerEnum::WritingDirection::Horizontal));

	// Verify in-memory
	CHECK(text_layer->orientation().value() == TextLayerEnum::WritingDirection::Horizontal);
	CHECK_FALSE(text_layer->is_vertical());
}

TEST_CASE("TextLayer set_orientation writes Procession=0 for horizontal text")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Vertical.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "VERTICAL");
	REQUIRE(text_layer != nullptr);
	REQUIRE(text_layer->orientation().value() == TextLayerEnum::WritingDirection::Vertical);

	CHECK_NOTHROW(text_layer->set_orientation(TextLayerEnum::WritingDirection::Horizontal));

	auto [record, _channel_data] = text_layer->to_photoshop();
	REQUIRE(record.m_AdditionalLayerInfo.has_value());
	const auto tysh = record.m_AdditionalLayerInfo->getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrTypeTool);
	REQUIRE(tysh.has_value());

	const auto procession = extract_engine_int_value(*tysh.value(), "/Procession");
	REQUIRE(procession.has_value());
	CHECK(procession.value() == 0);
}

TEST_CASE("TextLayer is_vertical false for Control layer in Vertical fixture")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Vertical.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Control");
	REQUIRE(text_layer != nullptr);

	CHECK(text_layer->orientation().value() == TextLayerEnum::WritingDirection::Horizontal);
	CHECK_FALSE(text_layer->is_vertical());
}

// ---------------------------------------------------------------------------
//  Missing-font query tests
// ---------------------------------------------------------------------------

TEST_CASE("TextLayer used_font_indices returns sorted unique indices from style runs")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer != nullptr);

	const auto indices = text_layer->used_font_indices();
	// Must have at least 1 entry
	CHECK(indices.size() >= 1u);

	// Must be sorted ascending
	for (size_t i = 1u; i < indices.size(); ++i)
	{
		CHECK(indices[i] > indices[i - 1u]);
	}

	// All indices must be valid (< font_count)
	const auto fc = text_layer->font_count();
	for (const auto idx : indices)
	{
		CHECK(idx < fc);
	}
}

TEST_CASE("TextLayer used_font_names returns real font names excluding AdobeInvisFont")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer != nullptr);

	const auto names = text_layer->used_font_names();
	CHECK(names.size() >= 1u);

	// None of the returned names should be AdobeInvisFont
	for (const auto& name : names)
	{
		CHECK(name != "AdobeInvisFont");
		CHECK_FALSE(name.empty());
	}
}

TEST_CASE("TextLayer is_sentinel_font detects AdobeInvisFont")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_FontFallback.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Fallback Probe");
	REQUIRE(text_layer != nullptr);

	const auto fc = text_layer->font_count();
	REQUIRE(fc == 2u);

	// font[0] = ZZZMissingFontTok -> not sentinel
	CHECK_FALSE(text_layer->is_sentinel_font(0));

	// font[1] = AdobeInvisFont -> sentinel
	CHECK(text_layer->is_sentinel_font(1));
}

TEST_CASE("TextLayer used_font_names from FontFallback contains the patched font name")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_FontFallback.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Fallback Probe");
	REQUIRE(text_layer != nullptr);

	const auto names = text_layer->used_font_names();
	REQUIRE(names.size() >= 1u);

	// The patched bogus font name should be in the list
	bool found_missing = false;
	for (const auto& name : names)
	{
		if (name == "ZZZMissingFontTok")
		{
			found_missing = true;
		}
	}
	CHECK(found_missing);
}

TEST_CASE("TextLayer used_font_indices from Basic fixture references valid fonts")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer != nullptr);

	const auto indices = text_layer->used_font_indices();
	CHECK(indices.size() >= 1u);

	// Resolve each index to a name - all should be non-empty
	for (const auto idx : indices)
	{
		const auto name = text_layer->font_postscript_name(idx);
		REQUIRE(name.has_value());
		CHECK_FALSE(name.value().empty());
	}
}

// ---------------------------------------------------------------------------
//  FontSet write / sync tests
// ---------------------------------------------------------------------------

TEST_CASE("TextLayer add_font appends a new font entry and increments font_count")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer != nullptr);

	const auto before_count = text_layer->font_count();
	REQUIRE(before_count >= 1u);

	const auto new_index = text_layer->add_font("TestNewFont-Bold", TextLayerEnum::FontType::TrueType, TextLayerEnum::FontScript::Roman);
	REQUIRE(new_index >= 0);
	CHECK(static_cast<size_t>(new_index) == before_count);
	CHECK(text_layer->font_count() == before_count + 1u);

	// Verify the name round-trips
	const auto name = text_layer->font_postscript_name(static_cast<size_t>(new_index));
	REQUIRE(name.has_value());
	CHECK(name.value() == "TestNewFont-Bold");

	// Verify the other properties
	const auto ftype = text_layer->font_type(static_cast<size_t>(new_index));
	REQUIRE(ftype.has_value());
	CHECK(ftype.value() == TextLayerEnum::FontType::TrueType);

	const auto fscript = text_layer->font_script(static_cast<size_t>(new_index));
	REQUIRE(fscript.has_value());
	CHECK(fscript.value() == TextLayerEnum::FontScript::Roman);

	const auto fsynth = text_layer->font_synthetic(static_cast<size_t>(new_index));
	REQUIRE(fsynth.has_value());
	CHECK(fsynth.value() == 0);
}

TEST_CASE("TextLayer set_font_postscript_name renames an existing font")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer != nullptr);

	const auto count = text_layer->font_count();
	REQUIRE(count >= 1u);

	// Remember original name
	const auto original = text_layer->font_postscript_name(0);
	REQUIRE(original.has_value());

	// Rename
	CHECK_NOTHROW(text_layer->set_font_postscript_name(0, "ReplacedFont-Regular"));

	// Verify
	const auto renamed = text_layer->font_postscript_name(0);
	REQUIRE(renamed.has_value());
	CHECK(renamed.value() == "ReplacedFont-Regular");

	// Count should be unchanged
	CHECK(text_layer->font_count() == count);
}

TEST_CASE("TextLayer set_font_postscript_name out of range returns false")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer != nullptr);

	CHECK_THROWS(text_layer->set_font_postscript_name(9999, "NoSuchFont"));
}

TEST_CASE("TextLayer add_font then use the new font in a style run")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer != nullptr);

	// Add new font
	const auto new_idx = text_layer->add_font("CustomFont-Italic", TextLayerEnum::FontType::OpenType);
	REQUIRE(new_idx >= 0);

	// Point style run 0 at the new font
	CHECK_NOTHROW(text_layer->set_style_run_font(0, new_idx));

	// Verify
	const auto run_font = text_layer->style_run_font(0);
	REQUIRE(run_font.has_value());
	CHECK(run_font.value() == new_idx);

	// The used_font_names should now include the new font
	const auto names = text_layer->used_font_names();
	bool found = false;
	for (const auto& n : names)
	{
		if (n == "CustomFont-Italic")
		{
			found = true;
		}
	}
	CHECK(found);
}

TEST_CASE("TextLayer add_font with synthetic parameter")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer != nullptr);

	const auto new_idx = text_layer->add_font("SynthFont-Bold", TextLayerEnum::FontType::TrueType, TextLayerEnum::FontScript::Roman, /*synthetic=*/1);
	REQUIRE(new_idx >= 0);

	const auto fsynth = text_layer->font_synthetic(static_cast<size_t>(new_idx));
	REQUIRE(fsynth.has_value());
	CHECK(fsynth.value() == 1);
}

TEST_CASE("TextLayer find_font_index returns correct index for existing font")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer != nullptr);

	// Font 0 should exist
	const auto name0 = text_layer->font_postscript_name(0);
	REQUIRE(name0.has_value());

	// find_font_index should return 0
	CHECK(text_layer->find_font_index(name0.value()) == 0);

	// Non-existent font should return -1
	CHECK(text_layer->find_font_index("NoSuchFont-12345") == -1);
}

TEST_CASE("TextLayer set_style_run_font_by_name with existing font")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer != nullptr);

	// Get the name of font at index 0 - this is already in the FontSet
	const auto existing_name = text_layer->font_postscript_name(0);
	REQUIRE(existing_name.has_value());
	const auto count_before = text_layer->font_count();

	// set_style_run_font_by_name should find the existing font, not add a new one
	CHECK_NOTHROW(text_layer->set_style_run_font_by_name(0, existing_name.value()));

	// Count should be unchanged since the font already existed
	CHECK(text_layer->font_count() == count_before);

	// Verify the run is now pointing at font 0
	const auto run_font = text_layer->style_run_font(0);
	REQUIRE(run_font.has_value());
	CHECK(run_font.value() == 0);
}

TEST_CASE("TextLayer set_style_run_font_by_name adds new font when not found")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer != nullptr);

	const auto count_before = text_layer->font_count();

	// Use a font name not in the set - should add it
	CHECK_NOTHROW(text_layer->set_style_run_font_by_name(0, "BrandNewFont-Regular"));

	// Count should be incremented
	CHECK(text_layer->font_count() == count_before + 1u);

	// Run 0 should now point at the newly-added index
	const auto run_font = text_layer->style_run_font(0);
	REQUIRE(run_font.has_value());
	CHECK(run_font.value() == static_cast<int32_t>(count_before));

	// The name should be retrievable
	const auto name = text_layer->font_postscript_name(static_cast<size_t>(run_font.value()));
	REQUIRE(name.has_value());
	CHECK(name.value() == "BrandNewFont-Regular");
}

TEST_CASE("TextLayer set_style_normal_font_by_name sets the normal sheet font")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer != nullptr);

	const auto count_before = text_layer->font_count();

	// Set normal font to a new name
	CHECK_NOTHROW(text_layer->set_style_normal_font_by_name("NormalNewFont-Light"));

	// Font should have been added
	CHECK(text_layer->font_count() == count_before + 1u);

	// Normal font should be the new index
	const auto normal_font = text_layer->style_normal_font();
	REQUIRE(normal_font.has_value());
	CHECK(normal_font.value() == static_cast<int32_t>(count_before));

	// Verify font name
	const auto name = text_layer->font_postscript_name(static_cast<size_t>(normal_font.value()));
	REQUIRE(name.has_value());
	CHECK(name.value() == "NormalNewFont-Light");
}

TEST_CASE("TextLayer set_style_normal_font_by_name reuses existing font")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer != nullptr);

	// Get the name of an existing font
	const auto existing_name = text_layer->font_postscript_name(0);
	REQUIRE(existing_name.has_value());
	const auto count_before = text_layer->font_count();

	// set_style_normal_font_by_name with existing font should NOT add a new entry
	CHECK_NOTHROW(text_layer->set_style_normal_font_by_name(existing_name.value()));
	CHECK(text_layer->font_count() == count_before);

	// Normal font should be 0
	const auto normal_font = text_layer->style_normal_font();
	REQUIRE(normal_font.has_value());
	CHECK(normal_font.value() == 0);
}

TEST_CASE("TextLayer add_font roundtrip through file write and read")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer != nullptr);

	// Add a font and wire it into run 0
	const auto new_idx = text_layer->add_font("RoundtripFont-Medium");
	REQUIRE(new_idx >= 0);
	CHECK_NOTHROW(text_layer->set_style_run_font(0, new_idx));
	const auto count_after_add = text_layer->font_count();

	// Write to temp file
	auto tmp_path = std::filesystem::temp_directory_path() / "roundtrip_fontset_test.psb";
	LayeredFile<bpp8_t>::write(std::move(file), tmp_path);

	// Read back
	auto file2 = LayeredFile<bpp8_t>::read(tmp_path);
	auto text_layer2 = find_text_layer_with_contents(file2, "Hello 123");
	REQUIRE(text_layer2 != nullptr);

	// Verify font count survived
	CHECK(text_layer2->font_count() == count_after_add);
	// Verify the added font is still there
	const auto name = text_layer2->font_postscript_name(static_cast<size_t>(new_idx));
	REQUIRE(name.has_value());
	CHECK(name.value() == "RoundtripFont-Medium");

	// Verify style run 0 still points at the new font
	const auto run_font = text_layer2->style_run_font(0);
	REQUIRE(run_font.has_value());
	CHECK(run_font.value() == new_idx);

	// Clean up
	std::filesystem::remove(tmp_path);
}



