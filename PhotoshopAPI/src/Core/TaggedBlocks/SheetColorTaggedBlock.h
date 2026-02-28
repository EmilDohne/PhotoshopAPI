#pragma once

#include "Macros.h"
#include "TaggedBlock.h"

#include "Core/Struct/File.h"
#include "Core/Struct/Signature.h"
#include "Util/ProgressCallback.h"
#include "Util/Enum.h"
#include "Core/FileIO/LengthMarkers.h"

#include <variant>

PSAPI_NAMESPACE_BEGIN

struct SheetColorTaggedBlock : TaggedBlock
{
	Enum::LayerColor m_Color = Enum::LayerColor::none;

	SheetColorTaggedBlock() = default;
	SheetColorTaggedBlock(std::variant<Enum::LayerColor, uint16_t> value)
	{
		if (std::holds_alternative<uint16_t>(value))
		{
			m_Color = Enum::from_integer<Enum::LayerColor, uint16_t>(std::get<uint16_t>(value));
		}
		else
		{
			m_Color = std::get<Enum::LayerColor>(value);
		}

		m_Key = Enum::TaggedBlockKey::lrSheetColorSetting;
	};

	void read(File& document, const uint64_t offset, const Signature signature, [[maybe_unused]] const uint16_t padding = 1u)
	{
		m_Key = Enum::TaggedBlockKey::lrSheetColorSetting;
		m_Offset = offset;
		m_Signature = signature;
		auto length = ReadBinaryData<uint32_t>(document);

		if (length != 8u)
		{
			throw std::runtime_error(
				std::format(
					"Error while reading SheetColorSetting tagged block, expected exactly 8 bytes of size but instead"
					" got {}", length
				)
			);
		}

		m_Color = Enum::from_integer<Enum::LayerColor, uint16_t>(ReadBinaryData<uint16_t>(document));
		document.skip(6u);
	}
	void write(File& document, [[maybe_unused]] const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, const uint16_t padding = 1u) override
	{
		WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
		WriteBinaryData<uint32_t>(document, Signature("lclr").m_Value);
		Impl::ScopedLengthBlock<uint32_t> len_block(document, padding);

		WriteBinaryData<uint16_t>(document, static_cast<uint16_t>(Enum::as_integer(m_Color)));
		WritePadddingBytes(document, 6u);
	}
};

PSAPI_NAMESPACE_END