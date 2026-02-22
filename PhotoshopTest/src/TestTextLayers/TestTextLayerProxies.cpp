#include "doctest.h"

#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <string>

namespace
{
	std::filesystem::path temp_psd_path()
	{
		static std::atomic<uint64_t> counter{ 0u };
		const auto stamp = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
		const auto idx = counter.fetch_add(1u, std::memory_order_relaxed);
		return std::filesystem::temp_directory_path() / ("psapi_text_proxy_" + std::to_string(stamp) + "_" + std::to_string(idx) + ".psd");
	}

	template <typename T>
	std::shared_ptr<NAMESPACE_PSAPI::TextLayer<T>> find_text_layer_by_name(
		NAMESPACE_PSAPI::LayeredFile<T>& file,
		const std::string& name)
	{
		auto flat_layers = file.flat_layers();
		for (const auto& layer : flat_layers)
		{
			auto text_layer = std::dynamic_pointer_cast<NAMESPACE_PSAPI::TextLayer<T>>(layer);
			if (text_layer && text_layer->name() == name)
			{
				return text_layer;
			}
		}
		return nullptr;
	}

	template <typename T>
	std::shared_ptr<NAMESPACE_PSAPI::TextLayer<T>> find_text_layer_containing(
		NAMESPACE_PSAPI::LayeredFile<T>& file,
		const std::string& substring)
	{
		auto flat_layers = file.flat_layers();
		for (const auto& layer : flat_layers)
		{
			auto text_layer = std::dynamic_pointer_cast<NAMESPACE_PSAPI::TextLayer<T>>(layer);
			if (text_layer)
			{
				const auto contents = text_layer->text();
				if (contents && contents->find(substring) != std::string::npos)
				{
					return text_layer;
				}
			}
		}
		return nullptr;
	}

	const auto kCharStyleFixture = std::filesystem::path("documents") / "TextLayers" / "TextLayers_CharacterStyles.psd";
	const auto kParagraphFixture = std::filesystem::path("documents") / "TextLayers" / "TextLayers_Paragraph.psd";
	const auto kBasicFixture     = std::filesystem::path("documents") / "TextLayers" / "TextLayers_Basic.psd";
	const auto kStyleRunFixture  = std::filesystem::path("documents") / "TextLayers" / "TextLayers_StyleRuns.psd";
}


// ===========================================================================
//  StyleRunProxy
// ===========================================================================

TEST_CASE("Proxy: StyleRunProxy getters agree with flat mixin getters")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kCharStyleFixture;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "CharacterStylePrimary");
	REQUIRE(layer != nullptr);

	auto proxy = layer->style_run(0u);

	// Compare each proxy getter against the raw flat getter
	CHECK(proxy.font_size()           == layer->style_run_font_size(0u));
	CHECK(proxy.leading()             == layer->style_run_leading(0u));
	CHECK(proxy.auto_leading()        == layer->style_run_auto_leading(0u));
	CHECK(proxy.kerning()             == layer->style_run_kerning(0u));
	CHECK(proxy.fill_color()          == layer->style_run_fill_color(0u));
	CHECK(proxy.stroke_color()        == layer->style_run_stroke_color(0u));
	CHECK(proxy.font()                == layer->style_run_font(0u));
	CHECK(proxy.faux_bold()           == layer->style_run_faux_bold(0u));
	CHECK(proxy.faux_italic()         == layer->style_run_faux_italic(0u));
	CHECK(proxy.horizontal_scale()    == layer->style_run_horizontal_scale(0u));
	CHECK(proxy.vertical_scale()      == layer->style_run_vertical_scale(0u));
	CHECK(proxy.tracking()            == layer->style_run_tracking(0u));
	CHECK(proxy.auto_kerning()        == layer->style_run_auto_kerning(0u));
	CHECK(proxy.baseline_shift()      == layer->style_run_baseline_shift(0u));
	CHECK(proxy.font_caps()           == layer->style_run_font_caps(0u));
	CHECK(proxy.font_baseline()       == layer->style_run_font_baseline(0u));
	CHECK(proxy.no_break()            == layer->style_run_no_break(0u));
	CHECK(proxy.language()            == layer->style_run_language(0u));
	CHECK(proxy.character_direction() == layer->style_run_character_direction(0u));
	CHECK(proxy.baseline_direction()  == layer->style_run_baseline_direction(0u));
	CHECK(proxy.tsume()               == layer->style_run_tsume(0u));
	CHECK(proxy.kashida()             == layer->style_run_kashida(0u));
	CHECK(proxy.diacritic_pos()       == layer->style_run_diacritic_pos(0u));
	CHECK(proxy.ligatures()           == layer->style_run_ligatures(0u));
	CHECK(proxy.dligatures()          == layer->style_run_dligatures(0u));
	CHECK(proxy.underline()           == layer->style_run_underline(0u));
	CHECK(proxy.strikethrough()       == layer->style_run_strikethrough(0u));
	CHECK(proxy.stroke_flag()         == layer->style_run_stroke_flag(0u));
	CHECK(proxy.fill_flag()           == layer->style_run_fill_flag(0u));
	CHECK(proxy.fill_first()          == layer->style_run_fill_first(0u));
	CHECK(proxy.outline_width()       == layer->style_run_outline_width(0u));
}


TEST_CASE("Proxy: StyleRunProxy setter mutates via proxy and roundtrips")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kCharStyleFixture;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "CharacterStylePrimary");
	REQUIRE(layer != nullptr);

	auto proxy = layer->style_run(0u);

	// Mutate font size through the proxy
	const auto old_size = proxy.font_size();
	REQUIRE(old_size.has_value());
	const double new_size = old_size.value() + 10.0;

	CHECK_NOTHROW(proxy.set_font_size(new_size));

	// Verify readback through flat API too
	CHECK(layer->style_run_font_size(0u).value() == doctest::Approx(new_size));

	// Roundtrip
	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_by_name(reread, "CharacterStylePrimary");
	REQUIRE(reread_layer != nullptr);

	CHECK(reread_layer->style_run(0u).font_size().value() == doctest::Approx(new_size));

	std::filesystem::remove(out_path);
}


// ===========================================================================
//  StyleNormalProxy
// ===========================================================================

TEST_CASE("Proxy: StyleNormalProxy getters agree with flat mixin getters")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kCharStyleFixture;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "CharacterStylePrimary");
	REQUIRE(layer != nullptr);

	auto proxy = layer->style_normal();

	CHECK(proxy.sheet_index()           == layer->style_normal_sheet_index());
	CHECK(proxy.font()                  == layer->style_normal_font());
	CHECK(proxy.font_size()             == layer->style_normal_font_size());
	CHECK(proxy.leading()               == layer->style_normal_leading());
	CHECK(proxy.auto_leading()          == layer->style_normal_auto_leading());
	CHECK(proxy.kerning()               == layer->style_normal_kerning());
	CHECK(proxy.faux_bold()             == layer->style_normal_faux_bold());
	CHECK(proxy.faux_italic()           == layer->style_normal_faux_italic());
	CHECK(proxy.horizontal_scale()      == layer->style_normal_horizontal_scale());
	CHECK(proxy.vertical_scale()        == layer->style_normal_vertical_scale());
	CHECK(proxy.tracking()              == layer->style_normal_tracking());
	CHECK(proxy.auto_kerning()          == layer->style_normal_auto_kerning());
	CHECK(proxy.baseline_shift()        == layer->style_normal_baseline_shift());
	CHECK(proxy.font_caps()             == layer->style_normal_font_caps());
	CHECK(proxy.font_baseline()         == layer->style_normal_font_baseline());
	CHECK(proxy.no_break()              == layer->style_normal_no_break());
	CHECK(proxy.language()              == layer->style_normal_language());
	CHECK(proxy.character_direction()   == layer->style_normal_character_direction());
	CHECK(proxy.baseline_direction()    == layer->style_normal_baseline_direction());
	CHECK(proxy.tsume()                 == layer->style_normal_tsume());
	CHECK(proxy.kashida()               == layer->style_normal_kashida());
	CHECK(proxy.diacritic_pos()         == layer->style_normal_diacritic_pos());
	CHECK(proxy.ligatures()             == layer->style_normal_ligatures());
	CHECK(proxy.dligatures()            == layer->style_normal_dligatures());
	CHECK(proxy.underline()             == layer->style_normal_underline());
	CHECK(proxy.strikethrough()         == layer->style_normal_strikethrough());
	CHECK(proxy.stroke_flag()           == layer->style_normal_stroke_flag());
	CHECK(proxy.fill_flag()             == layer->style_normal_fill_flag());
	CHECK(proxy.fill_first()            == layer->style_normal_fill_first());
	CHECK(proxy.outline_width()         == layer->style_normal_outline_width());
	CHECK(proxy.fill_color()            == layer->style_normal_fill_color());
	CHECK(proxy.stroke_color()          == layer->style_normal_stroke_color());
}


TEST_CASE("Proxy: StyleNormalProxy setter mutates via proxy and roundtrips")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kCharStyleFixture;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "CharacterStylePrimary");
	REQUIRE(layer != nullptr);

	auto proxy = layer->style_normal();

	const auto old_size = proxy.font_size();
	REQUIRE(old_size.has_value());
	const double new_size = old_size.value() + 5.0;

	CHECK_NOTHROW(proxy.set_font_size(new_size));
	CHECK(layer->style_normal_font_size().value() == doctest::Approx(new_size));

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_by_name(reread, "CharacterStylePrimary");
	REQUIRE(reread_layer != nullptr);

	CHECK(reread_layer->style_normal().font_size().value() == doctest::Approx(new_size));

	std::filesystem::remove(out_path);
}


// ===========================================================================
//  ParagraphRunProxy
// ===========================================================================

TEST_CASE("Proxy: ParagraphRunProxy getters agree with flat mixin getters")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kParagraphFixture;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_containing(file, "paragraph text for fixture coverage");
	REQUIRE(layer != nullptr);

	auto proxy = layer->paragraph_run(0u);

	CHECK(proxy.justification()        == layer->paragraph_run_justification(0u));
	CHECK(proxy.first_line_indent()    == layer->paragraph_run_first_line_indent(0u));
	CHECK(proxy.start_indent()         == layer->paragraph_run_start_indent(0u));
	CHECK(proxy.end_indent()           == layer->paragraph_run_end_indent(0u));
	CHECK(proxy.space_before()         == layer->paragraph_run_space_before(0u));
	CHECK(proxy.space_after()          == layer->paragraph_run_space_after(0u));
	CHECK(proxy.auto_hyphenate()       == layer->paragraph_run_auto_hyphenate(0u));
	CHECK(proxy.hyphenated_word_size() == layer->paragraph_run_hyphenated_word_size(0u));
	CHECK(proxy.pre_hyphen()           == layer->paragraph_run_pre_hyphen(0u));
	CHECK(proxy.post_hyphen()          == layer->paragraph_run_post_hyphen(0u));
	CHECK(proxy.consecutive_hyphens()  == layer->paragraph_run_consecutive_hyphens(0u));
	CHECK(proxy.zone()                 == layer->paragraph_run_zone(0u));
	CHECK(proxy.word_spacing()         == layer->paragraph_run_word_spacing(0u));
	CHECK(proxy.letter_spacing()       == layer->paragraph_run_letter_spacing(0u));
	CHECK(proxy.glyph_spacing()        == layer->paragraph_run_glyph_spacing(0u));
	CHECK(proxy.auto_leading()         == layer->paragraph_run_auto_leading(0u));
	CHECK(proxy.leading_type()         == layer->paragraph_run_leading_type(0u));
	CHECK(proxy.hanging()              == layer->paragraph_run_hanging(0u));
	CHECK(proxy.burasagari()           == layer->paragraph_run_burasagari(0u));
	CHECK(proxy.kinsoku_order()        == layer->paragraph_run_kinsoku_order(0u));
	CHECK(proxy.every_line_composer()  == layer->paragraph_run_every_line_composer(0u));
}


TEST_CASE("Proxy: ParagraphRunProxy setter mutates via proxy and roundtrips")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kParagraphFixture;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_containing(file, "paragraph text for fixture coverage");
	REQUIRE(layer != nullptr);

	auto proxy = layer->paragraph_run(0u);
	const auto old_just = proxy.justification();
	REQUIRE(old_just.has_value());

	// Toggle to a different justification
	const auto new_just = (old_just.value() == TextLayerEnum::Justification::Left)
		? TextLayerEnum::Justification::Center
		: TextLayerEnum::Justification::Left;

	CHECK_NOTHROW(proxy.set_justification(new_just));
	CHECK(layer->paragraph_run_justification(0u).value() == new_just);

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_containing(reread, "paragraph text for fixture coverage");
	REQUIRE(reread_layer != nullptr);

	CHECK(reread_layer->paragraph_run(0u).justification().value() == new_just);

	std::filesystem::remove(out_path);
}


// ===========================================================================
//  ParagraphNormalProxy
// ===========================================================================

TEST_CASE("Proxy: ParagraphNormalProxy getters agree with flat mixin getters")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kParagraphFixture;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_containing(file, "paragraph text for fixture coverage");
	REQUIRE(layer != nullptr);

	auto proxy = layer->paragraph_normal();

	CHECK(proxy.sheet_index()            == layer->paragraph_normal_sheet_index());
	CHECK(proxy.justification()          == layer->paragraph_normal_justification());
	CHECK(proxy.first_line_indent()      == layer->paragraph_normal_first_line_indent());
	CHECK(proxy.start_indent()           == layer->paragraph_normal_start_indent());
	CHECK(proxy.end_indent()             == layer->paragraph_normal_end_indent());
	CHECK(proxy.space_before()           == layer->paragraph_normal_space_before());
	CHECK(proxy.space_after()            == layer->paragraph_normal_space_after());
	CHECK(proxy.auto_hyphenate()         == layer->paragraph_normal_auto_hyphenate());
	CHECK(proxy.hyphenated_word_size()   == layer->paragraph_normal_hyphenated_word_size());
	CHECK(proxy.pre_hyphen()             == layer->paragraph_normal_pre_hyphen());
	CHECK(proxy.post_hyphen()            == layer->paragraph_normal_post_hyphen());
	CHECK(proxy.consecutive_hyphens()    == layer->paragraph_normal_consecutive_hyphens());
	CHECK(proxy.zone()                   == layer->paragraph_normal_zone());
	CHECK(proxy.word_spacing()           == layer->paragraph_normal_word_spacing());
	CHECK(proxy.letter_spacing()         == layer->paragraph_normal_letter_spacing());
	CHECK(proxy.glyph_spacing()          == layer->paragraph_normal_glyph_spacing());
	CHECK(proxy.auto_leading()           == layer->paragraph_normal_auto_leading());
	CHECK(proxy.leading_type()           == layer->paragraph_normal_leading_type());
	CHECK(proxy.hanging()                == layer->paragraph_normal_hanging());
	CHECK(proxy.burasagari()             == layer->paragraph_normal_burasagari());
	CHECK(proxy.kinsoku_order()          == layer->paragraph_normal_kinsoku_order());
	CHECK(proxy.every_line_composer()    == layer->paragraph_normal_every_line_composer());
}


TEST_CASE("Proxy: ParagraphNormalProxy setter mutates via proxy and roundtrips")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kParagraphFixture;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_containing(file, "paragraph text for fixture coverage");
	REQUIRE(layer != nullptr);

	auto proxy = layer->paragraph_normal();
	const auto old_just = proxy.justification();
	REQUIRE(old_just.has_value());

	const auto new_just = (old_just.value() == TextLayerEnum::Justification::Left)
		? TextLayerEnum::Justification::Right
		: TextLayerEnum::Justification::Left;

	CHECK_NOTHROW(proxy.set_justification(new_just));
	CHECK(layer->paragraph_normal_justification().value() == new_just);

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_containing(reread, "paragraph text for fixture coverage");
	REQUIRE(reread_layer != nullptr);

	CHECK(reread_layer->paragraph_normal().justification().value() == new_just);

	std::filesystem::remove(out_path);
}


// ===========================================================================
//  FontProxy
// ===========================================================================

TEST_CASE("Proxy: FontProxy getters agree with flat mixin getters")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kBasicFixture;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	// Use the first text layer we can find
	auto flat_layers = file.flat_layers();
	std::shared_ptr<TextLayer<bpp8_t>> layer;
	for (const auto& l : flat_layers)
	{
		layer = std::dynamic_pointer_cast<TextLayer<bpp8_t>>(l);
		if (layer) break;
	}
	REQUIRE(layer != nullptr);
	REQUIRE(layer->font_count() > 0u);

	auto proxy = layer->font(0u);

	CHECK(proxy.postscript_name() == layer->font_postscript_name(0u));
	CHECK(proxy.name()            == layer->font_name(0u));
	CHECK(proxy.script()          == layer->font_script(0u));
	CHECK(proxy.type()            == layer->font_type(0u));
	CHECK(proxy.synthetic()       == layer->font_synthetic(0u));
	CHECK(proxy.is_sentinel()     == layer->is_sentinel_font(0u));
}


TEST_CASE("Proxy: FontProxy setter roundtrips postscript_name")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kBasicFixture;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto flat_layers = file.flat_layers();
	std::shared_ptr<TextLayer<bpp8_t>> layer;
	for (const auto& l : flat_layers)
	{
		layer = std::dynamic_pointer_cast<TextLayer<bpp8_t>>(l);
		if (layer) break;
	}
	REQUIRE(layer != nullptr);
	REQUIRE(layer->font_count() > 0u);

	auto proxy = layer->font(0u);
	const auto old_name = proxy.postscript_name();
	REQUIRE(old_name.has_value());

	CHECK_NOTHROW(proxy.set_postscript_name("Helvetica-Bold"));
	CHECK(layer->font_postscript_name(0u).value() == "Helvetica-Bold");

	const auto out_path = temp_psd_path();
	auto layer_name = layer->name(); // save for lookup after move
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_by_name(reread, layer_name);
	REQUIRE(reread_layer != nullptr);

	CHECK(reread_layer->font(0u).postscript_name().value() == "Helvetica-Bold");

	std::filesystem::remove(out_path);
}


// ===========================================================================
//  FontSetProxy
// ===========================================================================

TEST_CASE("Proxy: FontSetProxy getters agree with flat mixin getters")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kBasicFixture;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto flat_layers = file.flat_layers();
	std::shared_ptr<TextLayer<bpp8_t>> layer;
	for (const auto& l : flat_layers)
	{
		layer = std::dynamic_pointer_cast<TextLayer<bpp8_t>>(l);
		if (layer) break;
	}
	REQUIRE(layer != nullptr);

	auto proxy = layer->font_set();

	CHECK(proxy.count()        == layer->font_count());
	CHECK(proxy.used_indices() == layer->used_font_indices());
	CHECK(proxy.used_names()   == layer->used_font_names());
}


TEST_CASE("Proxy: FontSetProxy add() creates a new font entry")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kBasicFixture;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto flat_layers = file.flat_layers();
	std::shared_ptr<TextLayer<bpp8_t>> layer;
	for (const auto& l : flat_layers)
	{
		layer = std::dynamic_pointer_cast<TextLayer<bpp8_t>>(l);
		if (layer) break;
	}
	REQUIRE(layer != nullptr);

	auto proxy = layer->font_set();
	const size_t count_before = proxy.count();

	const int32_t new_idx = proxy.add("TestFont-Regular");
	CHECK(new_idx >= 0);
	CHECK(proxy.count() == count_before + 1u);

	// Verify through find_index
	CHECK(proxy.find_index("TestFont-Regular") == new_idx);

	// Verify through the FontProxy at the new index
	auto font_proxy = layer->font(static_cast<size_t>(new_idx));
	CHECK(font_proxy.postscript_name().value() == "TestFont-Regular");
}


TEST_CASE("Proxy: FontSetProxy find_index returns -1 for unknown fonts")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kBasicFixture;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto flat_layers = file.flat_layers();
	std::shared_ptr<TextLayer<bpp8_t>> layer;
	for (const auto& l : flat_layers)
	{
		layer = std::dynamic_pointer_cast<TextLayer<bpp8_t>>(l);
		if (layer) break;
	}
	REQUIRE(layer != nullptr);

	CHECK(layer->font_set().find_index("NonExistentFont-XYZ123") == -1);
}
