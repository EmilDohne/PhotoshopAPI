#include "doctest.h"
#include "TestTextLayerMutationUtils.h"

TEST_CASE("TextLayer mutates paragraph run properties with roundtrip")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Paragraph.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_containing(file, "paragraph text for fixture coverage");
	REQUIRE(text_layer != nullptr);

	REQUIRE(text_layer->paragraph_run_count() >= 1u);
	const auto justification_before = text_layer->paragraph_run_justification(0u);
	const auto first_line_before = text_layer->paragraph_run_first_line_indent(0u);
	const auto start_before = text_layer->paragraph_run_start_indent(0u);
	const auto end_before = text_layer->paragraph_run_end_indent(0u);
	const auto space_before_before = text_layer->paragraph_run_space_before(0u);
	const auto space_after_before = text_layer->paragraph_run_space_after(0u);
	const auto auto_hyphenate_before = text_layer->paragraph_run_auto_hyphenate(0u);
	const auto hyphenated_word_size_before = text_layer->paragraph_run_hyphenated_word_size(0u);
	const auto pre_hyphen_before = text_layer->paragraph_run_pre_hyphen(0u);
	const auto post_hyphen_before = text_layer->paragraph_run_post_hyphen(0u);
	const auto consecutive_hyphens_before = text_layer->paragraph_run_consecutive_hyphens(0u);
	const auto zone_before = text_layer->paragraph_run_zone(0u);
	const auto word_spacing_before = text_layer->paragraph_run_word_spacing(0u);
	const auto letter_spacing_before = text_layer->paragraph_run_letter_spacing(0u);
	const auto glyph_spacing_before = text_layer->paragraph_run_glyph_spacing(0u);
	const auto auto_leading_before = text_layer->paragraph_run_auto_leading(0u);
	const auto leading_type_before = text_layer->paragraph_run_leading_type(0u);
	const auto hanging_before = text_layer->paragraph_run_hanging(0u);
	const auto burasagari_before = text_layer->paragraph_run_burasagari(0u);
	const auto kinsoku_order_before = text_layer->paragraph_run_kinsoku_order(0u);
	const auto every_line_composer_before = text_layer->paragraph_run_every_line_composer(0u);
	REQUIRE(justification_before.has_value());
	REQUIRE(first_line_before.has_value());
	REQUIRE(start_before.has_value());
	REQUIRE(end_before.has_value());
	REQUIRE(space_before_before.has_value());
	REQUIRE(space_after_before.has_value());
	REQUIRE(auto_hyphenate_before.has_value());
	REQUIRE(hyphenated_word_size_before.has_value());
	REQUIRE(pre_hyphen_before.has_value());
	REQUIRE(post_hyphen_before.has_value());
	REQUIRE(consecutive_hyphens_before.has_value());
	REQUIRE(zone_before.has_value());
	REQUIRE(word_spacing_before.has_value());
	REQUIRE(letter_spacing_before.has_value());
	REQUIRE(glyph_spacing_before.has_value());
	REQUIRE(auto_leading_before.has_value());
	REQUIRE(leading_type_before.has_value());
	REQUIRE(hanging_before.has_value());
	REQUIRE(burasagari_before.has_value());
	REQUIRE(kinsoku_order_before.has_value());
	REQUIRE(every_line_composer_before.has_value());
	REQUIRE(word_spacing_before->size() == 3u);
	REQUIRE(letter_spacing_before->size() == 3u);
	REQUIRE(glyph_spacing_before->size() == 3u);

	const auto justification_after = (justification_before.value() == TextLayerEnum::Justification::Left) ? TextLayerEnum::Justification::Right : TextLayerEnum::Justification::Left;
	const double first_line_after = first_line_before.value() + 12.5;
	const double start_after = start_before.value() + 4.25;
	const double end_after = end_before.value() + 2.75;
	const double space_before_after = space_before_before.value() + 3.0;
	const double space_after_after = space_after_before.value() + 6.0;
	const bool auto_hyphenate_after = !auto_hyphenate_before.value();
	const int32_t hyphenated_word_size_after = hyphenated_word_size_before.value() + 1;
	const int32_t pre_hyphen_after = pre_hyphen_before.value() + 1;
	const int32_t post_hyphen_after = post_hyphen_before.value() + 1;
	const int32_t consecutive_hyphens_after = consecutive_hyphens_before.value() + 1;
	const double zone_after = zone_before.value() + 5.5;
	const std::vector<double> word_spacing_after{
		(*word_spacing_before)[0] + 0.05,
		(*word_spacing_before)[1] + 0.05,
		(*word_spacing_before)[2] + 0.05
	};
	const std::vector<double> letter_spacing_after{
		(*letter_spacing_before)[0] + 1.0,
		(*letter_spacing_before)[1] + 1.0,
		(*letter_spacing_before)[2] + 1.0
	};
	const std::vector<double> glyph_spacing_after{
		(*glyph_spacing_before)[0] + 0.1,
		(*glyph_spacing_before)[1] + 0.1,
		(*glyph_spacing_before)[2] + 0.1
	};
	const double auto_leading_after = auto_leading_before.value() + 0.2;
	const auto leading_type_after = (leading_type_before.value() == TextLayerEnum::LeadingType::BottomToBottom) ? TextLayerEnum::LeadingType::TopToTop : TextLayerEnum::LeadingType::BottomToBottom;
	const bool hanging_after = !hanging_before.value();
	const bool burasagari_after = !burasagari_before.value();
	const auto kinsoku_order_after = (kinsoku_order_before.value() == TextLayerEnum::KinsokuOrder::PushInFirst) ? TextLayerEnum::KinsokuOrder::PushOutFirst : TextLayerEnum::KinsokuOrder::PushInFirst;
	const bool every_line_composer_after = !every_line_composer_before.value();

	CHECK_NOTHROW(text_layer->set_paragraph_run_justification(0u, justification_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_first_line_indent(0u, first_line_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_start_indent(0u, start_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_end_indent(0u, end_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_space_before(0u, space_before_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_space_after(0u, space_after_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_auto_hyphenate(0u, auto_hyphenate_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_hyphenated_word_size(0u, hyphenated_word_size_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_pre_hyphen(0u, pre_hyphen_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_post_hyphen(0u, post_hyphen_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_consecutive_hyphens(0u, consecutive_hyphens_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_zone(0u, zone_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_word_spacing(0u, word_spacing_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_letter_spacing(0u, letter_spacing_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_glyph_spacing(0u, glyph_spacing_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_auto_leading(0u, auto_leading_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_leading_type(0u, leading_type_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_hanging(0u, hanging_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_burasagari(0u, burasagari_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_kinsoku_order(0u, kinsoku_order_after));
	CHECK_NOTHROW(text_layer->set_paragraph_run_every_line_composer(0u, every_line_composer_after));
	CHECK_THROWS(text_layer->set_paragraph_run_justification(200u, justification_after));
	CHECK_THROWS(text_layer->set_paragraph_run_first_line_indent(200u, first_line_after));
	CHECK_THROWS(text_layer->set_paragraph_run_start_indent(200u, start_after));
	CHECK_THROWS(text_layer->set_paragraph_run_end_indent(200u, end_after));
	CHECK_THROWS(text_layer->set_paragraph_run_space_before(200u, space_before_after));
	CHECK_THROWS(text_layer->set_paragraph_run_space_after(200u, space_after_after));
	CHECK_THROWS(text_layer->set_paragraph_run_auto_hyphenate(200u, auto_hyphenate_after));
	CHECK_THROWS(text_layer->set_paragraph_run_hyphenated_word_size(200u, hyphenated_word_size_after));
	CHECK_THROWS(text_layer->set_paragraph_run_pre_hyphen(200u, pre_hyphen_after));
	CHECK_THROWS(text_layer->set_paragraph_run_post_hyphen(200u, post_hyphen_after));
	CHECK_THROWS(text_layer->set_paragraph_run_consecutive_hyphens(200u, consecutive_hyphens_after));
	CHECK_THROWS(text_layer->set_paragraph_run_zone(200u, zone_after));
	CHECK_THROWS(text_layer->set_paragraph_run_word_spacing(200u, word_spacing_after));
	CHECK_THROWS(text_layer->set_paragraph_run_letter_spacing(200u, letter_spacing_after));
	CHECK_THROWS(text_layer->set_paragraph_run_glyph_spacing(200u, glyph_spacing_after));
	CHECK_THROWS(text_layer->set_paragraph_run_auto_leading(200u, auto_leading_after));
	CHECK_THROWS(text_layer->set_paragraph_run_leading_type(200u, leading_type_after));
	CHECK_THROWS(text_layer->set_paragraph_run_hanging(200u, hanging_after));
	CHECK_THROWS(text_layer->set_paragraph_run_burasagari(200u, burasagari_after));
	CHECK_THROWS(text_layer->set_paragraph_run_kinsoku_order(200u, kinsoku_order_after));
	CHECK_THROWS(text_layer->set_paragraph_run_every_line_composer(200u, every_line_composer_after));
	CHECK_THROWS(text_layer->set_paragraph_run_space_before(0u, std::numeric_limits<double>::infinity()));
	CHECK_THROWS(text_layer->set_paragraph_run_word_spacing(0u, {}));
	CHECK_THROWS(text_layer->set_paragraph_run_word_spacing(0u, std::vector<double>{ 1.0, std::numeric_limits<double>::infinity(), 2.0 }));

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_containing(reread, "paragraph text for fixture coverage");
	REQUIRE(reread_layer != nullptr);

	const auto reread_justification = reread_layer->paragraph_run_justification(0u);
	const auto reread_first_line = reread_layer->paragraph_run_first_line_indent(0u);
	const auto reread_start = reread_layer->paragraph_run_start_indent(0u);
	const auto reread_end = reread_layer->paragraph_run_end_indent(0u);
	const auto reread_space_before = reread_layer->paragraph_run_space_before(0u);
	const auto reread_space_after = reread_layer->paragraph_run_space_after(0u);
	const auto reread_auto_hyphenate = reread_layer->paragraph_run_auto_hyphenate(0u);
	const auto reread_hyphenated_word_size = reread_layer->paragraph_run_hyphenated_word_size(0u);
	const auto reread_pre_hyphen = reread_layer->paragraph_run_pre_hyphen(0u);
	const auto reread_post_hyphen = reread_layer->paragraph_run_post_hyphen(0u);
	const auto reread_consecutive_hyphens = reread_layer->paragraph_run_consecutive_hyphens(0u);
	const auto reread_zone = reread_layer->paragraph_run_zone(0u);
	const auto reread_word_spacing = reread_layer->paragraph_run_word_spacing(0u);
	const auto reread_letter_spacing = reread_layer->paragraph_run_letter_spacing(0u);
	const auto reread_glyph_spacing = reread_layer->paragraph_run_glyph_spacing(0u);
	const auto reread_auto_leading = reread_layer->paragraph_run_auto_leading(0u);
	const auto reread_leading_type = reread_layer->paragraph_run_leading_type(0u);
	const auto reread_hanging = reread_layer->paragraph_run_hanging(0u);
	const auto reread_burasagari = reread_layer->paragraph_run_burasagari(0u);
	const auto reread_kinsoku_order = reread_layer->paragraph_run_kinsoku_order(0u);
	const auto reread_every_line_composer = reread_layer->paragraph_run_every_line_composer(0u);
	REQUIRE(reread_justification.has_value());
	REQUIRE(reread_first_line.has_value());
	REQUIRE(reread_start.has_value());
	REQUIRE(reread_end.has_value());
	REQUIRE(reread_space_before.has_value());
	REQUIRE(reread_space_after.has_value());
	REQUIRE(reread_auto_hyphenate.has_value());
	REQUIRE(reread_hyphenated_word_size.has_value());
	REQUIRE(reread_pre_hyphen.has_value());
	REQUIRE(reread_post_hyphen.has_value());
	REQUIRE(reread_consecutive_hyphens.has_value());
	REQUIRE(reread_zone.has_value());
	REQUIRE(reread_word_spacing.has_value());
	REQUIRE(reread_letter_spacing.has_value());
	REQUIRE(reread_glyph_spacing.has_value());
	REQUIRE(reread_auto_leading.has_value());
	REQUIRE(reread_leading_type.has_value());
	REQUIRE(reread_hanging.has_value());
	REQUIRE(reread_burasagari.has_value());
	REQUIRE(reread_kinsoku_order.has_value());
	REQUIRE(reread_every_line_composer.has_value());
	REQUIRE(reread_word_spacing->size() == 3u);
	REQUIRE(reread_letter_spacing->size() == 3u);
	REQUIRE(reread_glyph_spacing->size() == 3u);

	CHECK(reread_justification.value() == justification_after);
	CHECK(doctest::Approx(reread_first_line.value()).epsilon(0.0001) == first_line_after);
	CHECK(doctest::Approx(reread_start.value()).epsilon(0.0001) == start_after);
	CHECK(doctest::Approx(reread_end.value()).epsilon(0.0001) == end_after);
	CHECK(doctest::Approx(reread_space_before.value()).epsilon(0.0001) == space_before_after);
	CHECK(doctest::Approx(reread_space_after.value()).epsilon(0.0001) == space_after_after);
	CHECK(reread_auto_hyphenate.value() == auto_hyphenate_after);
	CHECK(reread_hyphenated_word_size.value() == hyphenated_word_size_after);
	CHECK(reread_pre_hyphen.value() == pre_hyphen_after);
	CHECK(reread_post_hyphen.value() == post_hyphen_after);
	CHECK(reread_consecutive_hyphens.value() == consecutive_hyphens_after);
	CHECK(doctest::Approx(reread_zone.value()).epsilon(0.0001) == zone_after);
	CHECK(doctest::Approx((*reread_word_spacing)[0]).epsilon(0.0001) == word_spacing_after[0]);
	CHECK(doctest::Approx((*reread_word_spacing)[1]).epsilon(0.0001) == word_spacing_after[1]);
	CHECK(doctest::Approx((*reread_word_spacing)[2]).epsilon(0.0001) == word_spacing_after[2]);
	CHECK(doctest::Approx((*reread_letter_spacing)[0]).epsilon(0.0001) == letter_spacing_after[0]);
	CHECK(doctest::Approx((*reread_letter_spacing)[1]).epsilon(0.0001) == letter_spacing_after[1]);
	CHECK(doctest::Approx((*reread_letter_spacing)[2]).epsilon(0.0001) == letter_spacing_after[2]);
	CHECK(doctest::Approx((*reread_glyph_spacing)[0]).epsilon(0.0001) == glyph_spacing_after[0]);
	CHECK(doctest::Approx((*reread_glyph_spacing)[1]).epsilon(0.0001) == glyph_spacing_after[1]);
	CHECK(doctest::Approx((*reread_glyph_spacing)[2]).epsilon(0.0001) == glyph_spacing_after[2]);
	CHECK(doctest::Approx(reread_auto_leading.value()).epsilon(0.0001) == auto_leading_after);
	CHECK(reread_leading_type.value() == leading_type_after);
	CHECK(reread_hanging.value() == hanging_after);
	CHECK(reread_burasagari.value() == burasagari_after);
	CHECK(reread_kinsoku_order.value() == kinsoku_order_after);
	CHECK(reread_every_line_composer.value() == every_line_composer_after);

	CHECK_FALSE(reread_layer->paragraph_run_justification(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_first_line_indent(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_start_indent(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_end_indent(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_space_before(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_space_after(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_auto_hyphenate(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_hyphenated_word_size(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_pre_hyphen(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_post_hyphen(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_consecutive_hyphens(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_zone(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_word_spacing(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_letter_spacing(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_glyph_spacing(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_auto_leading(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_leading_type(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_hanging(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_burasagari(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_kinsoku_order(200u).has_value());
	CHECK_FALSE(reread_layer->paragraph_run_every_line_composer(200u).has_value());

	std::filesystem::remove(out_path);
}


TEST_CASE("TextLayer mutates normal paragraph sheet properties with roundtrip")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Paragraph.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_containing(file, "paragraph text for fixture coverage");
	REQUIRE(text_layer != nullptr);

	REQUIRE(text_layer->paragraph_sheet_count() >= 1u);
	const auto normal_sheet_index_before = text_layer->paragraph_normal_sheet_index();
	const auto normal_justification_before = text_layer->paragraph_normal_justification();
	const auto normal_first_line_before = text_layer->paragraph_normal_first_line_indent();
	const auto normal_start_before = text_layer->paragraph_normal_start_indent();
	const auto normal_end_before = text_layer->paragraph_normal_end_indent();
	const auto normal_space_before_before = text_layer->paragraph_normal_space_before();
	const auto normal_space_after_before = text_layer->paragraph_normal_space_after();
	const auto normal_auto_hyphenate_before = text_layer->paragraph_normal_auto_hyphenate();
	const auto normal_hyphenated_word_size_before = text_layer->paragraph_normal_hyphenated_word_size();
	const auto normal_pre_hyphen_before = text_layer->paragraph_normal_pre_hyphen();
	const auto normal_post_hyphen_before = text_layer->paragraph_normal_post_hyphen();
	const auto normal_consecutive_hyphens_before = text_layer->paragraph_normal_consecutive_hyphens();
	const auto normal_zone_before = text_layer->paragraph_normal_zone();
	const auto normal_word_spacing_before = text_layer->paragraph_normal_word_spacing();
	const auto normal_letter_spacing_before = text_layer->paragraph_normal_letter_spacing();
	const auto normal_glyph_spacing_before = text_layer->paragraph_normal_glyph_spacing();
	const auto normal_auto_leading_before = text_layer->paragraph_normal_auto_leading();
	const auto normal_leading_type_before = text_layer->paragraph_normal_leading_type();
	const auto normal_hanging_before = text_layer->paragraph_normal_hanging();
	const auto normal_burasagari_before = text_layer->paragraph_normal_burasagari();
	const auto normal_kinsoku_order_before = text_layer->paragraph_normal_kinsoku_order();
	const auto normal_every_line_composer_before = text_layer->paragraph_normal_every_line_composer();
	const auto run_justification_before = text_layer->paragraph_run_justification(0u);

	REQUIRE(normal_sheet_index_before.has_value());
	REQUIRE(normal_justification_before.has_value());
	REQUIRE(normal_first_line_before.has_value());
	REQUIRE(normal_start_before.has_value());
	REQUIRE(normal_end_before.has_value());
	REQUIRE(normal_space_before_before.has_value());
	REQUIRE(normal_space_after_before.has_value());
	REQUIRE(normal_auto_hyphenate_before.has_value());
	REQUIRE(normal_hyphenated_word_size_before.has_value());
	REQUIRE(normal_pre_hyphen_before.has_value());
	REQUIRE(normal_post_hyphen_before.has_value());
	REQUIRE(normal_consecutive_hyphens_before.has_value());
	REQUIRE(normal_zone_before.has_value());
	REQUIRE(normal_word_spacing_before.has_value());
	REQUIRE(normal_letter_spacing_before.has_value());
	REQUIRE(normal_glyph_spacing_before.has_value());
	REQUIRE(normal_auto_leading_before.has_value());
	REQUIRE(normal_leading_type_before.has_value());
	REQUIRE(normal_hanging_before.has_value());
	REQUIRE(normal_burasagari_before.has_value());
	REQUIRE(normal_kinsoku_order_before.has_value());
	REQUIRE(normal_every_line_composer_before.has_value());
	REQUIRE(normal_word_spacing_before->size() == 3u);
	REQUIRE(normal_letter_spacing_before->size() == 3u);
	REQUIRE(normal_glyph_spacing_before->size() == 3u);
	REQUIRE(run_justification_before.has_value());

	const auto normal_justification_after = (normal_justification_before.value() == TextLayerEnum::Justification::Left) ? TextLayerEnum::Justification::Center : TextLayerEnum::Justification::Left;
	const double normal_first_line_after = normal_first_line_before.value() + 8.5;
	const double normal_start_after = normal_start_before.value() + 2.25;
	const double normal_end_after = normal_end_before.value() + 1.75;
	const double normal_space_before_after = normal_space_before_before.value() + 1.0;
	const double normal_space_after_after = normal_space_after_before.value() + 2.0;
	const bool normal_auto_hyphenate_after = !normal_auto_hyphenate_before.value();
	const int32_t normal_hyphenated_word_size_after = normal_hyphenated_word_size_before.value() + 1;
	const int32_t normal_pre_hyphen_after = normal_pre_hyphen_before.value() + 1;
	const int32_t normal_post_hyphen_after = normal_post_hyphen_before.value() + 1;
	const int32_t normal_consecutive_hyphens_after = normal_consecutive_hyphens_before.value() + 1;
	const double normal_zone_after = normal_zone_before.value() + 3.5;
	const std::vector<double> normal_word_spacing_after{
		(*normal_word_spacing_before)[0] + 0.02,
		(*normal_word_spacing_before)[1] + 0.02,
		(*normal_word_spacing_before)[2] + 0.02
	};
	const std::vector<double> normal_letter_spacing_after{
		(*normal_letter_spacing_before)[0] + 0.5,
		(*normal_letter_spacing_before)[1] + 0.5,
		(*normal_letter_spacing_before)[2] + 0.5
	};
	const std::vector<double> normal_glyph_spacing_after{
		(*normal_glyph_spacing_before)[0] + 0.02,
		(*normal_glyph_spacing_before)[1] + 0.02,
		(*normal_glyph_spacing_before)[2] + 0.02
	};
	const double normal_auto_leading_after = normal_auto_leading_before.value() + 0.1;
	const auto normal_leading_type_after = (normal_leading_type_before.value() == TextLayerEnum::LeadingType::BottomToBottom) ? TextLayerEnum::LeadingType::TopToTop : TextLayerEnum::LeadingType::BottomToBottom;
	const bool normal_hanging_after = !normal_hanging_before.value();
	const bool normal_burasagari_after = !normal_burasagari_before.value();
	const auto normal_kinsoku_order_after = (normal_kinsoku_order_before.value() == TextLayerEnum::KinsokuOrder::PushInFirst) ? TextLayerEnum::KinsokuOrder::PushOutFirst : TextLayerEnum::KinsokuOrder::PushInFirst;
	const bool normal_every_line_composer_after = !normal_every_line_composer_before.value();

	CHECK_NOTHROW(text_layer->set_paragraph_normal_sheet_index(normal_sheet_index_before.value()));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_justification(normal_justification_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_first_line_indent(normal_first_line_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_start_indent(normal_start_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_end_indent(normal_end_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_space_before(normal_space_before_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_space_after(normal_space_after_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_auto_hyphenate(normal_auto_hyphenate_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_hyphenated_word_size(normal_hyphenated_word_size_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_pre_hyphen(normal_pre_hyphen_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_post_hyphen(normal_post_hyphen_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_consecutive_hyphens(normal_consecutive_hyphens_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_zone(normal_zone_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_word_spacing(normal_word_spacing_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_letter_spacing(normal_letter_spacing_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_glyph_spacing(normal_glyph_spacing_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_auto_leading(normal_auto_leading_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_leading_type(normal_leading_type_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_hanging(normal_hanging_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_burasagari(normal_burasagari_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_kinsoku_order(normal_kinsoku_order_after));
	CHECK_NOTHROW(text_layer->set_paragraph_normal_every_line_composer(normal_every_line_composer_after));
	CHECK_THROWS(text_layer->set_paragraph_normal_sheet_index(200));
	CHECK_THROWS(text_layer->set_paragraph_normal_sheet_index(-1));
	CHECK_THROWS(text_layer->set_paragraph_normal_space_before(std::numeric_limits<double>::infinity()));
	CHECK_THROWS(text_layer->set_paragraph_normal_word_spacing({}));
	CHECK_THROWS(text_layer->set_paragraph_normal_word_spacing(std::vector<double>{ 1.0, std::numeric_limits<double>::infinity(), 2.0 }));
	CHECK_THROWS(text_layer->set_paragraph_normal_letter_spacing({}));
	CHECK_THROWS(text_layer->set_paragraph_normal_glyph_spacing({}));

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_containing(reread, "paragraph text for fixture coverage");
	REQUIRE(reread_layer != nullptr);

	const auto reread_normal_sheet_index = reread_layer->paragraph_normal_sheet_index();
	const auto reread_normal_justification = reread_layer->paragraph_normal_justification();
	const auto reread_normal_first_line = reread_layer->paragraph_normal_first_line_indent();
	const auto reread_normal_start = reread_layer->paragraph_normal_start_indent();
	const auto reread_normal_end = reread_layer->paragraph_normal_end_indent();
	const auto reread_normal_space_before = reread_layer->paragraph_normal_space_before();
	const auto reread_normal_space_after = reread_layer->paragraph_normal_space_after();
	const auto reread_normal_auto_hyphenate = reread_layer->paragraph_normal_auto_hyphenate();
	const auto reread_normal_hyphenated_word_size = reread_layer->paragraph_normal_hyphenated_word_size();
	const auto reread_normal_pre_hyphen = reread_layer->paragraph_normal_pre_hyphen();
	const auto reread_normal_post_hyphen = reread_layer->paragraph_normal_post_hyphen();
	const auto reread_normal_consecutive_hyphens = reread_layer->paragraph_normal_consecutive_hyphens();
	const auto reread_normal_zone = reread_layer->paragraph_normal_zone();
	const auto reread_normal_word_spacing = reread_layer->paragraph_normal_word_spacing();
	const auto reread_normal_letter_spacing = reread_layer->paragraph_normal_letter_spacing();
	const auto reread_normal_glyph_spacing = reread_layer->paragraph_normal_glyph_spacing();
	const auto reread_normal_auto_leading = reread_layer->paragraph_normal_auto_leading();
	const auto reread_normal_leading_type = reread_layer->paragraph_normal_leading_type();
	const auto reread_normal_hanging = reread_layer->paragraph_normal_hanging();
	const auto reread_normal_burasagari = reread_layer->paragraph_normal_burasagari();
	const auto reread_normal_kinsoku_order = reread_layer->paragraph_normal_kinsoku_order();
	const auto reread_normal_every_line_composer = reread_layer->paragraph_normal_every_line_composer();
	const auto reread_run_justification = reread_layer->paragraph_run_justification(0u);

	REQUIRE(reread_normal_sheet_index.has_value());
	REQUIRE(reread_normal_justification.has_value());
	REQUIRE(reread_normal_first_line.has_value());
	REQUIRE(reread_normal_start.has_value());
	REQUIRE(reread_normal_end.has_value());
	REQUIRE(reread_normal_space_before.has_value());
	REQUIRE(reread_normal_space_after.has_value());
	REQUIRE(reread_normal_auto_hyphenate.has_value());
	REQUIRE(reread_normal_hyphenated_word_size.has_value());
	REQUIRE(reread_normal_pre_hyphen.has_value());
	REQUIRE(reread_normal_post_hyphen.has_value());
	REQUIRE(reread_normal_consecutive_hyphens.has_value());
	REQUIRE(reread_normal_zone.has_value());
	REQUIRE(reread_normal_word_spacing.has_value());
	REQUIRE(reread_normal_letter_spacing.has_value());
	REQUIRE(reread_normal_glyph_spacing.has_value());
	REQUIRE(reread_normal_auto_leading.has_value());
	REQUIRE(reread_normal_leading_type.has_value());
	REQUIRE(reread_normal_hanging.has_value());
	REQUIRE(reread_normal_burasagari.has_value());
	REQUIRE(reread_normal_kinsoku_order.has_value());
	REQUIRE(reread_normal_every_line_composer.has_value());
	REQUIRE(reread_normal_word_spacing->size() == 3u);
	REQUIRE(reread_normal_letter_spacing->size() == 3u);
	REQUIRE(reread_normal_glyph_spacing->size() == 3u);
	REQUIRE(reread_run_justification.has_value());

	CHECK(reread_normal_sheet_index.value() == normal_sheet_index_before.value());
	CHECK(reread_normal_justification.value() == normal_justification_after);
	CHECK(doctest::Approx(reread_normal_first_line.value()).epsilon(0.0001) == normal_first_line_after);
	CHECK(doctest::Approx(reread_normal_start.value()).epsilon(0.0001) == normal_start_after);
	CHECK(doctest::Approx(reread_normal_end.value()).epsilon(0.0001) == normal_end_after);
	CHECK(doctest::Approx(reread_normal_space_before.value()).epsilon(0.0001) == normal_space_before_after);
	CHECK(doctest::Approx(reread_normal_space_after.value()).epsilon(0.0001) == normal_space_after_after);
	CHECK(reread_normal_auto_hyphenate.value() == normal_auto_hyphenate_after);
	CHECK(reread_normal_hyphenated_word_size.value() == normal_hyphenated_word_size_after);
	CHECK(reread_normal_pre_hyphen.value() == normal_pre_hyphen_after);
	CHECK(reread_normal_post_hyphen.value() == normal_post_hyphen_after);
	CHECK(reread_normal_consecutive_hyphens.value() == normal_consecutive_hyphens_after);
	CHECK(doctest::Approx(reread_normal_zone.value()).epsilon(0.0001) == normal_zone_after);
	CHECK(doctest::Approx((*reread_normal_word_spacing)[0]).epsilon(0.0001) == normal_word_spacing_after[0]);
	CHECK(doctest::Approx((*reread_normal_word_spacing)[1]).epsilon(0.0001) == normal_word_spacing_after[1]);
	CHECK(doctest::Approx((*reread_normal_word_spacing)[2]).epsilon(0.0001) == normal_word_spacing_after[2]);
	CHECK(doctest::Approx((*reread_normal_letter_spacing)[0]).epsilon(0.0001) == normal_letter_spacing_after[0]);
	CHECK(doctest::Approx((*reread_normal_letter_spacing)[1]).epsilon(0.0001) == normal_letter_spacing_after[1]);
	CHECK(doctest::Approx((*reread_normal_letter_spacing)[2]).epsilon(0.0001) == normal_letter_spacing_after[2]);
	CHECK(doctest::Approx((*reread_normal_glyph_spacing)[0]).epsilon(0.0001) == normal_glyph_spacing_after[0]);
	CHECK(doctest::Approx((*reread_normal_glyph_spacing)[1]).epsilon(0.0001) == normal_glyph_spacing_after[1]);
	CHECK(doctest::Approx((*reread_normal_glyph_spacing)[2]).epsilon(0.0001) == normal_glyph_spacing_after[2]);
	CHECK(doctest::Approx(reread_normal_auto_leading.value()).epsilon(0.0001) == normal_auto_leading_after);
	CHECK(reread_normal_leading_type.value() == normal_leading_type_after);
	CHECK(reread_normal_hanging.value() == normal_hanging_after);
	CHECK(reread_normal_burasagari.value() == normal_burasagari_after);
	CHECK(reread_normal_kinsoku_order.value() == normal_kinsoku_order_after);
	CHECK(reread_normal_every_line_composer.value() == normal_every_line_composer_after);
	// Normal sheet mutation should not implicitly rewrite explicit run properties.
	CHECK(reread_run_justification.value() == run_justification_before.value());

	std::filesystem::remove(out_path);
}

