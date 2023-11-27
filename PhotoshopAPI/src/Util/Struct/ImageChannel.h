#pragma once

#include "Macros.h"
#include "Enum.h"

#include <vector>
#include <memory>

PSAPI_NAMESPACE_BEGIN


struct BaseImageChannel
{
	Enum::Compression m_Compression = Enum::Compression::Raw;
	Enum::ChannelID m_ChannelID = Enum::ChannelID::Red;

	BaseImageChannel(Enum::Compression compression, const Enum::ChannelID channelID, const uint32_t width, const uint32_t height)
	{
		if (width > 300000u)
		{
			PSAPI_LOG_ERROR("ImageChannel", "Invalid width parsed to image channel. Photoshop channels can be 300,000 pixels wide, got %" PRIu32 " instead",
				width)
		}
		if (height > 300000u)
		{
			PSAPI_LOG_ERROR("ImageChannel", "Invalid height parsed to image channel. Photoshop channels can be 300,000 pixels high, got %" PRIu32 " instead",
				height)
		}

		m_Compression = compression;
		m_Width = width;
		m_Height = height;
		m_ChannelID = channelID;
	}

	virtual ~BaseImageChannel() = default;

protected:
	uint32_t m_Width = 0u;
	uint32_t m_Height = 0u;
};


// A generic Image Channel that could either be part of the Channel Image Data section or Image Data section
// It is entirely valid to have each channel have a different compression method, width and height. We only
// store the image data in here but do not deal with reading or writing it. Ownership of the image data belongs
// to this struct
template <typename T>
struct ImageChannel : public BaseImageChannel
{
	// Take a reference to a decompressed image vector stream and set the according member variables
	ImageChannel(Enum::Compression compression, std::vector<T> imageData, const Enum::ChannelID channelID, const uint32_t width, const uint32_t height) : BaseImageChannel(compression, channelID, width, height)
	{
		m_Data = std::move(imageData);
	};


	std::vector<T>& getData() {
		return m_Data;
	}

private:
	std::vector<T> m_Data;
};


PSAPI_NAMESPACE_END