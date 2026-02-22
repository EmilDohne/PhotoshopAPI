#include "doctest.h"
#include "TestTextLayerMutationUtils.h"

TEST_CASE("TextLayer mutates style run font size and fill color with roundtrip")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer != nullptr);

	REQUIRE(text_layer->style_run_count() == 5u);
	const auto initial_font_size = text_layer->style_run_font_size(2u);
	REQUIRE(initial_font_size.has_value());
	CHECK(doctest::Approx(initial_font_size.value()).epsilon(0.0001) == 44.0);

	const auto initial_fill = text_layer->style_run_fill_color(2u);
	REQUIRE(initial_fill.has_value());
	REQUIRE(initial_fill->size() == 4u);

	CHECK(text_layer->set_style_run_font_size(2u, 42.5));
	CHECK(text_layer->set_style_run_fill_color(2u, std::vector<double>{ 1.0, 0.2, 0.3, 0.4 }));
	CHECK_FALSE(text_layer->set_style_run_font_size(20u, 55.0));
	CHECK_FALSE(text_layer->set_style_run_fill_color(2u, std::vector<double>{ 1.0, 0.2, 0.3 }));

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_with_contents(reread, "Alpha Beta Gamma");
	REQUIRE(reread_layer != nullptr);

	const auto mutated_font_size = reread_layer->style_run_font_size(2u);
	REQUIRE(mutated_font_size.has_value());
	CHECK(doctest::Approx(mutated_font_size.value()).epsilon(0.0001) == 42.5);

	const auto mutated_fill = reread_layer->style_run_fill_color(2u);
	REQUIRE(mutated_fill.has_value());
	REQUIRE(mutated_fill->size() == 4u);
	CHECK(doctest::Approx((*mutated_fill)[0]).epsilon(0.0001) == 1.0);
	CHECK(doctest::Approx((*mutated_fill)[1]).epsilon(0.0001) == 0.2);
	CHECK(doctest::Approx((*mutated_fill)[2]).epsilon(0.0001) == 0.3);
	CHECK(doctest::Approx((*mutated_fill)[3]).epsilon(0.0001) == 0.4);

	std::filesystem::remove(out_path);
}


TEST_CASE("TextLayer mutates style run character properties with roundtrip")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_CharacterStyles.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_by_name(file, "CharacterStylePrimary");
	REQUIRE(text_layer != nullptr);

	REQUIRE(text_layer->style_run_count() >= 1u);
	const auto run_font_before = text_layer->style_run_font(0u);
	const auto run_font_size_before = text_layer->style_run_font_size(0u);
	const auto run_faux_bold_before = text_layer->style_run_faux_bold(0u);
	const auto run_faux_italic_before = text_layer->style_run_faux_italic(0u);
	const auto run_horizontal_scale_before = text_layer->style_run_horizontal_scale(0u);
	const auto run_vertical_scale_before = text_layer->style_run_vertical_scale(0u);
	const auto run_tracking_before = text_layer->style_run_tracking(0u);
	const auto run_auto_kerning_before = text_layer->style_run_auto_kerning(0u);
	const auto run_baseline_shift_before = text_layer->style_run_baseline_shift(0u);
	const auto run_leading_before = text_layer->style_run_leading(0u);
	const auto run_auto_leading_before = text_layer->style_run_auto_leading(0u);
	const auto run_kerning_before = text_layer->style_run_kerning(0u);
	const auto run_font_caps_before = text_layer->style_run_font_caps(0u);
	const auto run_no_break_before = text_layer->style_run_no_break(0u);
	const auto run_font_baseline_before = text_layer->style_run_font_baseline(0u);
	const auto run_language_before = text_layer->style_run_language(0u);
	const auto run_character_direction_before = text_layer->style_run_character_direction(0u);
	const auto run_baseline_direction_before = text_layer->style_run_baseline_direction(0u);
	const auto run_tsume_before = text_layer->style_run_tsume(0u);
	const auto run_kashida_before = text_layer->style_run_kashida(0u);
	const auto run_diacritic_pos_before = text_layer->style_run_diacritic_pos(0u);
	const auto run_ligatures_before = text_layer->style_run_ligatures(0u);
	const auto run_dligatures_before = text_layer->style_run_dligatures(0u);
	const auto run_underline_before = text_layer->style_run_underline(0u);
	const auto run_strikethrough_before = text_layer->style_run_strikethrough(0u);
	const auto run_stroke_flag_before = text_layer->style_run_stroke_flag(0u);
	const auto run_fill_flag_before = text_layer->style_run_fill_flag(0u);
	const auto run_fill_first_before = text_layer->style_run_fill_first(0u);
	const auto run_outline_width_before = text_layer->style_run_outline_width(0u);
	const auto run_fill_color_before = text_layer->style_run_fill_color(0u);
	const auto run_stroke_color_before = text_layer->style_run_stroke_color(0u);
	const auto normal_font_size_before = text_layer->style_normal_font_size();

	REQUIRE(run_font_before.has_value());
	REQUIRE(run_font_size_before.has_value());
	REQUIRE(run_faux_bold_before.has_value());
	REQUIRE(run_faux_italic_before.has_value());
	REQUIRE(run_horizontal_scale_before.has_value());
	REQUIRE(run_vertical_scale_before.has_value());
	REQUIRE(run_tracking_before.has_value());
	REQUIRE(run_auto_kerning_before.has_value());
	REQUIRE(run_baseline_shift_before.has_value());
	REQUIRE(run_leading_before.has_value());
	REQUIRE(run_auto_leading_before.has_value());
	REQUIRE(run_kerning_before.has_value());
	REQUIRE(run_font_caps_before.has_value());
	REQUIRE(run_no_break_before.has_value());
	REQUIRE(run_font_baseline_before.has_value());
	REQUIRE(run_language_before.has_value());
	REQUIRE(run_baseline_direction_before.has_value());
	REQUIRE(run_tsume_before.has_value());
	REQUIRE(run_kashida_before.has_value());
	REQUIRE(run_ligatures_before.has_value());
	REQUIRE(run_dligatures_before.has_value());
	REQUIRE(run_underline_before.has_value());
	REQUIRE(run_strikethrough_before.has_value());
	REQUIRE(run_fill_color_before.has_value());
	REQUIRE(run_fill_color_before->size() == 4u);
	if (run_stroke_color_before.has_value())
	{
		REQUIRE(run_stroke_color_before->size() == 4u);
	}
	REQUIRE(normal_font_size_before.has_value());

	const int32_t run_font_after = run_font_before.value() == 0 ? 1 : 0;
	const double run_font_size_after = run_font_size_before.value() + 2.0;
	const bool run_faux_bold_after = !run_faux_bold_before.value();
	const bool run_faux_italic_after = !run_faux_italic_before.value();
	const double run_horizontal_scale_after = run_horizontal_scale_before.value() + 0.05;
	const double run_vertical_scale_after = run_vertical_scale_before.value() + 0.05;
	const int32_t run_tracking_after = run_tracking_before.value() + 8;
	const bool run_auto_kerning_after = !run_auto_kerning_before.value();
	const double run_baseline_shift_after = run_baseline_shift_before.value() + 1.0;
	const double run_leading_after = run_leading_before.value() + 1.25;
	const bool run_auto_leading_after = !run_auto_leading_before.value();
	const int32_t run_kerning_after = run_kerning_before.value() + 12;
	const auto run_font_caps_after = (run_font_caps_before.value() == TextLayerEnum::FontCaps::Normal) ? TextLayerEnum::FontCaps::AllCaps : TextLayerEnum::FontCaps::Normal;
	const bool run_no_break_after = !run_no_break_before.value();
	const auto run_font_baseline_after = (run_font_baseline_before.value() == TextLayerEnum::FontBaseline::Normal) ? TextLayerEnum::FontBaseline::Superscript : TextLayerEnum::FontBaseline::Normal;
	const int32_t run_language_after = run_language_before.value() + 1;
	const std::optional<TextLayerEnum::CharacterDirection> run_character_direction_after = run_character_direction_before.has_value()
		? std::optional<TextLayerEnum::CharacterDirection>(run_character_direction_before.value() == TextLayerEnum::CharacterDirection::Default ? TextLayerEnum::CharacterDirection::LeftToRight : TextLayerEnum::CharacterDirection::Default)
		: std::nullopt;
	const auto run_baseline_direction_after = (run_baseline_direction_before.value() == TextLayerEnum::BaselineDirection::Default) ? TextLayerEnum::BaselineDirection::Vertical : TextLayerEnum::BaselineDirection::Default;
	const double run_tsume_after = run_tsume_before.value() + 5.0;
	const int32_t run_kashida_after = run_kashida_before.value() + 1;
	const std::optional<TextLayerEnum::DiacriticPosition> run_diacritic_pos_after = run_diacritic_pos_before.has_value()
		? std::optional<TextLayerEnum::DiacriticPosition>(run_diacritic_pos_before.value() == TextLayerEnum::DiacriticPosition::OpenType ? TextLayerEnum::DiacriticPosition::Loose : TextLayerEnum::DiacriticPosition::OpenType)
		: std::nullopt;
	const bool run_ligatures_after = !run_ligatures_before.value();
	const bool run_dligatures_after = !run_dligatures_before.value();
	const bool run_underline_after = !run_underline_before.value();
	const bool run_strikethrough_after = !run_strikethrough_before.value();
	const std::optional<bool> run_stroke_flag_after = run_stroke_flag_before.has_value()
		? std::optional<bool>(!run_stroke_flag_before.value())
		: std::nullopt;
	const std::optional<bool> run_fill_flag_after = run_fill_flag_before.has_value()
		? std::optional<bool>(!run_fill_flag_before.value())
		: std::nullopt;
	const std::optional<bool> run_fill_first_after = run_fill_first_before.has_value()
		? std::optional<bool>(!run_fill_first_before.value())
		: std::nullopt;
	const std::optional<double> run_outline_width_after = run_outline_width_before.has_value()
		? std::optional<double>(run_outline_width_before.value() + 1.0)
		: std::nullopt;
	const TextLayerEnum::CharacterDirection expected_character_direction_after = run_character_direction_after.value_or(TextLayerEnum::CharacterDirection::LeftToRight);
	const TextLayerEnum::DiacriticPosition expected_diacritic_pos_after = run_diacritic_pos_after.value_or(TextLayerEnum::DiacriticPosition::Loose);
	const double expected_outline_width_after = run_outline_width_after.value_or(2.0);
	const std::vector<double> run_fill_color_after{
		(*run_fill_color_before)[0],
		std::clamp((*run_fill_color_before)[1] + 0.05, 0.0, 1.0),
		std::clamp((*run_fill_color_before)[2] + 0.05, 0.0, 1.0),
		std::clamp((*run_fill_color_before)[3] + 0.05, 0.0, 1.0)
	};
	std::optional<std::vector<double>> run_stroke_color_after = std::nullopt;
	if (run_stroke_color_before.has_value() && run_stroke_color_before->size() == 4u)
	{
		run_stroke_color_after = std::vector<double>{
			(*run_stroke_color_before)[0],
			std::clamp((*run_stroke_color_before)[1] + 0.05, 0.0, 1.0),
			std::clamp((*run_stroke_color_before)[2] + 0.05, 0.0, 1.0),
			std::clamp((*run_stroke_color_before)[3] + 0.05, 0.0, 1.0)
		};
	}

	CHECK(text_layer->set_style_run_font(0u, run_font_after));
	CHECK(text_layer->set_style_run_font_size(0u, run_font_size_after));
	CHECK(text_layer->set_style_run_faux_bold(0u, run_faux_bold_after));
	CHECK(text_layer->set_style_run_faux_italic(0u, run_faux_italic_after));
	CHECK(text_layer->set_style_run_horizontal_scale(0u, run_horizontal_scale_after));
	CHECK(text_layer->set_style_run_vertical_scale(0u, run_vertical_scale_after));
	CHECK(text_layer->set_style_run_tracking(0u, run_tracking_after));
	CHECK(text_layer->set_style_run_auto_kerning(0u, run_auto_kerning_after));
	CHECK(text_layer->set_style_run_baseline_shift(0u, run_baseline_shift_after));
	CHECK(text_layer->set_style_run_leading(0u, run_leading_after));
	CHECK(text_layer->set_style_run_auto_leading(0u, run_auto_leading_after));
	CHECK(text_layer->set_style_run_kerning(0u, run_kerning_after));
	CHECK(text_layer->set_style_run_font_caps(0u, run_font_caps_after));
	CHECK(text_layer->set_style_run_no_break(0u, run_no_break_after));
	CHECK(text_layer->set_style_run_font_baseline(0u, run_font_baseline_after));
	CHECK(text_layer->set_style_run_language(0u, run_language_after));
	CHECK(text_layer->set_style_run_character_direction(0u, expected_character_direction_after));
	CHECK(text_layer->set_style_run_baseline_direction(0u, run_baseline_direction_after));
	CHECK(text_layer->set_style_run_tsume(0u, run_tsume_after));
	CHECK(text_layer->set_style_run_kashida(0u, run_kashida_after));
	CHECK(text_layer->set_style_run_diacritic_pos(0u, expected_diacritic_pos_after));
	CHECK(text_layer->set_style_run_ligatures(0u, run_ligatures_after));
	CHECK(text_layer->set_style_run_dligatures(0u, run_dligatures_after));
	CHECK(text_layer->set_style_run_underline(0u, run_underline_after));
	CHECK(text_layer->set_style_run_strikethrough(0u, run_strikethrough_after));
	if (run_stroke_flag_after.has_value())
	{
		CHECK(text_layer->set_style_run_stroke_flag(0u, run_stroke_flag_after.value()));
	}
	else
	{
		CHECK(text_layer->set_style_run_stroke_flag(0u, true));
	}
	if (run_fill_flag_after.has_value())
	{
		CHECK(text_layer->set_style_run_fill_flag(0u, run_fill_flag_after.value()));
	}
	else
	{
		CHECK(text_layer->set_style_run_fill_flag(0u, true));
	}
	if (run_fill_first_after.has_value())
	{
		CHECK(text_layer->set_style_run_fill_first(0u, run_fill_first_after.value()));
	}
	else
	{
		CHECK(text_layer->set_style_run_fill_first(0u, false));
	}
	CHECK(text_layer->set_style_run_outline_width(0u, expected_outline_width_after));
	CHECK(text_layer->set_style_run_fill_color(0u, run_fill_color_after));
	if (run_stroke_color_after.has_value())
	{
		CHECK(text_layer->set_style_run_stroke_color(0u, run_stroke_color_after.value()));
	}
	else
	{
		CHECK_FALSE(text_layer->set_style_run_stroke_color(0u, std::vector<double>{ 1.0, 0.1, 0.2, 0.3 }));
	}
	CHECK_FALSE(text_layer->set_style_run_font(200u, run_font_after));
	CHECK_FALSE(text_layer->set_style_run_font_size(0u, std::numeric_limits<double>::infinity()));
	CHECK_FALSE(text_layer->set_style_run_leading(200u, run_leading_after));
	CHECK_FALSE(text_layer->set_style_run_auto_leading(200u, run_auto_leading_after));
	CHECK_FALSE(text_layer->set_style_run_kerning(200u, run_kerning_after));
	CHECK_FALSE(text_layer->set_style_run_font_baseline(200u, run_font_baseline_after));
	CHECK_FALSE(text_layer->set_style_run_language(200u, run_language_after));
	CHECK_FALSE(text_layer->set_style_run_baseline_direction(200u, run_baseline_direction_after));
	CHECK_FALSE(text_layer->set_style_run_tsume(200u, run_tsume_after));
	CHECK_FALSE(text_layer->set_style_run_kashida(200u, run_kashida_after));
	CHECK_FALSE(text_layer->set_style_run_stroke_flag(200u, true));
	CHECK_FALSE(text_layer->set_style_run_fill_flag(200u, true));
	CHECK_FALSE(text_layer->set_style_run_fill_first(200u, true));
	CHECK_FALSE(text_layer->set_style_run_outline_width(200u, 1.0));
	CHECK_FALSE(text_layer->set_style_run_fill_color(0u, {}));
	CHECK_FALSE(text_layer->set_style_run_fill_color(0u, std::vector<double>{ 1.0, 0.0, 0.0 }));
	CHECK_FALSE(text_layer->set_style_run_stroke_color(0u, {}));
	CHECK_FALSE(text_layer->set_style_run_stroke_color(0u, std::vector<double>{ 1.0, 0.0, 0.0 }));

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_by_name(reread, "CharacterStylePrimary");
	REQUIRE(reread_layer != nullptr);

	const auto reread_run_font = reread_layer->style_run_font(0u);
	const auto reread_run_font_size = reread_layer->style_run_font_size(0u);
	const auto reread_run_faux_bold = reread_layer->style_run_faux_bold(0u);
	const auto reread_run_faux_italic = reread_layer->style_run_faux_italic(0u);
	const auto reread_run_horizontal_scale = reread_layer->style_run_horizontal_scale(0u);
	const auto reread_run_vertical_scale = reread_layer->style_run_vertical_scale(0u);
	const auto reread_run_tracking = reread_layer->style_run_tracking(0u);
	const auto reread_run_auto_kerning = reread_layer->style_run_auto_kerning(0u);
	const auto reread_run_baseline_shift = reread_layer->style_run_baseline_shift(0u);
	const auto reread_run_leading = reread_layer->style_run_leading(0u);
	const auto reread_run_auto_leading = reread_layer->style_run_auto_leading(0u);
	const auto reread_run_kerning = reread_layer->style_run_kerning(0u);
	const auto reread_run_font_caps = reread_layer->style_run_font_caps(0u);
	const auto reread_run_no_break = reread_layer->style_run_no_break(0u);
	const auto reread_run_font_baseline = reread_layer->style_run_font_baseline(0u);
	const auto reread_run_language = reread_layer->style_run_language(0u);
	const auto reread_run_character_direction = reread_layer->style_run_character_direction(0u);
	const auto reread_run_baseline_direction = reread_layer->style_run_baseline_direction(0u);
	const auto reread_run_tsume = reread_layer->style_run_tsume(0u);
	const auto reread_run_kashida = reread_layer->style_run_kashida(0u);
	const auto reread_run_diacritic_pos = reread_layer->style_run_diacritic_pos(0u);
	const auto reread_run_ligatures = reread_layer->style_run_ligatures(0u);
	const auto reread_run_dligatures = reread_layer->style_run_dligatures(0u);
	const auto reread_run_underline = reread_layer->style_run_underline(0u);
	const auto reread_run_strikethrough = reread_layer->style_run_strikethrough(0u);
	const auto reread_run_stroke_flag = reread_layer->style_run_stroke_flag(0u);
	const auto reread_run_fill_flag = reread_layer->style_run_fill_flag(0u);
	const auto reread_run_fill_first = reread_layer->style_run_fill_first(0u);
	const auto reread_run_outline_width = reread_layer->style_run_outline_width(0u);
	const auto reread_run_fill_color = reread_layer->style_run_fill_color(0u);
	const auto reread_run_stroke_color = reread_layer->style_run_stroke_color(0u);
	const auto reread_normal_font_size = reread_layer->style_normal_font_size();

	REQUIRE(reread_run_font.has_value());
	REQUIRE(reread_run_font_size.has_value());
	REQUIRE(reread_run_faux_bold.has_value());
	REQUIRE(reread_run_faux_italic.has_value());
	REQUIRE(reread_run_horizontal_scale.has_value());
	REQUIRE(reread_run_vertical_scale.has_value());
	REQUIRE(reread_run_tracking.has_value());
	REQUIRE(reread_run_auto_kerning.has_value());
	REQUIRE(reread_run_baseline_shift.has_value());
	REQUIRE(reread_run_leading.has_value());
	REQUIRE(reread_run_auto_leading.has_value());
	REQUIRE(reread_run_kerning.has_value());
	REQUIRE(reread_run_font_caps.has_value());
	REQUIRE(reread_run_no_break.has_value());
	REQUIRE(reread_run_font_baseline.has_value());
	REQUIRE(reread_run_language.has_value());
	REQUIRE(reread_run_baseline_direction.has_value());
	REQUIRE(reread_run_tsume.has_value());
	REQUIRE(reread_run_kashida.has_value());
	REQUIRE(reread_run_ligatures.has_value());
	REQUIRE(reread_run_dligatures.has_value());
	REQUIRE(reread_run_underline.has_value());
	REQUIRE(reread_run_strikethrough.has_value());
	REQUIRE(reread_run_fill_color.has_value());
	REQUIRE(reread_run_fill_color->size() == 4u);
	REQUIRE(reread_normal_font_size.has_value());

	CHECK(reread_run_font.value() == run_font_after);
	CHECK(doctest::Approx(reread_run_font_size.value()).epsilon(0.0001) == run_font_size_after);
	CHECK(reread_run_faux_bold.value() == run_faux_bold_after);
	CHECK(reread_run_faux_italic.value() == run_faux_italic_after);
	CHECK(doctest::Approx(reread_run_horizontal_scale.value()).epsilon(0.0001) == run_horizontal_scale_after);
	CHECK(doctest::Approx(reread_run_vertical_scale.value()).epsilon(0.0001) == run_vertical_scale_after);
	CHECK(reread_run_tracking.value() == run_tracking_after);
	CHECK(reread_run_auto_kerning.value() == run_auto_kerning_after);
	CHECK(doctest::Approx(reread_run_baseline_shift.value()).epsilon(0.0001) == run_baseline_shift_after);
	CHECK(doctest::Approx(reread_run_leading.value()).epsilon(0.0001) == run_leading_after);
	CHECK(reread_run_auto_leading.value() == run_auto_leading_after);
	CHECK(reread_run_kerning.value() == run_kerning_after);
	CHECK(reread_run_font_caps.value() == run_font_caps_after);
	CHECK(reread_run_no_break.value() == run_no_break_after);
	CHECK(reread_run_font_baseline.value() == run_font_baseline_after);
	CHECK(reread_run_language.value() == run_language_after);
	REQUIRE(reread_run_character_direction.has_value());
	CHECK(reread_run_character_direction.value() == expected_character_direction_after);
	CHECK(reread_run_baseline_direction.value() == run_baseline_direction_after);
	CHECK(doctest::Approx(reread_run_tsume.value()).epsilon(0.0001) == run_tsume_after);
	CHECK(reread_run_kashida.value() == run_kashida_after);
	REQUIRE(reread_run_diacritic_pos.has_value());
	CHECK(reread_run_diacritic_pos.value() == expected_diacritic_pos_after);
	CHECK(reread_run_ligatures.value() == run_ligatures_after);
	CHECK(reread_run_dligatures.value() == run_dligatures_after);
	CHECK(reread_run_underline.value() == run_underline_after);
	CHECK(reread_run_strikethrough.value() == run_strikethrough_after);
	if (run_stroke_flag_after.has_value())
	{
		REQUIRE(reread_run_stroke_flag.has_value());
		CHECK(reread_run_stroke_flag.value() == run_stroke_flag_after.value());
	}
	else
	{
		REQUIRE(reread_run_stroke_flag.has_value());
		CHECK(reread_run_stroke_flag.value());
	}
	if (run_fill_flag_after.has_value())
	{
		REQUIRE(reread_run_fill_flag.has_value());
		CHECK(reread_run_fill_flag.value() == run_fill_flag_after.value());
	}
	else
	{
		REQUIRE(reread_run_fill_flag.has_value());
		CHECK(reread_run_fill_flag.value());
	}
	if (run_fill_first_after.has_value())
	{
		REQUIRE(reread_run_fill_first.has_value());
		CHECK(reread_run_fill_first.value() == run_fill_first_after.value());
	}
	else
	{
		REQUIRE(reread_run_fill_first.has_value());
		CHECK_FALSE(reread_run_fill_first.value());
	}
	REQUIRE(reread_run_outline_width.has_value());
	CHECK(doctest::Approx(reread_run_outline_width.value()).epsilon(0.0001) == expected_outline_width_after);
	CHECK(doctest::Approx((*reread_run_fill_color)[0]).epsilon(0.0001) == run_fill_color_after[0]);
	CHECK(doctest::Approx((*reread_run_fill_color)[1]).epsilon(0.0001) == run_fill_color_after[1]);
	CHECK(doctest::Approx((*reread_run_fill_color)[2]).epsilon(0.0001) == run_fill_color_after[2]);
	CHECK(doctest::Approx((*reread_run_fill_color)[3]).epsilon(0.0001) == run_fill_color_after[3]);
	if (run_stroke_color_after.has_value())
	{
		REQUIRE(reread_run_stroke_color.has_value());
		REQUIRE(reread_run_stroke_color->size() == 4u);
		CHECK(doctest::Approx((*reread_run_stroke_color)[0]).epsilon(0.0001) == (*run_stroke_color_after)[0]);
		CHECK(doctest::Approx((*reread_run_stroke_color)[1]).epsilon(0.0001) == (*run_stroke_color_after)[1]);
		CHECK(doctest::Approx((*reread_run_stroke_color)[2]).epsilon(0.0001) == (*run_stroke_color_after)[2]);
		CHECK(doctest::Approx((*reread_run_stroke_color)[3]).epsilon(0.0001) == (*run_stroke_color_after)[3]);
	}
	else
	{
		CHECK_FALSE(reread_run_stroke_color.has_value());
	}
	// Style-run mutation should not rewrite normal style-sheet defaults.
	CHECK(doctest::Approx(reread_normal_font_size.value()).epsilon(0.0001) == normal_font_size_before.value());

	std::filesystem::remove(out_path);
}


TEST_CASE("TextLayer mutates normal style sheet properties with roundtrip")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_CharacterStyles.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_by_name(file, "CharacterStylePrimary");
	REQUIRE(text_layer != nullptr);

	REQUIRE(text_layer->style_sheet_count() >= 1u);
	const auto normal_sheet_index_before = text_layer->style_normal_sheet_index();
	const auto normal_font_before = text_layer->style_normal_font();
	const auto normal_font_size_before = text_layer->style_normal_font_size();
	const auto normal_leading_before = text_layer->style_normal_leading();
	const auto normal_auto_leading_before = text_layer->style_normal_auto_leading();
	const auto normal_kerning_before = text_layer->style_normal_kerning();
	const auto normal_faux_bold_before = text_layer->style_normal_faux_bold();
	const auto normal_faux_italic_before = text_layer->style_normal_faux_italic();
	const auto normal_horizontal_scale_before = text_layer->style_normal_horizontal_scale();
	const auto normal_vertical_scale_before = text_layer->style_normal_vertical_scale();
	const auto normal_tracking_before = text_layer->style_normal_tracking();
	const auto normal_auto_kerning_before = text_layer->style_normal_auto_kerning();
	const auto normal_baseline_shift_before = text_layer->style_normal_baseline_shift();
	const auto normal_font_caps_before = text_layer->style_normal_font_caps();
	const auto normal_font_baseline_before = text_layer->style_normal_font_baseline();
	const auto normal_no_break_before = text_layer->style_normal_no_break();
	const auto normal_language_before = text_layer->style_normal_language();
	const auto normal_character_direction_before = text_layer->style_normal_character_direction();
	const auto normal_baseline_direction_before = text_layer->style_normal_baseline_direction();
	const auto normal_tsume_before = text_layer->style_normal_tsume();
	const auto normal_kashida_before = text_layer->style_normal_kashida();
	const auto normal_diacritic_pos_before = text_layer->style_normal_diacritic_pos();
	const auto normal_ligatures_before = text_layer->style_normal_ligatures();
	const auto normal_dligatures_before = text_layer->style_normal_dligatures();
	const auto normal_underline_before = text_layer->style_normal_underline();
	const auto normal_strikethrough_before = text_layer->style_normal_strikethrough();
	const auto normal_stroke_flag_before = text_layer->style_normal_stroke_flag();
	const auto normal_fill_flag_before = text_layer->style_normal_fill_flag();
	const auto normal_fill_first_before = text_layer->style_normal_fill_first();
	const auto normal_outline_width_before = text_layer->style_normal_outline_width();
	const auto normal_fill_color_before = text_layer->style_normal_fill_color();
	const auto normal_stroke_color_before = text_layer->style_normal_stroke_color();
	const auto run_font_size_before = text_layer->style_run_font_size(0u);

	REQUIRE(normal_sheet_index_before.has_value());
	REQUIRE(normal_font_before.has_value());
	REQUIRE(normal_font_size_before.has_value());
	REQUIRE(normal_leading_before.has_value());
	REQUIRE(normal_auto_leading_before.has_value());
	REQUIRE(normal_kerning_before.has_value());
	REQUIRE(normal_faux_bold_before.has_value());
	REQUIRE(normal_faux_italic_before.has_value());
	REQUIRE(normal_horizontal_scale_before.has_value());
	REQUIRE(normal_vertical_scale_before.has_value());
	REQUIRE(normal_tracking_before.has_value());
	REQUIRE(normal_auto_kerning_before.has_value());
	REQUIRE(normal_baseline_shift_before.has_value());
	REQUIRE(normal_font_caps_before.has_value());
	REQUIRE(normal_font_baseline_before.has_value());
	REQUIRE(normal_no_break_before.has_value());
	REQUIRE(normal_language_before.has_value());
	REQUIRE(normal_character_direction_before.has_value());
	REQUIRE(normal_baseline_direction_before.has_value());
	REQUIRE(normal_tsume_before.has_value());
	REQUIRE(normal_kashida_before.has_value());
	REQUIRE(normal_diacritic_pos_before.has_value());
	REQUIRE(normal_ligatures_before.has_value());
	REQUIRE(normal_dligatures_before.has_value());
	REQUIRE(normal_underline_before.has_value());
	REQUIRE(normal_strikethrough_before.has_value());
	REQUIRE(normal_stroke_flag_before.has_value());
	REQUIRE(normal_fill_flag_before.has_value());
	REQUIRE(normal_fill_first_before.has_value());
	REQUIRE(normal_outline_width_before.has_value());
	REQUIRE(normal_fill_color_before.has_value());
	REQUIRE(normal_fill_color_before->size() == 4u);
	REQUIRE(normal_stroke_color_before.has_value());
	REQUIRE(normal_stroke_color_before->size() == 4u);
	REQUIRE(run_font_size_before.has_value());

	const int32_t normal_font_after = normal_font_before.value() == 0 ? 1 : 0;
	const double normal_font_size_after = normal_font_size_before.value() + 3.0;
	const double normal_leading_after = normal_leading_before.value() + 2.0;
	const bool normal_auto_leading_after = !normal_auto_leading_before.value();
	const int32_t normal_kerning_after = normal_kerning_before.value() + 12;
	const bool normal_faux_bold_after = !normal_faux_bold_before.value();
	const bool normal_faux_italic_after = !normal_faux_italic_before.value();
	const double normal_horizontal_scale_after = normal_horizontal_scale_before.value() + 0.05;
	const double normal_vertical_scale_after = normal_vertical_scale_before.value() + 0.05;
	const int32_t normal_tracking_after = normal_tracking_before.value() + 12;
	const bool normal_auto_kerning_after = !normal_auto_kerning_before.value();
	const double normal_baseline_shift_after = normal_baseline_shift_before.value() + 2.5;
	const auto normal_font_caps_after = (normal_font_caps_before.value() == TextLayerEnum::FontCaps::Normal) ? TextLayerEnum::FontCaps::AllCaps : TextLayerEnum::FontCaps::Normal;
	const auto normal_font_baseline_after = (normal_font_baseline_before.value() == TextLayerEnum::FontBaseline::Normal) ? TextLayerEnum::FontBaseline::Superscript : TextLayerEnum::FontBaseline::Normal;
	const bool normal_no_break_after = !normal_no_break_before.value();
	const int32_t normal_language_after = normal_language_before.value() + 1;
	const auto normal_character_direction_after = (normal_character_direction_before.value() == TextLayerEnum::CharacterDirection::Default) ? TextLayerEnum::CharacterDirection::LeftToRight : TextLayerEnum::CharacterDirection::Default;
	const auto normal_baseline_direction_after = (normal_baseline_direction_before.value() == TextLayerEnum::BaselineDirection::Default) ? TextLayerEnum::BaselineDirection::Vertical : TextLayerEnum::BaselineDirection::Default;
	const double normal_tsume_after = normal_tsume_before.value() + 5.0;
	const int32_t normal_kashida_after = normal_kashida_before.value() + 1;
	const auto normal_diacritic_pos_after = (normal_diacritic_pos_before.value() == TextLayerEnum::DiacriticPosition::OpenType) ? TextLayerEnum::DiacriticPosition::Loose : TextLayerEnum::DiacriticPosition::OpenType;
	const bool normal_ligatures_after = !normal_ligatures_before.value();
	const bool normal_dligatures_after = !normal_dligatures_before.value();
	const bool normal_underline_after = !normal_underline_before.value();
	const bool normal_strikethrough_after = !normal_strikethrough_before.value();
	const bool normal_stroke_flag_after = !normal_stroke_flag_before.value();
	const bool normal_fill_flag_after = !normal_fill_flag_before.value();
	const bool normal_fill_first_after = !normal_fill_first_before.value();
	const double normal_outline_width_after = normal_outline_width_before.value() + 1.0;
	const std::vector<double> normal_fill_color_after{
		(*normal_fill_color_before)[0],
		std::clamp((*normal_fill_color_before)[1] + 0.1, 0.0, 1.0),
		std::clamp((*normal_fill_color_before)[2] + 0.1, 0.0, 1.0),
		std::clamp((*normal_fill_color_before)[3] + 0.1, 0.0, 1.0)
	};
	const std::vector<double> normal_stroke_color_after{
		(*normal_stroke_color_before)[0],
		std::clamp((*normal_stroke_color_before)[1] + 0.1, 0.0, 1.0),
		std::clamp((*normal_stroke_color_before)[2] + 0.1, 0.0, 1.0),
		std::clamp((*normal_stroke_color_before)[3] + 0.1, 0.0, 1.0)
	};

	CHECK(text_layer->set_style_normal_sheet_index(normal_sheet_index_before.value()));
	CHECK(text_layer->set_style_normal_font(normal_font_after));
	CHECK(text_layer->set_style_normal_font_size(normal_font_size_after));
	CHECK(text_layer->set_style_normal_leading(normal_leading_after));
	CHECK(text_layer->set_style_normal_auto_leading(normal_auto_leading_after));
	CHECK(text_layer->set_style_normal_kerning(normal_kerning_after));
	CHECK(text_layer->set_style_normal_faux_bold(normal_faux_bold_after));
	CHECK(text_layer->set_style_normal_faux_italic(normal_faux_italic_after));
	CHECK(text_layer->set_style_normal_horizontal_scale(normal_horizontal_scale_after));
	CHECK(text_layer->set_style_normal_vertical_scale(normal_vertical_scale_after));
	CHECK(text_layer->set_style_normal_tracking(normal_tracking_after));
	CHECK(text_layer->set_style_normal_auto_kerning(normal_auto_kerning_after));
	CHECK(text_layer->set_style_normal_baseline_shift(normal_baseline_shift_after));
	CHECK(text_layer->set_style_normal_font_caps(normal_font_caps_after));
	CHECK(text_layer->set_style_normal_font_baseline(normal_font_baseline_after));
	CHECK(text_layer->set_style_normal_no_break(normal_no_break_after));
	CHECK(text_layer->set_style_normal_language(normal_language_after));
	CHECK(text_layer->set_style_normal_character_direction(normal_character_direction_after));
	CHECK(text_layer->set_style_normal_baseline_direction(normal_baseline_direction_after));
	CHECK(text_layer->set_style_normal_tsume(normal_tsume_after));
	CHECK(text_layer->set_style_normal_kashida(normal_kashida_after));
	CHECK(text_layer->set_style_normal_diacritic_pos(normal_diacritic_pos_after));
	CHECK(text_layer->set_style_normal_ligatures(normal_ligatures_after));
	CHECK(text_layer->set_style_normal_dligatures(normal_dligatures_after));
	CHECK(text_layer->set_style_normal_underline(normal_underline_after));
	CHECK(text_layer->set_style_normal_strikethrough(normal_strikethrough_after));
	CHECK(text_layer->set_style_normal_stroke_flag(normal_stroke_flag_after));
	CHECK(text_layer->set_style_normal_fill_flag(normal_fill_flag_after));
	CHECK(text_layer->set_style_normal_fill_first(normal_fill_first_after));
	CHECK(text_layer->set_style_normal_outline_width(normal_outline_width_after));
	CHECK(text_layer->set_style_normal_fill_color(normal_fill_color_after));
	CHECK(text_layer->set_style_normal_stroke_color(normal_stroke_color_after));
	CHECK_FALSE(text_layer->set_style_normal_sheet_index(-1));
	CHECK_FALSE(text_layer->set_style_normal_sheet_index(200));
	CHECK_FALSE(text_layer->set_style_normal_font_size(std::numeric_limits<double>::infinity()));
	CHECK_FALSE(text_layer->set_style_normal_fill_color({}));
	CHECK_FALSE(text_layer->set_style_normal_fill_color(std::vector<double>{ 1.0, 0.0, 0.0 }));
	CHECK_FALSE(text_layer->set_style_normal_fill_color(std::vector<double>{ 1.0, 0.0, std::numeric_limits<double>::infinity(), 0.0 }));
	CHECK_FALSE(text_layer->set_style_normal_stroke_color({}));
	CHECK_FALSE(text_layer->set_style_normal_stroke_color(std::vector<double>{ 1.0, 0.0, 0.0 }));
	CHECK_FALSE(text_layer->set_style_normal_stroke_color(std::vector<double>{ 1.0, 0.0, std::numeric_limits<double>::infinity(), 0.0 }));

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_by_name(reread, "CharacterStylePrimary");
	REQUIRE(reread_layer != nullptr);

	const auto reread_normal_sheet_index = reread_layer->style_normal_sheet_index();
	const auto reread_normal_font = reread_layer->style_normal_font();
	const auto reread_normal_font_size = reread_layer->style_normal_font_size();
	const auto reread_normal_leading = reread_layer->style_normal_leading();
	const auto reread_normal_auto_leading = reread_layer->style_normal_auto_leading();
	const auto reread_normal_kerning = reread_layer->style_normal_kerning();
	const auto reread_normal_faux_bold = reread_layer->style_normal_faux_bold();
	const auto reread_normal_faux_italic = reread_layer->style_normal_faux_italic();
	const auto reread_normal_horizontal_scale = reread_layer->style_normal_horizontal_scale();
	const auto reread_normal_vertical_scale = reread_layer->style_normal_vertical_scale();
	const auto reread_normal_tracking = reread_layer->style_normal_tracking();
	const auto reread_normal_auto_kerning = reread_layer->style_normal_auto_kerning();
	const auto reread_normal_baseline_shift = reread_layer->style_normal_baseline_shift();
	const auto reread_normal_font_caps = reread_layer->style_normal_font_caps();
	const auto reread_normal_font_baseline = reread_layer->style_normal_font_baseline();
	const auto reread_normal_no_break = reread_layer->style_normal_no_break();
	const auto reread_normal_language = reread_layer->style_normal_language();
	const auto reread_normal_character_direction = reread_layer->style_normal_character_direction();
	const auto reread_normal_baseline_direction = reread_layer->style_normal_baseline_direction();
	const auto reread_normal_tsume = reread_layer->style_normal_tsume();
	const auto reread_normal_kashida = reread_layer->style_normal_kashida();
	const auto reread_normal_diacritic_pos = reread_layer->style_normal_diacritic_pos();
	const auto reread_normal_ligatures = reread_layer->style_normal_ligatures();
	const auto reread_normal_dligatures = reread_layer->style_normal_dligatures();
	const auto reread_normal_underline = reread_layer->style_normal_underline();
	const auto reread_normal_strikethrough = reread_layer->style_normal_strikethrough();
	const auto reread_normal_stroke_flag = reread_layer->style_normal_stroke_flag();
	const auto reread_normal_fill_flag = reread_layer->style_normal_fill_flag();
	const auto reread_normal_fill_first = reread_layer->style_normal_fill_first();
	const auto reread_normal_outline_width = reread_layer->style_normal_outline_width();
	const auto reread_normal_fill_color = reread_layer->style_normal_fill_color();
	const auto reread_normal_stroke_color = reread_layer->style_normal_stroke_color();
	const auto reread_run_font_size = reread_layer->style_run_font_size(0u);

	REQUIRE(reread_normal_sheet_index.has_value());
	REQUIRE(reread_normal_font.has_value());
	REQUIRE(reread_normal_font_size.has_value());
	REQUIRE(reread_normal_leading.has_value());
	REQUIRE(reread_normal_auto_leading.has_value());
	REQUIRE(reread_normal_kerning.has_value());
	REQUIRE(reread_normal_faux_bold.has_value());
	REQUIRE(reread_normal_faux_italic.has_value());
	REQUIRE(reread_normal_horizontal_scale.has_value());
	REQUIRE(reread_normal_vertical_scale.has_value());
	REQUIRE(reread_normal_tracking.has_value());
	REQUIRE(reread_normal_auto_kerning.has_value());
	REQUIRE(reread_normal_baseline_shift.has_value());
	REQUIRE(reread_normal_font_caps.has_value());
	REQUIRE(reread_normal_font_baseline.has_value());
	REQUIRE(reread_normal_no_break.has_value());
	REQUIRE(reread_normal_language.has_value());
	REQUIRE(reread_normal_character_direction.has_value());
	REQUIRE(reread_normal_baseline_direction.has_value());
	REQUIRE(reread_normal_tsume.has_value());
	REQUIRE(reread_normal_kashida.has_value());
	REQUIRE(reread_normal_diacritic_pos.has_value());
	REQUIRE(reread_normal_ligatures.has_value());
	REQUIRE(reread_normal_dligatures.has_value());
	REQUIRE(reread_normal_underline.has_value());
	REQUIRE(reread_normal_strikethrough.has_value());
	REQUIRE(reread_normal_stroke_flag.has_value());
	REQUIRE(reread_normal_fill_flag.has_value());
	REQUIRE(reread_normal_fill_first.has_value());
	REQUIRE(reread_normal_outline_width.has_value());
	REQUIRE(reread_normal_fill_color.has_value());
	REQUIRE(reread_normal_fill_color->size() == 4u);
	REQUIRE(reread_normal_stroke_color.has_value());
	REQUIRE(reread_normal_stroke_color->size() == 4u);
	REQUIRE(reread_run_font_size.has_value());

	CHECK(reread_normal_sheet_index.value() == normal_sheet_index_before.value());
	CHECK(reread_normal_font.value() == normal_font_after);
	CHECK(doctest::Approx(reread_normal_font_size.value()).epsilon(0.0001) == normal_font_size_after);
	CHECK(doctest::Approx(reread_normal_leading.value()).epsilon(0.0001) == normal_leading_after);
	CHECK(reread_normal_auto_leading.value() == normal_auto_leading_after);
	CHECK(reread_normal_kerning.value() == normal_kerning_after);
	CHECK(reread_normal_faux_bold.value() == normal_faux_bold_after);
	CHECK(reread_normal_faux_italic.value() == normal_faux_italic_after);
	CHECK(doctest::Approx(reread_normal_horizontal_scale.value()).epsilon(0.0001) == normal_horizontal_scale_after);
	CHECK(doctest::Approx(reread_normal_vertical_scale.value()).epsilon(0.0001) == normal_vertical_scale_after);
	CHECK(reread_normal_tracking.value() == normal_tracking_after);
	CHECK(reread_normal_auto_kerning.value() == normal_auto_kerning_after);
	CHECK(doctest::Approx(reread_normal_baseline_shift.value()).epsilon(0.0001) == normal_baseline_shift_after);
	CHECK(reread_normal_font_caps.value() == normal_font_caps_after);
	CHECK(reread_normal_font_baseline.value() == normal_font_baseline_after);
	CHECK(reread_normal_no_break.value() == normal_no_break_after);
	CHECK(reread_normal_language.value() == normal_language_after);
	CHECK(reread_normal_character_direction.value() == normal_character_direction_after);
	CHECK(reread_normal_baseline_direction.value() == normal_baseline_direction_after);
	CHECK(doctest::Approx(reread_normal_tsume.value()).epsilon(0.0001) == normal_tsume_after);
	CHECK(reread_normal_kashida.value() == normal_kashida_after);
	CHECK(reread_normal_diacritic_pos.value() == normal_diacritic_pos_after);
	CHECK(reread_normal_ligatures.value() == normal_ligatures_after);
	CHECK(reread_normal_dligatures.value() == normal_dligatures_after);
	CHECK(reread_normal_underline.value() == normal_underline_after);
	CHECK(reread_normal_strikethrough.value() == normal_strikethrough_after);
	CHECK(reread_normal_stroke_flag.value() == normal_stroke_flag_after);
	CHECK(reread_normal_fill_flag.value() == normal_fill_flag_after);
	CHECK(reread_normal_fill_first.value() == normal_fill_first_after);
	CHECK(doctest::Approx(reread_normal_outline_width.value()).epsilon(0.0001) == normal_outline_width_after);
	CHECK(doctest::Approx((*reread_normal_fill_color)[0]).epsilon(0.0001) == normal_fill_color_after[0]);
	CHECK(doctest::Approx((*reread_normal_fill_color)[1]).epsilon(0.0001) == normal_fill_color_after[1]);
	CHECK(doctest::Approx((*reread_normal_fill_color)[2]).epsilon(0.0001) == normal_fill_color_after[2]);
	CHECK(doctest::Approx((*reread_normal_fill_color)[3]).epsilon(0.0001) == normal_fill_color_after[3]);
	CHECK(doctest::Approx((*reread_normal_stroke_color)[0]).epsilon(0.0001) == normal_stroke_color_after[0]);
	CHECK(doctest::Approx((*reread_normal_stroke_color)[1]).epsilon(0.0001) == normal_stroke_color_after[1]);
	CHECK(doctest::Approx((*reread_normal_stroke_color)[2]).epsilon(0.0001) == normal_stroke_color_after[2]);
	CHECK(doctest::Approx((*reread_normal_stroke_color)[3]).epsilon(0.0001) == normal_stroke_color_after[3]);
	// Mutating normal style-sheet defaults should not rewrite explicit run values.
	CHECK(doctest::Approx(reread_run_font_size.value()).epsilon(0.0001) == run_font_size_before.value());

	std::filesystem::remove(out_path);
}



