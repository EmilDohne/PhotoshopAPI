#include "EngineDataStructure.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <sstream>

PSAPI_NAMESPACE_BEGIN

namespace EngineData
{
	namespace
	{
		uint8_t to_u8(const std::byte value)
		{
			return std::to_integer<uint8_t>(value);
		}

		bool is_ascii_whitespace(const uint8_t c)
		{
			return std::isspace(static_cast<unsigned char>(c)) != 0;
		}

		bool is_value_delimiter(const uint8_t c)
		{
			return is_ascii_whitespace(c) ||
				c == static_cast<uint8_t>('[') ||
				c == static_cast<uint8_t>(']') ||
				c == static_cast<uint8_t>('(') ||
				c == static_cast<uint8_t>(')') ||
				c == static_cast<uint8_t>('<') ||
				c == static_cast<uint8_t>('>') ||
				c == static_cast<uint8_t>('/') ||
				c == static_cast<uint8_t>('%');
		}

		std::optional<double> parse_double_token(const std::string& token, bool& is_integer, int64_t& integer_value)
		{
			is_integer = false;
			integer_value = 0;
			if (token.empty())
			{
				return std::nullopt;
			}

			bool has_digit = false;
			for (const auto c : token)
			{
				if (std::isdigit(static_cast<unsigned char>(c)) != 0)
				{
					has_digit = true;
					break;
				}
			}
			if (!has_digit)
			{
				return std::nullopt;
			}

			errno = 0;
			char* end_ptr = nullptr;
			const double parsed = std::strtod(token.c_str(), &end_ptr);
			if (end_ptr == nullptr || *end_ptr != '\0' || errno == ERANGE || !std::isfinite(parsed))
			{
				return std::nullopt;
			}

			const bool has_fraction_or_exp =
				token.find('.') != std::string::npos ||
				token.find('e') != std::string::npos ||
				token.find('E') != std::string::npos;

			if (!has_fraction_or_exp)
			{
				errno = 0;
				char* int_end = nullptr;
				const long long parsed_int = std::strtoll(token.c_str(), &int_end, 10);
				if (int_end != nullptr && *int_end == '\0' && errno != ERANGE)
				{
					is_integer = true;
					integer_value = static_cast<int64_t>(parsed_int);
				}
			}

			return parsed;
		}

		std::string format_number(const Value& value)
		{
			if (value.is_integer)
			{
				return std::to_string(value.integer_value);
			}

			std::ostringstream stream;
			stream.setf(std::ios::fixed);
			stream << std::setprecision(6) << value.number_value;
			std::string out = stream.str();
			while (out.size() > 2u && out.back() == '0')
			{
				out.pop_back();
			}
			if (!out.empty() && out.back() == '.')
			{
				out.push_back('0');
			}
			if (out == "-0.0")
			{
				out = "0.0";
			}
			return out;
		}

		void append_string_bytes(std::vector<std::byte>& out, const std::string& text)
		{
			out.reserve(out.size() + text.size());
			for (const auto c : text)
			{
				out.push_back(static_cast<std::byte>(static_cast<uint8_t>(c)));
			}
		}

		void append_indent(std::vector<std::byte>& out, const size_t depth)
		{
			for (size_t i = 0u; i < depth; ++i)
			{
				out.push_back(static_cast<std::byte>('\t'));
			}
		}

		bool is_primitive(const Value& value)
		{
			return value.type != ValueType::Dictionary && value.type != ValueType::Array;
		}

		void serialize_value(const Value& value, std::vector<std::byte>& out, const size_t depth)
		{
			switch (value.type)
			{
			case ValueType::Dictionary:
			{
				append_string_bytes(out, "<<");
				if (!value.dictionary_items.empty())
				{
					append_string_bytes(out, "\n");
				}

				for (size_t i = 0u; i < value.dictionary_items.size(); ++i)
				{
					append_indent(out, depth + 1u);
					out.push_back(static_cast<std::byte>('/'));
					append_string_bytes(out, value.dictionary_items[i].first);
					out.push_back(static_cast<std::byte>(' '));
					serialize_value(value.dictionary_items[i].second, out, depth + 1u);
					if (i + 1u < value.dictionary_items.size())
					{
						append_string_bytes(out, "\n");
					}
				}

				if (!value.dictionary_items.empty())
				{
					append_string_bytes(out, "\n");
					append_indent(out, depth);
				}
				append_string_bytes(out, ">>");
				break;
			}
			case ValueType::Array:
			{
				const bool compact = std::all_of(value.array_items.begin(), value.array_items.end(), [](const Value& item)
				{
					return is_primitive(item);
				});

				if (compact)
				{
					append_string_bytes(out, "[");
					if (!value.array_items.empty())
					{
						out.push_back(static_cast<std::byte>(' '));
					}

					for (size_t i = 0u; i < value.array_items.size(); ++i)
					{
						serialize_value(value.array_items[i], out, depth + 1u);
						if (i + 1u < value.array_items.size())
						{
							out.push_back(static_cast<std::byte>(' '));
						}
					}

					if (!value.array_items.empty())
					{
						out.push_back(static_cast<std::byte>(' '));
					}
					append_string_bytes(out, "]");
					break;
				}

				append_string_bytes(out, "[");
				if (!value.array_items.empty())
				{
					append_string_bytes(out, "\n");
				}

				for (size_t i = 0u; i < value.array_items.size(); ++i)
				{
					append_indent(out, depth + 1u);
					serialize_value(value.array_items[i], out, depth + 1u);
					if (i + 1u < value.array_items.size())
					{
						append_string_bytes(out, "\n");
					}
				}

				if (!value.array_items.empty())
				{
					append_string_bytes(out, "\n");
					append_indent(out, depth);
				}
				append_string_bytes(out, "]");
				break;
			}
			case ValueType::Name:
				out.push_back(static_cast<std::byte>('/'));
				append_string_bytes(out, value.string_value);
				break;
			case ValueType::Number:
				append_string_bytes(out, format_number(value));
				break;
			case ValueType::Boolean:
				append_string_bytes(out, value.bool_value ? "true" : "false");
				break;
			case ValueType::LiteralString:
				out.push_back(static_cast<std::byte>('('));
				append_string_bytes(out, value.string_value);
				out.push_back(static_cast<std::byte>(')'));
				break;
			case ValueType::HexString:
				out.push_back(static_cast<std::byte>('<'));
				append_string_bytes(out, value.string_value);
				out.push_back(static_cast<std::byte>('>'));
				break;
			case ValueType::Identifier:
			default:
				append_string_bytes(out, value.string_value);
				break;
			}
		}

		class Parser
		{
		public:
			explicit Parser(const std::vector<std::byte>& data) : m_Data(data) {}

			ParseResult run()
			{
				ParseResult result{};
				skip_whitespace_and_comments();
				if (m_Cursor >= m_Data.size())
				{
					result.error_offset = m_Cursor;
					result.error_message = "EngineData payload is empty";
					return result;
				}

				Value root{};
				if (!parse_value(root))
				{
					result.error_offset = m_ErrorOffset;
					result.error_message = m_ErrorMessage;
					return result;
				}

				skip_whitespace_and_comments();
				if (m_Cursor != m_Data.size())
				{
					result.error_offset = m_Cursor;
					result.error_message = "Trailing data after root EngineData value";
					return result;
				}

				result.ok = true;
				result.root = std::move(root);
				return result;
			}

		private:
			const std::vector<std::byte>& m_Data;
			size_t m_Cursor = 0u;
			size_t m_ErrorOffset = 0u;
			std::string m_ErrorMessage{};

			void set_error(const std::string& message)
			{
				if (!m_ErrorMessage.empty())
				{
					return;
				}
				m_ErrorOffset = m_Cursor;
				m_ErrorMessage = message;
			}

			void skip_whitespace_and_comments()
			{
				while (m_Cursor < m_Data.size())
				{
					const auto c = to_u8(m_Data[m_Cursor]);
					if (is_ascii_whitespace(c))
					{
						++m_Cursor;
						continue;
					}

					if (c == static_cast<uint8_t>('%'))
					{
						++m_Cursor;
						while (m_Cursor < m_Data.size())
						{
							const auto cc = to_u8(m_Data[m_Cursor]);
							++m_Cursor;
							if (cc == static_cast<uint8_t>('\n') || cc == static_cast<uint8_t>('\r'))
							{
								break;
							}
						}
						continue;
					}

					break;
				}
			}

			bool parse_value(Value& out)
			{
				skip_whitespace_and_comments();
				if (m_Cursor >= m_Data.size())
				{
					set_error("Unexpected end of EngineData payload");
					return false;
				}

				const auto c = to_u8(m_Data[m_Cursor]);
				if (c == static_cast<uint8_t>('<'))
				{
					if (m_Cursor + 1u < m_Data.size() && to_u8(m_Data[m_Cursor + 1u]) == static_cast<uint8_t>('<'))
					{
						return parse_dictionary(out);
					}
					return parse_hex_string(out);
				}
				if (c == static_cast<uint8_t>('['))
				{
					return parse_array(out);
				}
				if (c == static_cast<uint8_t>('('))
				{
					return parse_literal_string(out);
				}
				if (c == static_cast<uint8_t>('/'))
				{
					return parse_name(out);
				}
				return parse_number_or_identifier(out);
			}

			bool parse_dictionary(Value& out)
			{
				const size_t start = m_Cursor;
				if (m_Cursor + 1u >= m_Data.size() ||
					to_u8(m_Data[m_Cursor]) != static_cast<uint8_t>('<') ||
					to_u8(m_Data[m_Cursor + 1u]) != static_cast<uint8_t>('<'))
				{
					set_error("Expected dictionary start token '<<'");
					return false;
				}
				m_Cursor += 2u;

				out = Value{};
				out.type = ValueType::Dictionary;
				out.start_offset = start;

				while (true)
				{
					skip_whitespace_and_comments();
					if (m_Cursor >= m_Data.size())
					{
						set_error("Unterminated dictionary in EngineData payload");
						return false;
					}

					if (m_Cursor + 1u < m_Data.size() &&
						to_u8(m_Data[m_Cursor]) == static_cast<uint8_t>('>') &&
						to_u8(m_Data[m_Cursor + 1u]) == static_cast<uint8_t>('>'))
					{
						m_Cursor += 2u;
						out.end_offset = m_Cursor;
						return true;
					}

					Value key_value{};
					if (!parse_name(key_value))
					{
						if (m_ErrorMessage.empty())
						{
							set_error("Expected dictionary key name");
						}
						return false;
					}

					Value value{};
					if (!parse_value(value))
					{
						return false;
					}

					out.dictionary_items.push_back(std::make_pair(key_value.string_value, std::move(value)));
				}
			}

			bool parse_array(Value& out)
			{
				const size_t start = m_Cursor;
				if (to_u8(m_Data[m_Cursor]) != static_cast<uint8_t>('['))
				{
					set_error("Expected array start token '['");
					return false;
				}
				++m_Cursor;

				out = Value{};
				out.type = ValueType::Array;
				out.start_offset = start;

				while (true)
				{
					skip_whitespace_and_comments();
					if (m_Cursor >= m_Data.size())
					{
						set_error("Unterminated array in EngineData payload");
						return false;
					}

					if (to_u8(m_Data[m_Cursor]) == static_cast<uint8_t>(']'))
					{
						++m_Cursor;
						out.end_offset = m_Cursor;
						return true;
					}

					Value item{};
					if (!parse_value(item))
					{
						return false;
					}
					out.array_items.push_back(std::move(item));
				}
			}

			bool parse_name(Value& out)
			{
				const size_t start = m_Cursor;
				if (to_u8(m_Data[m_Cursor]) != static_cast<uint8_t>('/'))
				{
					set_error("Expected name token starting with '/'");
					return false;
				}
				++m_Cursor;

				const size_t value_start = m_Cursor;
				while (m_Cursor < m_Data.size() && !is_value_delimiter(to_u8(m_Data[m_Cursor])))
				{
					++m_Cursor;
				}

				out = Value{};
				out.type = ValueType::Name;
				out.start_offset = start;
				out.end_offset = m_Cursor;
				out.string_value.reserve(m_Cursor - value_start);
				for (size_t i = value_start; i < m_Cursor; ++i)
				{
					out.string_value.push_back(static_cast<char>(to_u8(m_Data[i])));
				}
				return true;
			}

			bool parse_literal_string(Value& out)
			{
				const size_t start = m_Cursor;
				if (to_u8(m_Data[m_Cursor]) != static_cast<uint8_t>('('))
				{
					set_error("Expected literal string starting with '('");
					return false;
				}
				++m_Cursor;
				const size_t content_start = m_Cursor;

				bool escape = false;
				int32_t nesting = 1;
				while (m_Cursor < m_Data.size())
				{
					const auto c = to_u8(m_Data[m_Cursor]);
					++m_Cursor;

					if (escape)
					{
						escape = false;
						continue;
					}
					if (c == static_cast<uint8_t>('\\'))
					{
						escape = true;
						continue;
					}
					if (c == static_cast<uint8_t>('('))
					{
						++nesting;
						continue;
					}
					if (c == static_cast<uint8_t>(')'))
					{
						--nesting;
						if (nesting == 0)
						{
							out = Value{};
							out.type = ValueType::LiteralString;
							out.start_offset = start;
							out.end_offset = m_Cursor;
							out.string_value.reserve((m_Cursor - 1u) - content_start);
							for (size_t i = content_start; i + 1u < m_Cursor; ++i)
							{
								out.string_value.push_back(static_cast<char>(to_u8(m_Data[i])));
							}
							return true;
						}
					}
				}

				set_error("Unterminated literal string in EngineData payload");
				return false;
			}

			bool parse_hex_string(Value& out)
			{
				const size_t start = m_Cursor;
				if (to_u8(m_Data[m_Cursor]) != static_cast<uint8_t>('<'))
				{
					set_error("Expected hex string starting with '<'");
					return false;
				}
				++m_Cursor;
				const size_t content_start = m_Cursor;

				while (m_Cursor < m_Data.size())
				{
					if (to_u8(m_Data[m_Cursor]) == static_cast<uint8_t>('>'))
					{
						out = Value{};
						out.type = ValueType::HexString;
						out.start_offset = start;
						out.end_offset = m_Cursor + 1u;
						out.string_value.reserve(m_Cursor - content_start);
						for (size_t i = content_start; i < m_Cursor; ++i)
						{
							out.string_value.push_back(static_cast<char>(to_u8(m_Data[i])));
						}
						++m_Cursor;
						return true;
					}
					++m_Cursor;
				}

				set_error("Unterminated hex string in EngineData payload");
				return false;
			}

			bool parse_number_or_identifier(Value& out)
			{
				const size_t start = m_Cursor;
				while (m_Cursor < m_Data.size() && !is_value_delimiter(to_u8(m_Data[m_Cursor])))
				{
					++m_Cursor;
				}
				if (m_Cursor == start)
				{
					set_error("Unexpected token while parsing EngineData value");
					return false;
				}

				std::string token{};
				token.reserve(m_Cursor - start);
				for (size_t i = start; i < m_Cursor; ++i)
				{
					token.push_back(static_cast<char>(to_u8(m_Data[i])));
				}

				out = Value{};
				out.start_offset = start;
				out.end_offset = m_Cursor;

				if (token == "true" || token == "false")
				{
					out.type = ValueType::Boolean;
					out.bool_value = token == "true";
					return true;
				}

				bool is_integer = false;
				int64_t integer_value = 0;
				const auto parsed_number = parse_double_token(token, is_integer, integer_value);
				if (parsed_number.has_value())
				{
					out.type = ValueType::Number;
					out.number_value = parsed_number.value();
					out.is_integer = is_integer;
					out.integer_value = integer_value;
					return true;
				}

				out.type = ValueType::Identifier;
				out.string_value = std::move(token);
				return true;
			}
		};
	}

	ParseResult parse(const std::vector<std::byte>& data)
	{
		Parser parser(data);
		return parser.run();
	}

	std::vector<std::byte> serialize(const Value& value)
	{
		std::vector<std::byte> out{};
		// Photoshop expects two leading newlines before the root '<<'
		out.push_back(std::byte{0x0A});
		out.push_back(std::byte{0x0A});
		serialize_value(value, out, 0u);
		out.push_back(static_cast<std::byte>('\n'));
		return out;
	}

	std::vector<std::byte> format_value_bytes(const Value& value, const size_t depth)
	{
		std::vector<std::byte> out{};
		serialize_value(value, out, depth);
		return out;
	}

	void splice_payload(std::vector<std::byte>& payload, const size_t old_start, const size_t old_end, const std::vector<std::byte>& new_bytes)
	{
		payload.erase(
			payload.begin() + static_cast<std::ptrdiff_t>(old_start),
			payload.begin() + static_cast<std::ptrdiff_t>(old_end));
		payload.insert(
			payload.begin() + static_cast<std::ptrdiff_t>(old_start),
			new_bytes.begin(), new_bytes.end());
	}

	void apply_patches(std::vector<std::byte>& payload, std::vector<PayloadPatch>& patches)
	{
		std::sort(patches.begin(), patches.end(), [](const PayloadPatch& a, const PayloadPatch& b)
		{
			return a.old_start > b.old_start;
		});
		for (const auto& p : patches)
		{
			splice_payload(payload, p.old_start, p.old_end, p.new_bytes);
		}
	}

	bool insert_dict_entry_bytes(std::vector<std::byte>& payload, const Value& parent_dict, const std::string& key, const Value& value)
	{
		if (parent_dict.type != ValueType::Dictionary) return false;
		if (parent_dict.end_offset < 2u) return false;

		// Position of the first '>' in the closing ">>"
		const size_t closing = parent_dict.end_offset - 2u;

		// Determine indentation: count tabs at the start of the ">>" line.
		size_t line_start = closing;
		while (line_start > 0u)
		{
			const auto c = std::to_integer<uint8_t>(payload[line_start - 1u]);
			if (c == static_cast<uint8_t>('\n') || c == static_cast<uint8_t>('\r'))
				break;
			--line_start;
		}
		size_t parent_tabs = 0u;
		while (line_start + parent_tabs < closing &&
			std::to_integer<uint8_t>(payload[line_start + parent_tabs]) == static_cast<uint8_t>('\t'))
		{
			++parent_tabs;
		}

		// Build entry bytes: <tabs>/Key value\n   (entry depth = parent_tabs + 1)
		std::vector<std::byte> entry_bytes{};
		for (size_t i = 0u; i <= parent_tabs; ++i)
			entry_bytes.push_back(static_cast<std::byte>('\t'));
		entry_bytes.push_back(static_cast<std::byte>('/'));
		for (const auto c : key)
			entry_bytes.push_back(static_cast<std::byte>(static_cast<uint8_t>(c)));
		entry_bytes.push_back(static_cast<std::byte>(' '));

		auto value_bytes = format_value_bytes(value);
		entry_bytes.insert(entry_bytes.end(), value_bytes.begin(), value_bytes.end());
		entry_bytes.push_back(static_cast<std::byte>('\n'));

		// Insert before the ">>" delimiter
		payload.insert(
			payload.begin() + static_cast<std::ptrdiff_t>(closing),
			entry_bytes.begin(), entry_bytes.end());

		return true;
	}

	bool insert_array_item_bytes(std::vector<std::byte>& payload, const Value& parent_array, const Value& item)
	{
		if (parent_array.type != ValueType::Array) return false;
		if (parent_array.end_offset < 1u) return false;

		// Position of the closing ']'
		const size_t closing = parent_array.end_offset - 1u;

		// Determine indentation: count tabs at the start of the ']' line.
		size_t line_start = closing;
		while (line_start > 0u)
		{
			const auto c = std::to_integer<uint8_t>(payload[line_start - 1u]);
			if (c == static_cast<uint8_t>('\n') || c == static_cast<uint8_t>('\r'))
				break;
			--line_start;
		}
		size_t parent_tabs = 0u;
		while (line_start + parent_tabs < closing &&
			std::to_integer<uint8_t>(payload[line_start + parent_tabs]) == static_cast<uint8_t>('\t'))
		{
			++parent_tabs;
		}

		const size_t item_depth = parent_tabs + 1u;

		// Build item bytes: <tabs><formatted_value>\n
		std::vector<std::byte> item_bytes{};
		for (size_t i = 0u; i < item_depth; ++i)
			item_bytes.push_back(static_cast<std::byte>('\t'));

		auto value_bytes = format_value_bytes(item, item_depth);
		item_bytes.insert(item_bytes.end(), value_bytes.begin(), value_bytes.end());
		item_bytes.push_back(static_cast<std::byte>('\n'));

		// Insert before the ']' delimiter
		payload.insert(
			payload.begin() + static_cast<std::ptrdiff_t>(closing),
			item_bytes.begin(), item_bytes.end());

		return true;
	}

	const Value* find_dict_value(const Value& dictionary, std::string_view key)
	{
		if (dictionary.type != ValueType::Dictionary)
		{
			return nullptr;
		}

		for (const auto& item : dictionary.dictionary_items)
		{
			if (item.first == key)
			{
				return &item.second;
			}
		}
		return nullptr;
	}

	Value* find_dict_value(Value& dictionary, std::string_view key)
	{
		if (dictionary.type != ValueType::Dictionary)
		{
			return nullptr;
		}

		for (auto& item : dictionary.dictionary_items)
		{
			if (item.first == key)
			{
				return &item.second;
			}
		}
		return nullptr;
	}

	const Value* find_by_path(const Value& dictionary, std::initializer_list<std::string_view> path)
	{
		const Value* cursor = &dictionary;
		for (const auto key : path)
		{
			cursor = find_dict_value(*cursor, key);
			if (cursor == nullptr)
			{
				return nullptr;
			}
		}
		return cursor;
	}

	Value* find_by_path(Value& dictionary, std::initializer_list<std::string_view> path)
	{
		Value* cursor = &dictionary;
		for (const auto key : path)
		{
			cursor = find_dict_value(*cursor, key);
			if (cursor == nullptr)
			{
				return nullptr;
			}
		}
		return cursor;
	}

	bool as_int32_vector(const Value& array_value, std::vector<int32_t>& out)
	{
		if (array_value.type != ValueType::Array)
		{
			return false;
		}

		out.clear();
		out.reserve(array_value.array_items.size());
		for (const auto& item : array_value.array_items)
		{
			if (item.type != ValueType::Number)
			{
				return false;
			}

			double rounded = std::round(item.number_value);
			if (!std::isfinite(rounded) || std::fabs(rounded - item.number_value) > 0.000001)
			{
				return false;
			}
			if (rounded < static_cast<double>(std::numeric_limits<int32_t>::min()) ||
				rounded > static_cast<double>(std::numeric_limits<int32_t>::max()))
			{
				return false;
			}
			out.push_back(static_cast<int32_t>(rounded));
		}
		return true;
	}

	bool as_double_vector(const Value& array_value, std::vector<double>& out)
	{
		if (array_value.type != ValueType::Array)
		{
			return false;
		}

		out.clear();
		out.reserve(array_value.array_items.size());
		for (const auto& item : array_value.array_items)
		{
			if (item.type != ValueType::Number)
			{
				return false;
			}
			out.push_back(item.number_value);
		}
		return true;
	}

	bool set_number(Value& value, double number)
	{
		if (value.type != ValueType::Number || !std::isfinite(number))
		{
			return false;
		}
		// Preserve the original integer/float type.  If the value was
		// originally a float (e.g. FontSize 28.0) it must stay a float
		// even when the new value is a whole number (e.g. 20.0), because
		// Photoshop distinguishes integer and float EngineData types.
		const bool was_integer = value.is_integer;
		value.number_value = number;
		const auto rounded = std::round(number);
		if (was_integer &&
			std::fabs(rounded - number) <= 0.000001 &&
			rounded >= static_cast<double>(std::numeric_limits<int64_t>::min()) &&
			rounded <= static_cast<double>(std::numeric_limits<int64_t>::max()))
		{
			value.is_integer = true;
			value.integer_value = static_cast<int64_t>(rounded);
		}
		else
		{
			value.is_integer = false;
		}
		return true;
	}

	bool set_int32_array(Value& value, const std::vector<int32_t>& values)
	{
		if (value.type != ValueType::Array)
		{
			return false;
		}

		value.array_items.clear();
		value.array_items.reserve(values.size());
		for (const auto v : values)
		{
			Value item{};
			item.type = ValueType::Number;
			item.number_value = static_cast<double>(v);
			item.is_integer = true;
			item.integer_value = v;
			value.array_items.push_back(std::move(item));
		}
		return true;
	}

	bool set_double_array(Value& value, const std::vector<double>& values)
	{
		if (value.type != ValueType::Array)
		{
			return false;
		}

		value.array_items.clear();
		value.array_items.reserve(values.size());
		for (const auto v : values)
		{
			if (!std::isfinite(v))
			{
				return false;
			}

			Value item{};
			item.type = ValueType::Number;
			item.number_value = v;
			const auto rounded = std::round(v);
			item.is_integer = std::fabs(rounded - v) <= 0.000001 &&
				rounded >= static_cast<double>(std::numeric_limits<int64_t>::min()) &&
				rounded <= static_cast<double>(std::numeric_limits<int64_t>::max());
			if (item.is_integer)
			{
				item.integer_value = static_cast<int64_t>(rounded);
			}
			value.array_items.push_back(std::move(item));
		}
		return true;
	}

	bool set_bool(Value& value, const bool b)
	{
		if (value.type != ValueType::Boolean)
		{
			return false;
		}
		value.bool_value = b;
		return true;
	}

	bool set_name(Value& value, const std::string& name)
	{
		if (value.type != ValueType::Name)
		{
			return false;
		}
		value.string_value = name;
		return true;
	}

	bool set_string(Value& value, const std::string& str)
	{
		if (value.type != ValueType::LiteralString)
		{
			return false;
		}
		value.string_value = str;
		return true;
	}

	// ---------- factory helpers ----------

	Value make_number(const double number)
	{
		Value v{};
		v.type = ValueType::Number;
		v.number_value = number;
		const auto rounded = std::round(number);
		v.is_integer = std::isfinite(number) &&
			std::fabs(rounded - number) <= 0.000001 &&
			rounded >= static_cast<double>(std::numeric_limits<int64_t>::min()) &&
			rounded <= static_cast<double>(std::numeric_limits<int64_t>::max());
		if (v.is_integer)
		{
			v.integer_value = static_cast<int64_t>(rounded);
		}
		return v;
	}

	Value make_integer(const int64_t integer)
	{
		Value v{};
		v.type = ValueType::Number;
		v.number_value = static_cast<double>(integer);
		v.is_integer = true;
		v.integer_value = integer;
		return v;
	}

	Value make_bool(const bool b)
	{
		Value v{};
		v.type = ValueType::Boolean;
		v.bool_value = b;
		return v;
	}

	Value make_name(const std::string& name)
	{
		Value v{};
		v.type = ValueType::Name;
		v.string_value = name;
		return v;
	}

	Value make_string(const std::string& str)
	{
		Value v{};
		v.type = ValueType::LiteralString;
		v.string_value = str;
		return v;
	}

	Value make_dict()
	{
		Value v{};
		v.type = ValueType::Dictionary;
		return v;
	}

	Value make_array()
	{
		Value v{};
		v.type = ValueType::Array;
		return v;
	}

	// ---------- structural mutation ----------

	bool insert_dict_value(Value& dictionary, const std::string& key, Value value)
	{
		if (dictionary.type != ValueType::Dictionary)
		{
			return false;
		}

		for (auto& item : dictionary.dictionary_items)
		{
			if (item.first == key)
			{
				item.second = std::move(value);
				return true;
			}
		}

		dictionary.dictionary_items.push_back(std::make_pair(key, std::move(value)));
		return true;
	}

	bool remove_dict_value(Value& dictionary, const std::string& key)
	{
		if (dictionary.type != ValueType::Dictionary)
		{
			return false;
		}

		for (auto it = dictionary.dictionary_items.begin(); it != dictionary.dictionary_items.end(); ++it)
		{
			if (it->first == key)
			{
				dictionary.dictionary_items.erase(it);
				return true;
			}
		}
		return false;
	}

	bool append_array_item(Value& array, Value item)
	{
		if (array.type != ValueType::Array)
		{
			return false;
		}
		array.array_items.push_back(std::move(item));
		return true;
	}
}

PSAPI_NAMESPACE_END
