#pragma once

#include "Core/TaggedBlocks/TaggedBlock.h"
#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"
#include "Util/Enum.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace
{
	[[maybe_unused]] std::filesystem::path temp_psd_path()
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

	[[maybe_unused]] uint8_t to_u8(const std::byte value)
	{
		return std::to_integer<uint8_t>(value);
	}

	[[maybe_unused]] uint32_t read_u32_be(const std::vector<std::byte>& data, const size_t offset)
	{
		return (static_cast<uint32_t>(to_u8(data[offset])) << 24) |
			(static_cast<uint32_t>(to_u8(data[offset + 1u])) << 16) |
			(static_cast<uint32_t>(to_u8(data[offset + 2u])) << 8) |
			static_cast<uint32_t>(to_u8(data[offset + 3u]));
	}

	[[maybe_unused]] bool ascii_equal_at(const std::vector<std::byte>& data, const size_t offset, const std::string_view needle)
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

	[[maybe_unused]] std::optional<size_t> find_ascii(const std::vector<std::byte>& data, const std::string_view needle, const size_t start = 0u)
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

	[[maybe_unused]] std::vector<std::vector<int32_t>> extract_engine_run_length_arrays(const NAMESPACE_PSAPI::TaggedBlock& block)
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

	[[maybe_unused]] std::optional<int32_t> extract_engine_int_value(const NAMESPACE_PSAPI::TaggedBlock& block, const std::string_view key)
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
}
