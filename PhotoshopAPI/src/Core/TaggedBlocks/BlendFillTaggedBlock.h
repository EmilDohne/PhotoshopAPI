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

struct BlendFillTaggedBlock : TaggedBlock
{
    uint8_t m_Fill = 255u;

    BlendFillTaggedBlock() = default;

    explicit BlendFillTaggedBlock(const uint8_t value) {m_Fill = value;};

    void read(File& document, const uint64_t offset, const Signature signature, [[maybe_unused]] const uint16_t padding = 1u)
    {
        m_Key = Enum::TaggedBlockKey::lrSheetColorSetting;
        m_Offset = offset;
        m_Signature = signature;

        if (const auto length = ReadBinaryData<uint32_t>(document); length != 4u)
        {
            throw std::runtime_error(
                std::format(
                    "Error while reading BlendFillTaggedBlock tagged block, expected exactly 4 bytes of size but instead"
                    " got {}", length
                )
            );
        }

        m_Fill = ReadBinaryData<uint8_t>(document);
        document.skip(3u);
    }
    void write(File& document, [[maybe_unused]] const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, const uint16_t padding = 1u) override
    {
        WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
        WriteBinaryData<uint32_t>(document, Signature("iOpa").m_Value);
        Impl::ScopedLengthBlock<uint32_t> len_block(document, padding);

        WriteBinaryData<uint8_t>(document, m_Fill);
        WritePadddingBytes(document, 3u);
    }
};

PSAPI_NAMESPACE_END