#include "Signature.h"

#include "Macros.h"
#include "Logger.h"
#include "Core/FileIO/Read.h"
#include "Core/FileIO/Write.h"

#include <string>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

PSAPI_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
Signature::Signature(const uint32_t val)
{
    for (int i = 0; i < 4; ++i) {
        m_Representation[i] = static_cast<char>((val >> (8 * (3 - i))));
    }
    m_Value = val;
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
Signature::Signature(const std::string val)
{
    if (val.length() < 4)
    {
        PSAPI_LOG_ERROR("Signature", "Signature cannot get initialized with less than 4 characters, got %s", val.c_str());
    }
    if (val.length() > 4)
    {
        PSAPI_LOG_WARNING("Signature", "Signature struct has a length of 4, the last %i characters of %s will be cut off", val.length() - 4, val.c_str());
    }

    m_Value = 0;
    for (int i = 0; i < 4; ++i) {
        m_Value = (m_Value << 8) | (uint8_t)val[i];
        m_Representation[i] = val[i];
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
bool Signature::operator==(const Signature& other)
{
    return m_Value == other.m_Value;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
bool Signature::operator==(const std::string& other)
{
    auto repr_str = std::string(m_Representation.begin(), m_Representation.end());
    return repr_str == other;
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
bool Signature::operator==(const uint32_t other)
{
    return m_Value == other;
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
bool Signature::operator!=(const Signature& other)
{
    return m_Value != other.m_Value;
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
bool Signature::operator!=(const std::string& other)
{
    auto repr_str = std::string(m_Representation.begin(), m_Representation.end());
    return repr_str != other;
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
bool Signature::operator!=(const uint32_t other)
{
    return m_Value != other;
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::string Signature::string()
{
    return std::string(m_Representation.begin(), m_Representation.end());
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
Signature Signature::read(File& document)
{
    auto signatureArray = ReadBinaryArray<char>(document, 4u);
    auto signature = std::string(signatureArray.begin(), signatureArray.end());
    return Signature(signature);
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void Signature::write(File& document)
{
    WriteBinaryData<uint32_t>(document, m_Value);
}

PSAPI_NAMESPACE_END