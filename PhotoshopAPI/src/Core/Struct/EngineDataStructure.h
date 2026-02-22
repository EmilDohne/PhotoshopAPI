#pragma once

#include "Macros.h"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

PSAPI_NAMESPACE_BEGIN

namespace EngineData
{
	enum class ValueType
	{
		Dictionary,
		Array,
		Name,
		Number,
		Boolean,
		LiteralString,
		HexString,
		Identifier
	};

	struct Value
	{
		struct DictionaryItem;

		ValueType type = ValueType::Identifier;
		size_t start_offset = 0u;
		size_t end_offset = 0u;

		std::vector<DictionaryItem> dictionary_items;
		std::vector<Value> array_items;

		std::string string_value;
		double number_value = 0.0;
		bool bool_value = false;
		bool is_integer = false;
		int64_t integer_value = 0;
	};

	struct Value::DictionaryItem
	{
		std::string first;
		Value second;

		DictionaryItem() = default;
		DictionaryItem(std::string key, Value value) : first(std::move(key)), second(std::move(value)) {}
	};

	struct ParseResult
	{
		bool ok = false;
		Value root{};
		size_t error_offset = 0u;
		std::string error_message{};
	};

	ParseResult parse(const std::vector<std::byte>& data);
	std::vector<std::byte> serialize(const Value& value);

	const Value* find_dict_value(const Value& dictionary, std::string_view key);
	Value* find_dict_value(Value& dictionary, std::string_view key);

	const Value* find_by_path(const Value& dictionary, std::initializer_list<std::string_view> path);
	Value* find_by_path(Value& dictionary, std::initializer_list<std::string_view> path);

	bool as_int32_vector(const Value& array_value, std::vector<int32_t>& out);
	bool as_double_vector(const Value& array_value, std::vector<double>& out);

	bool set_number(Value& value, double number);
	bool set_int32_array(Value& value, const std::vector<int32_t>& values);
	bool set_double_array(Value& value, const std::vector<double>& values);

	// ---------- set helpers for non-number types ----------
	bool set_bool(Value& value, bool b);
	bool set_name(Value& value, const std::string& name);
	bool set_string(Value& value, const std::string& str);

	// ---------- factory helpers ----------
	Value make_number(double number);
	Value make_integer(int64_t integer);
	Value make_bool(bool b);
	Value make_name(const std::string& name);
	Value make_string(const std::string& str);
	Value make_dict();
	Value make_array();

	// ---------- structural mutation ----------
	/// Insert or replace a key-value pair in a dictionary.
	/// Returns false if `dictionary` is not a Dictionary.
	bool insert_dict_value(Value& dictionary, const std::string& key, Value value);

	/// Remove a key from a dictionary.  Returns true if the key was found and removed.
	bool remove_dict_value(Value& dictionary, const std::string& key);

	/// Append an item to an array.  Returns false if `array` is not an Array.
	bool append_array_item(Value& array, Value item);

	// ---------- in-place payload patching ----------

	/// Serialize a single value to its EngineData text representation (no trailing newline).
	/// The depth parameter controls indentation for dict/array values.
	std::vector<std::byte> format_value_bytes(const Value& value, size_t depth = 0u);

	/// Replace bytes at [old_start, old_end) in payload with new_bytes.
	void splice_payload(std::vector<std::byte>& payload, size_t old_start, size_t old_end, const std::vector<std::byte>& new_bytes);

	/// A pending byte-range replacement inside an EngineData payload.
	struct PayloadPatch
	{
		size_t old_start = 0u;
		size_t old_end = 0u;
		std::vector<std::byte> new_bytes{};
	};

	/// Sort patches by descending offset and apply them all to payload.
	void apply_patches(std::vector<std::byte>& payload, std::vector<PayloadPatch>& patches);

	/// Insert a new key-value entry into a dictionary's raw payload bytes.
	/// Determines indentation from the closing ">>" line of parent_dict.
	bool insert_dict_entry_bytes(std::vector<std::byte>& payload, const Value& parent_dict, const std::string& key, const Value& value);

	/// Insert a new item into an array's raw payload bytes.
	/// Determines indentation from the closing "]" line of parent_array.
	bool insert_array_item_bytes(std::vector<std::byte>& payload, const Value& parent_array, const Value& item);
}

PSAPI_NAMESPACE_END
