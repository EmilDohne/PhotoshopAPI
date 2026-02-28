#pragma once

#include "Macros.h"
#include "TaggedBlock.h"

#include "Core/Struct/DescriptorStructure.h"
#include "Core/Struct/File.h"
#include "Core/Struct/Signature.h"
#include "PhotoshopFile/FileHeader.h"
#include "Util/Enum.h"
#include "Util/ProgressCallback.h"

#include <array>
#include <cstdint>
#include <vector>

PSAPI_NAMESPACE_BEGIN

/// Typed representation of the lrTypeTool ("TySh") tagged block.
///
/// This keeps parsed text/warp descriptors in memory so callers can mutate
/// descriptors directly without reparsing the raw payload on every call.
struct TypeToolTaggedBlock : TaggedBlock
{
	TypeToolTaggedBlock();

	void read(
		File& document,
		const FileHeader& header,
		const uint64_t offset,
		const Signature signature,
		const uint16_t padding = 1u);

	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) override;

	/// Rebuild raw TySh bytes from parsed descriptor members.
	void sync_data_from_descriptors();

	/// Reparse descriptor members from the current raw TySh bytes.
	bool parse_descriptors_from_data();

	bool has_parsed_descriptors() const noexcept { return m_DescriptorsParsed; }

	Descriptors::Descriptor& text_descriptor() noexcept { return m_TextDescriptor; }
	const Descriptors::Descriptor& text_descriptor() const noexcept { return m_TextDescriptor; }

	Descriptors::Descriptor& warp_descriptor() noexcept { return m_WarpDescriptor; }
	const Descriptors::Descriptor& warp_descriptor() const noexcept { return m_WarpDescriptor; }

	uint16_t text_version() const noexcept { return m_TextVersion; }
	uint16_t warp_version() const noexcept { return m_WarpVersion; }
	uint32_t text_descriptor_version() const noexcept { return m_TextDescriptorVersion; }
	uint32_t warp_descriptor_version() const noexcept { return m_WarpDescriptorVersion; }

private:
	uint16_t m_TyShVersion = 1u;
	std::array<double, 6u> m_Transform{ 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };

	uint16_t m_TextVersion = 50u;
	uint32_t m_TextDescriptorVersion = 16u;
	Descriptors::Descriptor m_TextDescriptor{ "TxLr" };

	uint16_t m_WarpVersion = 1u;
	uint32_t m_WarpDescriptorVersion = 16u;
	Descriptors::Descriptor m_WarpDescriptor{ "warp" };
	std::vector<std::byte> m_WarpTrailingData{};

	bool m_DescriptorsParsed = false;
};

PSAPI_NAMESPACE_END

