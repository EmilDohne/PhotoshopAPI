#pragma once

#include "Macros.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

PSAPI_NAMESPACE_BEGIN

namespace TextLayerDetail
{

struct ParsedTextSpan
{
	size_t count_offset = 0u;
	size_t value_offset = 0u;
	size_t code_unit_count = 0u;
	size_t text_utf16_length = 0u;
	size_t trailing_null_units = 0u;
	std::u16string text_utf16{};
	std::string text_utf8{};
};

struct TextReplacement
{
	size_t old_start = 0u;
	size_t old_length = 0u;
	size_t new_length = 0u;
};

struct Utf16ReplaceResult
{
	std::u16string text_utf16{};
	std::vector<TextReplacement> replacements{};
};

struct RawDataSpan
{
	size_t length_offset = 0u;
	size_t value_offset = 0u;
	size_t byte_count = 0u;
};

struct ByteRange
{
	size_t start = 0u;
	size_t end = 0u;
};

struct RunLengthArraySpan
{
	size_t open_bracket = 0u;
	size_t close_bracket = 0u;
	std::vector<int32_t> values{};
};

struct BytePatch
{
	size_t start = 0u;
	size_t end = 0u;
	std::vector<std::byte> replacement{};
};

struct StyleRunSpan
{
	size_t dict_start = 0u;
	size_t dict_end = 0u;
};

struct StyleRunInfo
{
	size_t run_array_open = 0u;
	size_t run_array_end = 0u;
	std::vector<StyleRunSpan> runs{};
};

struct DescKeyValue
{
	std::string key;
	std::string ostype;  // 4 chars: "long", "doub", "enum", "TEXT", "bool", "Objc", ...
	size_t data_offset;  // byte offset right after the 4-byte OSType tag
};

} // namespace TextLayerDetail

PSAPI_NAMESPACE_END
