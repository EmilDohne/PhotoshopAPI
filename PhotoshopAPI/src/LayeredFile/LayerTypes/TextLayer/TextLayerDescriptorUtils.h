#pragma once

// =========================================================================
//  TextLayerDescriptorUtils.h
//
//  Photoshop binary descriptor format walking / enumeration utilities.
//  These operate on raw std::vector<std::byte> buffers containing
//  Photoshop Descriptor data structures (OSType-tagged key-value pairs).
// =========================================================================

#include "Macros.h"
#include "TextLayerTypes.h"
#include "TextLayerParsingUtils.h"

#include "Core/Struct/DescriptorStructure.h"
#include "Core/Struct/File.h"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

PSAPI_NAMESPACE_BEGIN

namespace TextLayerDetail
{
inline std::filesystem::path make_descriptor_temp_path()
{
	static std::atomic<uint64_t> counter{ 0u };
	const uint64_t tick = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
	const uint64_t idx = counter.fetch_add(1u, std::memory_order_relaxed);
	return std::filesystem::temp_directory_path() /
		("psapi_descriptor_" + std::to_string(tick) + "_" + std::to_string(idx) + ".bin");
}

inline std::vector<std::byte> serialize_descriptor_body(const Descriptors::Descriptor& descriptor)
{
	const auto temp_path = make_descriptor_temp_path();
	{
		File::FileParams write_params{};
		write_params.doRead = false;
		write_params.forceOverwrite = true;
		File output(temp_path, write_params);
		descriptor.write(output);
	}

	std::vector<std::byte> bytes{};
	{
		File input(temp_path);
		bytes.resize(static_cast<size_t>(input.getSize()));
		input.read(std::span<uint8_t>(reinterpret_cast<uint8_t*>(bytes.data()), bytes.size()));
	}

	std::error_code ec{};
	std::filesystem::remove(temp_path, ec);
	return bytes;
}

inline bool parse_descriptor_body(
	const std::vector<std::byte>& data,
	const size_t offset,
	Descriptors::Descriptor& out_descriptor)
{
	if (offset >= data.size())
	{
		return false;
	}

	const auto temp_path = make_descriptor_temp_path();
	{
		File::FileParams write_params{};
		write_params.doRead = false;
		write_params.forceOverwrite = true;
		File output(temp_path, write_params);

		std::vector<uint8_t> tail{};
		tail.reserve(data.size() - offset);
		for (size_t i = offset; i < data.size(); ++i)
		{
			tail.push_back(std::to_integer<uint8_t>(data[i]));
		}
		output.write(std::span<uint8_t>(tail.data(), tail.size()));
	}

	bool success = true;
	try
	{
		File input(temp_path);
		Descriptors::Descriptor parsed{};
		parsed.read(input);
		out_descriptor = std::move(parsed);
	}
	catch (...)
	{
		success = false;
	}

	std::error_code ec{};
	std::filesystem::remove(temp_path, ec);
	return success;
}

// -----------------------------------------------------------------------
//  Shared key-reading helper
// -----------------------------------------------------------------------

/// Read a Photoshop descriptor key (length-prefixed, 0-length → 4-byte charID).
/// Returns { key_string, total_bytes_consumed }. Returns { "", 0 } on error.
inline std::pair<std::string, size_t> read_descriptor_key(const std::vector<std::byte>& data, size_t offset)
{
	if (offset + 4u > data.size()) return { "", 0u };
	const uint32_t len = read_u32_be(data, offset);
	const uint32_t actual = (len == 0u) ? 4u : len;
	if (offset + 4u + actual > data.size()) return { "", 0u };
	std::string key;
	for (uint32_t i = 0u; i < actual; ++i)
		key.push_back(static_cast<char>(to_u8(data[offset + 4u + i])));
	return { key, 4u + actual };
}

// -----------------------------------------------------------------------
//  Descriptor skip / enumerate
// -----------------------------------------------------------------------

/// Skip a descriptor body and return the number of bytes consumed (0 on error).
inline size_t skip_descriptor_body(const std::vector<std::byte>& data, size_t pos);

/// Skip a single descriptor value (OSType tag + payload) and return bytes consumed.
inline size_t skip_descriptor_value(const std::vector<std::byte>& data, size_t pos)
{
	if (pos + 4u > data.size()) return 0u;
	const char t0 = static_cast<char>(to_u8(data[pos]));
	const char t1 = static_cast<char>(to_u8(data[pos + 1u]));
	const char t2 = static_cast<char>(to_u8(data[pos + 2u]));
	const char t3 = static_cast<char>(to_u8(data[pos + 3u]));
	size_t p = pos + 4u;

	auto match4 = [](char a, char b, char c, char d, const char* s) {
		return a == s[0] && b == s[1] && c == s[2] && d == s[3];
	};

	auto read_key = [&](size_t off) -> size_t {
		if (off + 4u > data.size()) return 0u;
		const uint32_t kl = read_u32_be(data, off);
		return 4u + ((kl == 0u) ? 4u : static_cast<size_t>(kl));
	};

	if      (match4(t0,t1,t2,t3, "long")) { p += 4u; }
	else if (match4(t0,t1,t2,t3, "doub")) { p += 8u; }
	else if (match4(t0,t1,t2,t3, "bool")) { p += 1u; }
	else if (match4(t0,t1,t2,t3, "comp")) { p += 8u; }
	else if (match4(t0,t1,t2,t3, "TEXT"))
	{
		if (p + 4u > data.size()) return 0u;
		const uint32_t cnt = read_u32_be(data, p); p += 4u;
		p += static_cast<size_t>(cnt) * 2u;
	}
	else if (match4(t0,t1,t2,t3, "enum"))
	{
		const size_t a = read_key(p); if (a == 0u) return 0u; p += a;
		const size_t b = read_key(p); if (b == 0u) return 0u; p += b;
	}
	else if (match4(t0,t1,t2,t3, "UntF")) { p += 4u + 8u; }
	else if (match4(t0,t1,t2,t3, "UnFl"))
	{
		p += 4u; // unit tag
		if (p + 4u > data.size()) return 0u;
		const uint32_t cnt = read_u32_be(data, p); p += 4u;
		p += static_cast<size_t>(cnt) * 8u;
	}
	else if (match4(t0,t1,t2,t3, "Objc") || match4(t0,t1,t2,t3, "GlbO") || match4(t0,t1,t2,t3, "ObAr"))
	{
		const size_t c = skip_descriptor_body(data, p);
		if (c == 0u) return 0u;
		p += c;
	}
	else if (match4(t0,t1,t2,t3, "VlLs"))
	{
		if (p + 4u > data.size()) return 0u;
		const uint32_t cnt = read_u32_be(data, p); p += 4u;
		for (uint32_t i = 0u; i < cnt; ++i)
		{
			const size_t c = skip_descriptor_value(data, p);
			if (c == 0u) return 0u;
			p += c;
		}
	}
	else if (match4(t0,t1,t2,t3, "tdta") || match4(t0,t1,t2,t3, "alis") || match4(t0,t1,t2,t3, "Pth "))
	{
		if (p + 4u > data.size()) return 0u;
		const uint32_t cnt = read_u32_be(data, p); p += 4u;
		p += cnt;
	}
	else if (match4(t0,t1,t2,t3, "type") || match4(t0,t1,t2,t3, "GlbC") || match4(t0,t1,t2,t3, "Clss"))
	{
		if (p + 4u > data.size()) return 0u;
		const uint32_t cn = read_u32_be(data, p); p += 4u;
		p += static_cast<size_t>(cn) * 2u;
		const size_t a = read_key(p); if (a == 0u) return 0u; p += a;
	}
	else if (match4(t0,t1,t2,t3, "obj "))
	{
		// Reference type – walk sub-items
		if (p + 4u > data.size()) return 0u;
		const uint32_t cnt = read_u32_be(data, p); p += 4u;
		for (uint32_t i = 0u; i < cnt; ++i)
		{
			if (p + 4u > data.size()) return 0u;
			const char r0 = static_cast<char>(to_u8(data[p]));
			const char r1 = static_cast<char>(to_u8(data[p + 1u]));
			const char r2 = static_cast<char>(to_u8(data[p + 2u]));
			const char r3 = static_cast<char>(to_u8(data[p + 3u]));
			p += 4u;
			if (match4(r0,r1,r2,r3, "prop") || match4(r0,r1,r2,r3, "Clss") || match4(r0,r1,r2,r3, "name"))
			{
				if (p + 4u > data.size()) return 0u;
				const uint32_t cn = read_u32_be(data, p); p += 4u; p += static_cast<size_t>(cn) * 2u;
				const size_t a = read_key(p); if (a == 0u) return 0u; p += a;
				const size_t b = read_key(p); if (b == 0u) return 0u; p += b;
			}
			else if (match4(r0,r1,r2,r3, "Enmr"))
			{
				if (p + 4u > data.size()) return 0u;
				const uint32_t cn = read_u32_be(data, p); p += 4u; p += static_cast<size_t>(cn) * 2u;
				const size_t a = read_key(p); if (a == 0u) return 0u; p += a;
				const size_t b = read_key(p); if (b == 0u) return 0u; p += b;
				const size_t c = read_key(p); if (c == 0u) return 0u; p += c;
			}
			else if (match4(r0,r1,r2,r3, "rele"))
			{
				if (p + 4u > data.size()) return 0u;
				const uint32_t cn = read_u32_be(data, p); p += 4u; p += static_cast<size_t>(cn) * 2u;
				const size_t a = read_key(p); if (a == 0u) return 0u; p += a;
				p += 4u;
			}
			else if (match4(r0,r1,r2,r3, "Idnt") || match4(r0,r1,r2,r3, "indx")) { p += 4u; }
			else { return 0u; }
		}
	}
	else
	{
		return 0u; // unknown OSType
	}

	if (p > data.size()) return 0u;
	return p - pos;
}

inline size_t skip_descriptor_body(const std::vector<std::byte>& data, size_t pos)
{
	const size_t start = pos;
	if (pos + 4u > data.size()) return 0u;
	const uint32_t class_name_len = read_u32_be(data, pos); pos += 4u;
	pos += static_cast<size_t>(class_name_len) * 2u;
	// classID
	if (pos + 4u > data.size()) return 0u;
	const uint32_t cid_len = read_u32_be(data, pos); pos += 4u;
	pos += (cid_len == 0u) ? 4u : static_cast<size_t>(cid_len);
	// count
	if (pos + 4u > data.size()) return 0u;
	const uint32_t count = read_u32_be(data, pos); pos += 4u;
	for (uint32_t i = 0u; i < count; ++i)
	{
		// key
		if (pos + 4u > data.size()) return 0u;
		const uint32_t klen = read_u32_be(data, pos); pos += 4u;
		pos += (klen == 0u) ? 4u : static_cast<size_t>(klen);
		// value
		const size_t vbytes = skip_descriptor_value(data, pos);
		if (vbytes == 0u) return 0u;
		pos += vbytes;
	}
	return pos - start;
}

/// Walk a descriptor body and collect the top-level key/value entries.
inline std::vector<DescKeyValue> enumerate_descriptor_keys(
	const std::vector<std::byte>& data, size_t pos)
{
	std::vector<DescKeyValue> result;
	if (pos + 4u > data.size()) return result;
	const uint32_t class_name_len = read_u32_be(data, pos); pos += 4u;
	pos += static_cast<size_t>(class_name_len) * 2u;
	// classID
	if (pos + 4u > data.size()) return result;
	const uint32_t cid_len = read_u32_be(data, pos); pos += 4u;
	pos += (cid_len == 0u) ? 4u : static_cast<size_t>(cid_len);
	// count
	if (pos + 4u > data.size()) return result;
	const uint32_t count = read_u32_be(data, pos); pos += 4u;
	for (uint32_t i = 0u; i < count; ++i)
	{
		// key
		if (pos + 4u > data.size()) break;
		const uint32_t klen = read_u32_be(data, pos); pos += 4u;
		const uint32_t kact = (klen == 0u) ? 4u : klen;
		if (pos + kact > data.size()) break;
		std::string key;
		for (uint32_t j = 0u; j < kact; ++j)
			key.push_back(static_cast<char>(to_u8(data[pos + j])));
		pos += kact;
		// OSType tag
		if (pos + 4u > data.size()) break;
		std::string ostype;
		for (size_t j = 0u; j < 4u; ++j)
			ostype.push_back(static_cast<char>(to_u8(data[pos + j])));
		const size_t data_off = pos + 4u;  // right after the OSType tag
		result.push_back(DescKeyValue{ std::move(key), std::move(ostype), data_off });
		// skip the whole value
		const size_t vbytes = skip_descriptor_value(data, pos);
		if (vbytes == 0u) break;
		pos += vbytes;
	}
	return result;
}

} // namespace TextLayerDetail

PSAPI_NAMESPACE_END
