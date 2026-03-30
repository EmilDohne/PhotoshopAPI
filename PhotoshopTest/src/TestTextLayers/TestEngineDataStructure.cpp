#include "doctest.h"

#include "Core/Struct/EngineDataStructure.h"
#include "Core/TaggedBlocks/TaggedBlock.h"
#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"
#include "LayeredFile/LayeredFile.h"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace
{
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

	std::optional<std::vector<std::byte>> extract_engine_payload(const NAMESPACE_PSAPI::TaggedBlock& block)
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

		return std::vector<std::byte>(
			block.m_Data.begin() + static_cast<std::ptrdiff_t>(payload_offset),
			block.m_Data.begin() + static_cast<std::ptrdiff_t>(payload_offset + payload_len)
		);
	}
}


TEST_CASE("EngineData parser can parse and reserialize style run payload")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer != nullptr);

	auto [record, _channel_data] = text_layer->to_photoshop();
	REQUIRE(record.m_AdditionalLayerInfo.has_value());
	const auto tysh = record.m_AdditionalLayerInfo->getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrTypeTool);
	REQUIRE(tysh.has_value());

	const auto payload = extract_engine_payload(*tysh.value());
	REQUIRE(payload.has_value());

	const auto parsed = EngineData::parse(payload.value());
	REQUIRE(parsed.ok);

	const auto run_array = EngineData::find_by_path(parsed.root, { "EngineDict", "StyleRun", "RunArray" });
	REQUIRE(run_array != nullptr);
	CHECK(run_array->type == EngineData::ValueType::Array);
	CHECK(run_array->array_items.size() == 5u);

	const auto run_lengths = EngineData::find_by_path(parsed.root, { "EngineDict", "StyleRun", "RunLengthArray" });
	REQUIRE(run_lengths != nullptr);
	std::vector<int32_t> lengths{};
	REQUIRE(EngineData::as_int32_vector(*run_lengths, lengths));
	CHECK(lengths == std::vector<int32_t>{ 5, 1, 4, 1, 6 });

	const auto serialized = EngineData::serialize(parsed.root);
	const auto reparsed = EngineData::parse(serialized);
	REQUIRE(reparsed.ok);

	const auto run_lengths_reparsed = EngineData::find_by_path(reparsed.root, { "EngineDict", "StyleRun", "RunLengthArray" });
	REQUIRE(run_lengths_reparsed != nullptr);
	std::vector<int32_t> lengths_reparsed{};
	REQUIRE(EngineData::as_int32_vector(*run_lengths_reparsed, lengths_reparsed));
	CHECK(lengths_reparsed == lengths);
}


// ============================================================
// Factory helpers
// ============================================================

TEST_CASE("EngineData make_number creates a floating-point value")
{
	using namespace NAMESPACE_PSAPI;
	auto v = EngineData::make_number(3.14);
	CHECK(v.type == EngineData::ValueType::Number);
	CHECK(v.number_value == doctest::Approx(3.14));
	CHECK_FALSE(v.is_integer);
}

TEST_CASE("EngineData make_number with integer-valued double marks is_integer")
{
	using namespace NAMESPACE_PSAPI;
	auto v = EngineData::make_number(42.0);
	CHECK(v.type == EngineData::ValueType::Number);
	CHECK(v.number_value == doctest::Approx(42.0));
	CHECK(v.is_integer);
	CHECK(v.integer_value == 42);
}

TEST_CASE("EngineData make_integer creates integer value")
{
	using namespace NAMESPACE_PSAPI;
	auto v = EngineData::make_integer(7);
	CHECK(v.type == EngineData::ValueType::Number);
	CHECK(v.number_value == doctest::Approx(7.0));
	CHECK(v.is_integer);
	CHECK(v.integer_value == 7);
}

TEST_CASE("EngineData make_bool creates a boolean value")
{
	using namespace NAMESPACE_PSAPI;
	auto t = EngineData::make_bool(true);
	auto f = EngineData::make_bool(false);
	CHECK(t.type == EngineData::ValueType::Boolean);
	CHECK(t.bool_value == true);
	CHECK(f.type == EngineData::ValueType::Boolean);
	CHECK(f.bool_value == false);
}

TEST_CASE("EngineData make_name creates a Name value")
{
	using namespace NAMESPACE_PSAPI;
	auto v = EngineData::make_name("WritingDirection");
	CHECK(v.type == EngineData::ValueType::Name);
	CHECK(v.string_value == "WritingDirection");
}

TEST_CASE("EngineData make_string creates a LiteralString value")
{
	using namespace NAMESPACE_PSAPI;
	auto v = EngineData::make_string("Hello World");
	CHECK(v.type == EngineData::ValueType::LiteralString);
	CHECK(v.string_value == "Hello World");
}

TEST_CASE("EngineData make_dict creates an empty dictionary")
{
	using namespace NAMESPACE_PSAPI;
	auto v = EngineData::make_dict();
	CHECK(v.type == EngineData::ValueType::Dictionary);
	CHECK(v.dictionary_items.empty());
}

TEST_CASE("EngineData make_array creates an empty array")
{
	using namespace NAMESPACE_PSAPI;
	auto v = EngineData::make_array();
	CHECK(v.type == EngineData::ValueType::Array);
	CHECK(v.array_items.empty());
}


// ============================================================
// set_bool / set_name / set_string
// ============================================================

TEST_CASE("EngineData set_bool changes value and rejects wrong type")
{
	using namespace NAMESPACE_PSAPI;
	auto b = EngineData::make_bool(false);
	CHECK(EngineData::set_bool(b, true));
	CHECK(b.bool_value == true);

	auto n = EngineData::make_number(1.0);
	CHECK_FALSE(EngineData::set_bool(n, true));
}

TEST_CASE("EngineData set_name changes value and rejects wrong type")
{
	using namespace NAMESPACE_PSAPI;
	auto name = EngineData::make_name("Old");
	CHECK(EngineData::set_name(name, "New"));
	CHECK(name.string_value == "New");

	auto n = EngineData::make_number(1.0);
	CHECK_FALSE(EngineData::set_name(n, "Oops"));
}

TEST_CASE("EngineData set_string changes value and rejects wrong type")
{
	using namespace NAMESPACE_PSAPI;
	auto s = EngineData::make_string("Old");
	CHECK(EngineData::set_string(s, "New"));
	CHECK(s.string_value == "New");

	auto n = EngineData::make_number(1.0);
	CHECK_FALSE(EngineData::set_string(n, "Oops"));
}


// ============================================================
// insert_dict_value / remove_dict_value / append_array_item
// ============================================================

TEST_CASE("EngineData insert_dict_value inserts a new key")
{
	using namespace NAMESPACE_PSAPI;
	auto dict = EngineData::make_dict();
	CHECK(EngineData::insert_dict_value(dict, "FontSize", EngineData::make_number(24.0)));
	CHECK(dict.dictionary_items.size() == 1u);
	CHECK(dict.dictionary_items[0].first == "FontSize");
	CHECK(dict.dictionary_items[0].second.number_value == doctest::Approx(24.0));
}

TEST_CASE("EngineData insert_dict_value replaces an existing key")
{
	using namespace NAMESPACE_PSAPI;
	auto dict = EngineData::make_dict();
	EngineData::insert_dict_value(dict, "FontSize", EngineData::make_number(12.0));
	EngineData::insert_dict_value(dict, "FontSize", EngineData::make_number(36.0));
	CHECK(dict.dictionary_items.size() == 1u);
	CHECK(dict.dictionary_items[0].second.number_value == doctest::Approx(36.0));
}

TEST_CASE("EngineData insert_dict_value rejects non-dictionary")
{
	using namespace NAMESPACE_PSAPI;
	auto arr = EngineData::make_array();
	CHECK_FALSE(EngineData::insert_dict_value(arr, "Key", EngineData::make_number(1.0)));
}

TEST_CASE("EngineData remove_dict_value removes an existing key")
{
	using namespace NAMESPACE_PSAPI;
	auto dict = EngineData::make_dict();
	EngineData::insert_dict_value(dict, "A", EngineData::make_number(1.0));
	EngineData::insert_dict_value(dict, "B", EngineData::make_number(2.0));

	CHECK(EngineData::remove_dict_value(dict, "A"));
	CHECK(dict.dictionary_items.size() == 1u);
	CHECK(dict.dictionary_items[0].first == "B");
}

TEST_CASE("EngineData remove_dict_value returns false for missing key")
{
	using namespace NAMESPACE_PSAPI;
	auto dict = EngineData::make_dict();
	CHECK_FALSE(EngineData::remove_dict_value(dict, "Missing"));
}

TEST_CASE("EngineData remove_dict_value rejects non-dictionary")
{
	using namespace NAMESPACE_PSAPI;
	auto num = EngineData::make_number(42.0);
	CHECK_FALSE(EngineData::remove_dict_value(num, "Key"));
}

TEST_CASE("EngineData append_array_item appends items in order")
{
	using namespace NAMESPACE_PSAPI;
	auto arr = EngineData::make_array();
	CHECK(EngineData::append_array_item(arr, EngineData::make_number(1.0)));
	CHECK(EngineData::append_array_item(arr, EngineData::make_number(2.0)));
	CHECK(EngineData::append_array_item(arr, EngineData::make_number(3.0)));
	CHECK(arr.array_items.size() == 3u);
	CHECK(arr.array_items[0].number_value == doctest::Approx(1.0));
	CHECK(arr.array_items[2].number_value == doctest::Approx(3.0));
}

TEST_CASE("EngineData append_array_item rejects non-array")
{
	using namespace NAMESPACE_PSAPI;
	auto dict = EngineData::make_dict();
	CHECK_FALSE(EngineData::append_array_item(dict, EngineData::make_number(1.0)));
}


// ============================================================
// Round-trip: built tree serializes and re-parses correctly
// ============================================================

TEST_CASE("EngineData factory-built tree survives serialize-parse round-trip")
{
	using namespace NAMESPACE_PSAPI;

	// Build a small tree: << /Root << /Name /Hello  /Size 42  /Enabled true  /Items [ 1 2 3 ] >> >>
	auto inner = EngineData::make_dict();
	EngineData::insert_dict_value(inner, "Name", EngineData::make_name("Hello"));
	EngineData::insert_dict_value(inner, "Size", EngineData::make_integer(42));
	EngineData::insert_dict_value(inner, "Enabled", EngineData::make_bool(true));

	auto items = EngineData::make_array();
	EngineData::append_array_item(items, EngineData::make_integer(1));
	EngineData::append_array_item(items, EngineData::make_integer(2));
	EngineData::append_array_item(items, EngineData::make_integer(3));
	EngineData::insert_dict_value(inner, "Items", std::move(items));

	auto root = EngineData::make_dict();
	EngineData::insert_dict_value(root, "Root", std::move(inner));

	const auto serialized = EngineData::serialize(root);
	const auto reparsed = EngineData::parse(serialized);
	REQUIRE(reparsed.ok);

	// Validate structure
	const auto* root_dict = EngineData::find_dict_value(reparsed.root, "Root");
	REQUIRE(root_dict != nullptr);
	CHECK(root_dict->type == EngineData::ValueType::Dictionary);

	const auto* name_val = EngineData::find_dict_value(*root_dict, "Name");
	REQUIRE(name_val != nullptr);
	CHECK(name_val->type == EngineData::ValueType::Name);
	CHECK(name_val->string_value == "Hello");

	const auto* size_val = EngineData::find_dict_value(*root_dict, "Size");
	REQUIRE(size_val != nullptr);
	CHECK(size_val->is_integer);
	CHECK(size_val->integer_value == 42);

	const auto* enabled_val = EngineData::find_dict_value(*root_dict, "Enabled");
	REQUIRE(enabled_val != nullptr);
	CHECK(enabled_val->bool_value == true);

	const auto* items_val = EngineData::find_dict_value(*root_dict, "Items");
	REQUIRE(items_val != nullptr);
	CHECK(items_val->type == EngineData::ValueType::Array);
	CHECK(items_val->array_items.size() == 3u);

	std::vector<int32_t> item_ints{};
	REQUIRE(EngineData::as_int32_vector(*items_val, item_ints));
	CHECK(item_ints == std::vector<int32_t>{ 1, 2, 3 });
}