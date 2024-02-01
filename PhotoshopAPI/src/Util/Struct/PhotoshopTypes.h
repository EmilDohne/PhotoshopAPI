#pragma once

#include "Macros.h"
#include "Logger.h"

#include <limits>
#include <tuple>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

PSAPI_NAMESPACE_BEGIN

/// A 4-byte fixed float variable with 2-bytes for the number and 2-bytes for the fractional component.
/// This will lose precision when converting to and back from it!
struct FixedFloat4
{
	/// Initialize a FixedFloat4 from a floating point number. This will incur some precision loss
	inline FixedFloat4(const float number)
	{
		if (number > (std::numeric_limits<uint16_t>::max)()) [[unlikely]]
		{
			PSAPI_LOG_ERROR("FixedFloat4", "Input number cannot exceed 65536, got %f", number);
		}
		// This will be truncated which is correct
		m_Number = static_cast<uint16_t>(number);

		// Calculate the fractional component
		float remainder = number - m_Number;
		remainder *= (std::numeric_limits<uint16_t>::max)();
		m_Fraction = static_cast<uint16_t>(remainder);
	}

	/// Initialize a FixedFloat4 as it would be read from disk
	inline FixedFloat4(const uint16_t number, const uint16_t fraction) noexcept
	{
		m_Number = number;
		m_Fraction = fraction;
	}

	/// Initialize a default FixedFloat4
	inline FixedFloat4() : m_Number(0u), m_Fraction(0u) {};

	/// Get the FixedFloat4 as a floating point number which we can use
	inline float getFloat() const noexcept
	{
		float output = static_cast<float>(m_Number);
		output += (static_cast<float>(m_Fraction) / static_cast<float>((std::numeric_limits<uint16_t>::max)()));
		return output;
	}

	/// Get the Fixed Float4 as two uint16_t ready to be written out to disk
	inline std::tuple<uint16_t, uint16_t> getNumbers()
	{
		return std::make_tuple(m_Number, m_Fraction);
	}

	/// Override the *= operator as we need to multiply the FixedFloat4 sometimes
	/// e.g. when converting from DPCM -> DPI. To achieve this we convert to and 
	/// from float
	inline FixedFloat4 operator*=(float other)
	{
		float floatNum = this->getFloat();
		floatNum *= other;

		if (floatNum > (std::numeric_limits<uint16_t>::max)()) [[unlikely]]
		{
			PSAPI_LOG_ERROR("FixedFloat4", "Input number cannot exceed 65536, got %f", floatNum);
		}
		// This will be truncated which is correct
		m_Number = static_cast<uint16_t>(floatNum);

		// Calculate the fractional component
		float remainder = floatNum - m_Number;
		remainder *= (std::numeric_limits<uint16_t>::max)();
		m_Fraction = static_cast<uint16_t>(remainder);
	}

private:
	/// Stores the whole number
	uint16_t m_Number;
	/// Stores the fraction by dividing m_Fraction / 65536
	uint16_t m_Fraction;
};


PSAPI_NAMESPACE_END