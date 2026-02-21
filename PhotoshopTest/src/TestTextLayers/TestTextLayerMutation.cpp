#include "doctest.h"

#include "Core/TaggedBlocks/TaggedBlock.h"
#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace
{
	std::filesystem::path temp_psd_path()
	{
		static std::atomic<uint64_t> counter{ 0u };
		const auto stamp = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
		const auto idx = counter.fetch_add(1u, std::memory_order_relaxed);
		return std::filesystem::temp_directory_path() / ("psapi_text_mutation_" + std::to_string(stamp) + "_" + std::to_string(idx) + ".psd");
	}

	template <typename T>
	std::shared_ptr<NAMESPACE_PSAPI::TextLayer<T>> find_text_layer_with_contents(
		NAMESPACE_PSAPI::LayeredFile<T>& file,
		const std::string& text)
	{
		auto flat_layers = file.flat_layers();
		for (const auto& layer : flat_layers)
		{
			auto text_layer = std::dynamic_pointer_cast<NAMESPACE_PSAPI::TextLayer<T>>(layer);
			if (!text_layer)
			{
				continue;
			}

			const auto value = text_layer->text();
			if (value.has_value() && value.value() == text)
			{
				return text_layer;
			}
		}

		return nullptr;
	}

	template <typename T>
	std::shared_ptr<NAMESPACE_PSAPI::TextLayer<T>> find_text_layer_containing(
		NAMESPACE_PSAPI::LayeredFile<T>& file,
		const std::string_view needle)
	{
		auto flat_layers = file.flat_layers();
		for (const auto& layer : flat_layers)
		{
			auto text_layer = std::dynamic_pointer_cast<NAMESPACE_PSAPI::TextLayer<T>>(layer);
			if (!text_layer)
			{
				continue;
			}

			const auto value = text_layer->text();
			if (!value.has_value())
			{
				continue;
			}

			if (value->find(needle) != std::string::npos)
			{
				return text_layer;
			}
		}

		return nullptr;
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

	uint8_t to_u8(const std::byte value)
	{
		return std::to_integer<uint8_t>(value);
	}

	uint32_t read_u32_be(const std::vector<std::byte>& data, const size_t offset)
	{
		return (static_cast<uint32_t>(to_u8(data[offset])) << 24) |
			(static_cast<uint32_t>(to_u8(data[offset + 1u])) << 16) |
			(static_cast<uint32_t>(to_u8(data[offset + 2u])) << 8) |
			static_cast<uint32_t>(to_u8(data[offset + 3u]));
	}

	bool ascii_equal_at(const std::vector<std::byte>& data, const size_t offset, const std::string_view needle)
	{
		if (offset + needle.size() > data.size())
		{
			return false;
		}

		for (size_t i = 0u; i < needle.size(); ++i)
		{
			if (to_u8(data[offset + i]) != static_cast<uint8_t>(needle[i]))
			{
				return false;
			}
		}
		return true;
	}

	std::optional<size_t> find_ascii(const std::vector<std::byte>& data, const std::string_view needle, const size_t start = 0u)
	{
		if (needle.empty() || start >= data.size() || needle.size() > data.size() - start)
		{
			return std::nullopt;
		}

		for (size_t i = start; i + needle.size() <= data.size(); ++i)
		{
			if (ascii_equal_at(data, i, needle))
			{
				return i;
			}
		}
		return std::nullopt;
	}

	std::vector<std::vector<int32_t>> extract_engine_run_length_arrays(const NAMESPACE_PSAPI::TaggedBlock& block)
	{
		std::vector<std::vector<int32_t>> arrays;

		const auto marker_pos = find_ascii(block.m_Data, "EngineDatatdta");
		if (!marker_pos.has_value())
		{
			return arrays;
		}

		const size_t length_offset = marker_pos.value() + std::string_view("EngineDatatdta").size();
		if (length_offset + sizeof(uint32_t) > block.m_Data.size())
		{
			return arrays;
		}

		const size_t payload_len = static_cast<size_t>(read_u32_be(block.m_Data, length_offset));
		const size_t payload_offset = length_offset + sizeof(uint32_t);
		if (payload_offset + payload_len > block.m_Data.size())
		{
			return arrays;
		}

		std::vector<std::byte> payload(
			block.m_Data.begin() + static_cast<std::ptrdiff_t>(payload_offset),
			block.m_Data.begin() + static_cast<std::ptrdiff_t>(payload_offset + payload_len)
		);

		size_t search_pos = 0u;
		while (true)
		{
			const auto marker = find_ascii(payload, "/RunLengthArray", search_pos);
			if (!marker.has_value())
			{
				break;
			}

			size_t cursor = marker.value() + std::string_view("/RunLengthArray").size();
			while (cursor < payload.size() && std::isspace(static_cast<unsigned char>(to_u8(payload[cursor]))))
			{
				++cursor;
			}
			if (cursor >= payload.size() || to_u8(payload[cursor]) != static_cast<uint8_t>('['))
			{
				search_pos = marker.value() + 1u;
				continue;
			}
			++cursor;

			std::vector<int32_t> values;
			while (cursor < payload.size())
			{
				while (cursor < payload.size() && std::isspace(static_cast<unsigned char>(to_u8(payload[cursor]))))
				{
					++cursor;
				}
				if (cursor >= payload.size())
				{
					break;
				}

				const auto c = static_cast<char>(to_u8(payload[cursor]));
				if (c == ']')
				{
					++cursor;
					break;
				}

				bool negative = false;
				if (c == '-')
				{
					negative = true;
					++cursor;
				}

				if (cursor >= payload.size() || !std::isdigit(static_cast<unsigned char>(to_u8(payload[cursor]))))
				{
					++cursor;
					continue;
				}

				int64_t parsed = 0;
				while (cursor < payload.size() && std::isdigit(static_cast<unsigned char>(to_u8(payload[cursor]))))
				{
					parsed = parsed * 10 + static_cast<int64_t>(to_u8(payload[cursor]) - static_cast<uint8_t>('0'));
					++cursor;
				}
				if (negative)
				{
					parsed = -parsed;
				}

				if (parsed >= static_cast<int64_t>(std::numeric_limits<int32_t>::min()) &&
					parsed <= static_cast<int64_t>(std::numeric_limits<int32_t>::max()))
				{
					values.push_back(static_cast<int32_t>(parsed));
				}
			}

			if (!values.empty())
			{
				arrays.push_back(std::move(values));
			}
			search_pos = cursor;
		}

		return arrays;
	}

	std::optional<int32_t> extract_engine_int_value(const NAMESPACE_PSAPI::TaggedBlock& block, const std::string_view key)
	{
		const auto marker_pos = find_ascii(block.m_Data, "EngineDatatdta");
		if (!marker_pos.has_value())
		{
			return std::nullopt;
		}

		const size_t length_offset = marker_pos.value() + std::string_view("EngineDatatdta").size();
		if (length_offset + sizeof(uint32_t) > block.m_Data.size())
		{
			return std::nullopt;
		}

		const size_t payload_len = static_cast<size_t>(read_u32_be(block.m_Data, length_offset));
		const size_t payload_offset = length_offset + sizeof(uint32_t);
		if (payload_offset + payload_len > block.m_Data.size())
		{
			return std::nullopt;
		}

		std::vector<std::byte> payload(
			block.m_Data.begin() + static_cast<std::ptrdiff_t>(payload_offset),
			block.m_Data.begin() + static_cast<std::ptrdiff_t>(payload_offset + payload_len)
		);

		const auto key_pos = find_ascii(payload, key);
		if (!key_pos.has_value())
		{
			return std::nullopt;
		}

		size_t cursor = key_pos.value() + key.size();
		while (cursor < payload.size() && std::isspace(static_cast<unsigned char>(to_u8(payload[cursor]))))
		{
			++cursor;
		}
		if (cursor >= payload.size())
		{
			return std::nullopt;
		}

		bool negative = false;
		if (to_u8(payload[cursor]) == static_cast<uint8_t>('-'))
		{
			negative = true;
			++cursor;
		}
		if (cursor >= payload.size() || !std::isdigit(static_cast<unsigned char>(to_u8(payload[cursor]))))
		{
			return std::nullopt;
		}

		int64_t parsed = 0;
		while (cursor < payload.size() && std::isdigit(static_cast<unsigned char>(to_u8(payload[cursor]))))
		{
			parsed = parsed * 10 + static_cast<int64_t>(to_u8(payload[cursor]) - static_cast<uint8_t>('0'));
			++cursor;
		}
		if (negative)
		{
			parsed = -parsed;
		}

		if (parsed < static_cast<int64_t>(std::numeric_limits<int32_t>::min()) ||
			parsed > static_cast<int64_t>(std::numeric_limits<int32_t>::max()))
		{
			return std::nullopt;
		}
		return static_cast<int32_t>(parsed);
	}

	std::string arrays_to_string(const std::vector<std::vector<int32_t>>& arrays)
	{
		std::string out;
		for (size_t i = 0u; i < arrays.size(); ++i)
		{
			if (i > 0u)
			{
				out += " | ";
			}
			out += "[";
			for (size_t j = 0u; j < arrays[i].size(); ++j)
			{
				if (j > 0u)
				{
					out += ",";
				}
				out += std::to_string(arrays[i][j]);
			}
			out += "]";
		}
		return out;
	}
}


TEST_CASE("TextLayer can read text payload from fixture descriptor")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer);
	CHECK(text_layer->text().has_value());
	CHECK(text_layer->text().value() == "Hello 123");
}


TEST_CASE("TextLayer equal-length replacement survives write/read roundtrip")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer);

	CHECK(text_layer->replace_text_equal_length("Hello", "Hallo"));
	CHECK(text_layer->text().has_value());
	CHECK(text_layer->text().value() == "Hallo 123");

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_with_contents(reread, "Hallo 123");
	CHECK(reread_layer != nullptr);

	std::filesystem::remove(out_path);
}


TEST_CASE("TextLayer rejects non-equal-length replacements")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer);

	CHECK_FALSE(text_layer->replace_text_equal_length("Hello", "Greetings"));
	CHECK_FALSE(text_layer->set_text_equal_length("Longer than nine"));

	CHECK(text_layer->text().has_value());
	CHECK(text_layer->text().value() == "Hello 123");
}


TEST_CASE("TextLayer variable-length replacement survives write/read roundtrip")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer);

	CHECK(text_layer->replace_text("Hello", "Greetings"));
	CHECK(text_layer->text().has_value());
	CHECK(text_layer->text().value() == "Greetings 123");

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_with_contents(reread, "Greetings 123");
	CHECK(reread_layer != nullptr);

	std::filesystem::remove(out_path);
}


TEST_CASE("TextLayer remaps EngineData run lengths for style runs")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer);

	CHECK(text_layer->replace_text("Beta", "Betaaaa"));
	CHECK(text_layer->text().has_value());
	CHECK(text_layer->text().value() == "Alpha Betaaaa Gamma");

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_with_contents(reread, "Alpha Betaaaa Gamma");
	REQUIRE(reread_layer != nullptr);

	auto [reread_record, _reread_channel_data] = reread_layer->to_photoshop();
	REQUIRE(reread_record.m_AdditionalLayerInfo.has_value());
	const auto reread_tysh = reread_record.m_AdditionalLayerInfo->getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrTypeTool);
	REQUIRE(reread_tysh.has_value());

	const auto reread_arrays = extract_engine_run_length_arrays(*reread_tysh.value());
	CHECK(std::find(reread_arrays.begin(), reread_arrays.end(), std::vector<int32_t>{ 5, 1, 7, 1, 6 }) != reread_arrays.end());
	CHECK(std::find(reread_arrays.begin(), reread_arrays.end(), std::vector<int32_t>{ 20 }) != reread_arrays.end());

	std::filesystem::remove(out_path);
}


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
	if (run_character_direction_after.has_value())
	{
		CHECK(text_layer->set_style_run_character_direction(0u, run_character_direction_after.value()));
	}
	else
	{
		CHECK_FALSE(text_layer->set_style_run_character_direction(0u, TextLayerEnum::CharacterDirection::LeftToRight));
	}
	CHECK(text_layer->set_style_run_baseline_direction(0u, run_baseline_direction_after));
	CHECK(text_layer->set_style_run_tsume(0u, run_tsume_after));
	CHECK(text_layer->set_style_run_kashida(0u, run_kashida_after));
	if (run_diacritic_pos_after.has_value())
	{
		CHECK(text_layer->set_style_run_diacritic_pos(0u, run_diacritic_pos_after.value()));
	}
	else
	{
		CHECK_FALSE(text_layer->set_style_run_diacritic_pos(0u, TextLayerEnum::DiacriticPosition::Loose));
	}
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
	if (run_outline_width_after.has_value())
	{
		CHECK(text_layer->set_style_run_outline_width(0u, run_outline_width_after.value()));
	}
	else
	{
		CHECK_FALSE(text_layer->set_style_run_outline_width(0u, 2.0));
	}
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
	if (run_character_direction_after.has_value())
	{
		REQUIRE(reread_run_character_direction.has_value());
		CHECK(reread_run_character_direction.value() == run_character_direction_after.value());
	}
	else
	{
		CHECK_FALSE(reread_run_character_direction.has_value());
	}
	CHECK(reread_run_baseline_direction.value() == run_baseline_direction_after);
	CHECK(doctest::Approx(reread_run_tsume.value()).epsilon(0.0001) == run_tsume_after);
	CHECK(reread_run_kashida.value() == run_kashida_after);
	if (run_diacritic_pos_after.has_value())
	{
		REQUIRE(reread_run_diacritic_pos.has_value());
		CHECK(reread_run_diacritic_pos.value() == run_diacritic_pos_after.value());
	}
	else
	{
		CHECK_FALSE(reread_run_diacritic_pos.has_value());
	}
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
	if (run_outline_width_after.has_value())
	{
		REQUIRE(reread_run_outline_width.has_value());
		CHECK(doctest::Approx(reread_run_outline_width.value()).epsilon(0.0001) == run_outline_width_after.value());
	}
	else
	{
		CHECK_FALSE(reread_run_outline_width.has_value());
	}
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

	CHECK(text_layer->set_paragraph_run_justification(0u, justification_after));
	CHECK(text_layer->set_paragraph_run_first_line_indent(0u, first_line_after));
	CHECK(text_layer->set_paragraph_run_start_indent(0u, start_after));
	CHECK(text_layer->set_paragraph_run_end_indent(0u, end_after));
	CHECK(text_layer->set_paragraph_run_space_before(0u, space_before_after));
	CHECK(text_layer->set_paragraph_run_space_after(0u, space_after_after));
	CHECK(text_layer->set_paragraph_run_auto_hyphenate(0u, auto_hyphenate_after));
	CHECK(text_layer->set_paragraph_run_hyphenated_word_size(0u, hyphenated_word_size_after));
	CHECK(text_layer->set_paragraph_run_pre_hyphen(0u, pre_hyphen_after));
	CHECK(text_layer->set_paragraph_run_post_hyphen(0u, post_hyphen_after));
	CHECK(text_layer->set_paragraph_run_consecutive_hyphens(0u, consecutive_hyphens_after));
	CHECK(text_layer->set_paragraph_run_zone(0u, zone_after));
	CHECK(text_layer->set_paragraph_run_word_spacing(0u, word_spacing_after));
	CHECK(text_layer->set_paragraph_run_letter_spacing(0u, letter_spacing_after));
	CHECK(text_layer->set_paragraph_run_glyph_spacing(0u, glyph_spacing_after));
	CHECK(text_layer->set_paragraph_run_auto_leading(0u, auto_leading_after));
	CHECK(text_layer->set_paragraph_run_leading_type(0u, leading_type_after));
	CHECK(text_layer->set_paragraph_run_hanging(0u, hanging_after));
	CHECK(text_layer->set_paragraph_run_burasagari(0u, burasagari_after));
	CHECK(text_layer->set_paragraph_run_kinsoku_order(0u, kinsoku_order_after));
	CHECK(text_layer->set_paragraph_run_every_line_composer(0u, every_line_composer_after));
	CHECK_FALSE(text_layer->set_paragraph_run_justification(200u, justification_after));
	CHECK_FALSE(text_layer->set_paragraph_run_first_line_indent(200u, first_line_after));
	CHECK_FALSE(text_layer->set_paragraph_run_start_indent(200u, start_after));
	CHECK_FALSE(text_layer->set_paragraph_run_end_indent(200u, end_after));
	CHECK_FALSE(text_layer->set_paragraph_run_space_before(200u, space_before_after));
	CHECK_FALSE(text_layer->set_paragraph_run_space_after(200u, space_after_after));
	CHECK_FALSE(text_layer->set_paragraph_run_auto_hyphenate(200u, auto_hyphenate_after));
	CHECK_FALSE(text_layer->set_paragraph_run_hyphenated_word_size(200u, hyphenated_word_size_after));
	CHECK_FALSE(text_layer->set_paragraph_run_pre_hyphen(200u, pre_hyphen_after));
	CHECK_FALSE(text_layer->set_paragraph_run_post_hyphen(200u, post_hyphen_after));
	CHECK_FALSE(text_layer->set_paragraph_run_consecutive_hyphens(200u, consecutive_hyphens_after));
	CHECK_FALSE(text_layer->set_paragraph_run_zone(200u, zone_after));
	CHECK_FALSE(text_layer->set_paragraph_run_word_spacing(200u, word_spacing_after));
	CHECK_FALSE(text_layer->set_paragraph_run_letter_spacing(200u, letter_spacing_after));
	CHECK_FALSE(text_layer->set_paragraph_run_glyph_spacing(200u, glyph_spacing_after));
	CHECK_FALSE(text_layer->set_paragraph_run_auto_leading(200u, auto_leading_after));
	CHECK_FALSE(text_layer->set_paragraph_run_leading_type(200u, leading_type_after));
	CHECK_FALSE(text_layer->set_paragraph_run_hanging(200u, hanging_after));
	CHECK_FALSE(text_layer->set_paragraph_run_burasagari(200u, burasagari_after));
	CHECK_FALSE(text_layer->set_paragraph_run_kinsoku_order(200u, kinsoku_order_after));
	CHECK_FALSE(text_layer->set_paragraph_run_every_line_composer(200u, every_line_composer_after));
	CHECK_FALSE(text_layer->set_paragraph_run_space_before(0u, std::numeric_limits<double>::infinity()));
	CHECK_FALSE(text_layer->set_paragraph_run_word_spacing(0u, {}));
	CHECK_FALSE(text_layer->set_paragraph_run_word_spacing(0u, std::vector<double>{ 1.0, std::numeric_limits<double>::infinity(), 2.0 }));

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

	CHECK(text_layer->set_paragraph_normal_sheet_index(normal_sheet_index_before.value()));
	CHECK(text_layer->set_paragraph_normal_justification(normal_justification_after));
	CHECK(text_layer->set_paragraph_normal_first_line_indent(normal_first_line_after));
	CHECK(text_layer->set_paragraph_normal_start_indent(normal_start_after));
	CHECK(text_layer->set_paragraph_normal_end_indent(normal_end_after));
	CHECK(text_layer->set_paragraph_normal_space_before(normal_space_before_after));
	CHECK(text_layer->set_paragraph_normal_space_after(normal_space_after_after));
	CHECK(text_layer->set_paragraph_normal_auto_hyphenate(normal_auto_hyphenate_after));
	CHECK(text_layer->set_paragraph_normal_hyphenated_word_size(normal_hyphenated_word_size_after));
	CHECK(text_layer->set_paragraph_normal_pre_hyphen(normal_pre_hyphen_after));
	CHECK(text_layer->set_paragraph_normal_post_hyphen(normal_post_hyphen_after));
	CHECK(text_layer->set_paragraph_normal_consecutive_hyphens(normal_consecutive_hyphens_after));
	CHECK(text_layer->set_paragraph_normal_zone(normal_zone_after));
	CHECK(text_layer->set_paragraph_normal_word_spacing(normal_word_spacing_after));
	CHECK(text_layer->set_paragraph_normal_letter_spacing(normal_letter_spacing_after));
	CHECK(text_layer->set_paragraph_normal_glyph_spacing(normal_glyph_spacing_after));
	CHECK(text_layer->set_paragraph_normal_auto_leading(normal_auto_leading_after));
	CHECK(text_layer->set_paragraph_normal_leading_type(normal_leading_type_after));
	CHECK(text_layer->set_paragraph_normal_hanging(normal_hanging_after));
	CHECK(text_layer->set_paragraph_normal_burasagari(normal_burasagari_after));
	CHECK(text_layer->set_paragraph_normal_kinsoku_order(normal_kinsoku_order_after));
	CHECK(text_layer->set_paragraph_normal_every_line_composer(normal_every_line_composer_after));
	CHECK_FALSE(text_layer->set_paragraph_normal_sheet_index(200));
	CHECK_FALSE(text_layer->set_paragraph_normal_sheet_index(-1));
	CHECK_FALSE(text_layer->set_paragraph_normal_space_before(std::numeric_limits<double>::infinity()));
	CHECK_FALSE(text_layer->set_paragraph_normal_word_spacing({}));
	CHECK_FALSE(text_layer->set_paragraph_normal_word_spacing(std::vector<double>{ 1.0, std::numeric_limits<double>::infinity(), 2.0 }));
	CHECK_FALSE(text_layer->set_paragraph_normal_letter_spacing({}));
	CHECK_FALSE(text_layer->set_paragraph_normal_glyph_spacing({}));

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
	CHECK(text_layer->set_orientation(TextLayerEnum::WritingDirection::Vertical));

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

	CHECK(text_layer->set_orientation(TextLayerEnum::WritingDirection::Vertical));

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
	CHECK(text_layer->set_orientation(TextLayerEnum::WritingDirection::Horizontal));

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

	CHECK(text_layer->set_orientation(TextLayerEnum::WritingDirection::Horizontal));

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

	// Resolve each index to a name — all should be non-empty
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
	CHECK(text_layer->set_font_postscript_name(0, "ReplacedFont-Regular"));

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

	CHECK_FALSE(text_layer->set_font_postscript_name(9999, "NoSuchFont"));
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
	CHECK(text_layer->set_style_run_font(0, new_idx));

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

	// Get the name of font at index 0 — this is already in the FontSet
	const auto existing_name = text_layer->font_postscript_name(0);
	REQUIRE(existing_name.has_value());
	const auto count_before = text_layer->font_count();

	// set_style_run_font_by_name should find the existing font, not add a new one
	CHECK(text_layer->set_style_run_font_by_name(0, existing_name.value()));

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

	// Use a font name not in the set — should add it
	CHECK(text_layer->set_style_run_font_by_name(0, "BrandNewFont-Regular"));

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
	CHECK(text_layer->set_style_normal_font_by_name("NormalNewFont-Light"));

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
	CHECK(text_layer->set_style_normal_font_by_name(existing_name.value()));
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
	CHECK(text_layer->set_style_run_font(0, new_idx));
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


TEST_CASE("TextLayer legacy From/To remap: variable-length mutation on multi-style file roundtrips")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer);

	// Grow mid-run text so indices shift
	CHECK(text_layer->replace_text("Beta", "BetaBetaBeta"));
	CHECK(text_layer->text().value() == "Alpha BetaBetaBeta Gamma");

	// Write and re-read
	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_with_contents(reread, "Alpha BetaBetaBeta Gamma");
	REQUIRE(reread_layer != nullptr);

	// Verify EngineData run lengths are coherent.
	auto [rec, _] = reread_layer->to_photoshop();
	REQUIRE(rec.m_AdditionalLayerInfo.has_value());
	const auto tysh = rec.m_AdditionalLayerInfo->getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrTypeTool);
	REQUIRE(tysh.has_value());

	const auto arrays = extract_engine_run_length_arrays(*tysh.value());
	// Original: "Alpha Beta Gamma" → runs [5, 1, 4, 1, 6] → mutated "Alpha BetaBetaBeta Gamma" → [5, 1, 12, 1, 6]
	// Plus trailing sentinel. Total = 5+1+12+1+6 = 25 (24 visible + 1 trailing?)
	CHECK(std::find(arrays.begin(), arrays.end(), std::vector<int32_t>{ 5, 1, 12, 1, 6 }) != arrays.end());

	// Verify style run count is preserved
	CHECK(reread_layer->style_run_count() == 5u);

	std::filesystem::remove(out_path);
}


TEST_CASE("TextLayer legacy From/To remap: shrinking mutation on multi-style file roundtrips")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer);

	// Shrink: "Alpha" → "A"
	CHECK(text_layer->replace_text("Alpha", "A"));
	CHECK(text_layer->text().value() == "A Beta Gamma");

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_with_contents(reread, "A Beta Gamma");
	REQUIRE(reread_layer != nullptr);

	// Verify EngineData run lengths: original [5,1,4,1,6] → after "Alpha"→"A": [1,1,4,1,6]
	auto [rec, _] = reread_layer->to_photoshop();
	REQUIRE(rec.m_AdditionalLayerInfo.has_value());
	const auto tysh = rec.m_AdditionalLayerInfo->getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrTypeTool);
	REQUIRE(tysh.has_value());

	const auto arrays = extract_engine_run_length_arrays(*tysh.value());
	CHECK(std::find(arrays.begin(), arrays.end(), std::vector<int32_t>{ 1, 1, 4, 1, 6 }) != arrays.end());

	CHECK(reread_layer->style_run_count() == 5u);

	std::filesystem::remove(out_path);
}


TEST_CASE("TextLayer legacy From/To remap: no-op on same-length mutation preserves TySh descriptor")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer);

	// Same-length substitution: "Beta" → "XXXX"
	CHECK(text_layer->replace_text("Beta", "XXXX"));
	CHECK(text_layer->text().value() == "Alpha XXXX Gamma");

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_with_contents(reread, "Alpha XXXX Gamma");
	REQUIRE(reread_layer != nullptr);

	// Run lengths unchanged: [5, 1, 4, 1, 6]
	auto [rec, _] = reread_layer->to_photoshop();
	REQUIRE(rec.m_AdditionalLayerInfo.has_value());
	const auto tysh = rec.m_AdditionalLayerInfo->getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrTypeTool);
	REQUIRE(tysh.has_value());

	const auto arrays = extract_engine_run_length_arrays(*tysh.value());
	CHECK(std::find(arrays.begin(), arrays.end(), std::vector<int32_t>{ 5, 1, 4, 1, 6 }) != arrays.end());

	CHECK(reread_layer->style_run_count() == 5u);

	std::filesystem::remove(out_path);
}
