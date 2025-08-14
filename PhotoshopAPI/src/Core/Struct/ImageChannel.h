#pragma once

#include "Macros.h"
#include "Util/Enum.h"
#include "Util/Profiling/Perf/Instrumentor.h"
#include "PhotoshopFile/FileHeader.h"

#include <compressed/channel.h>

#include <vector>
#include <limits>
#include <cassert>
#include <span>


PSAPI_NAMESPACE_BEGIN


using compressed_channel_variant = std::variant<
	std::monostate, 
	compressed::channel<bpp8_t>, 
	compressed::channel<bpp16_t>, 
	compressed::channel<bpp32_t>
>;

template <typename T>
concept is_bitdepth = std::is_same_v<T, bpp8_t> || std::is_same_v<T, bpp16_t> || std::is_same_v<T, bpp32_t>;


struct channel_wrapper
{

	template <typename T>
		requires is_bitdepth<T>
	channel_wrapper(
		compressed::channel<T> data,
		Enum::Compression compression,
		Enum::ChannelIDInfo channel_id,
		float x_coord,
		float y_coord
	) 
	{
		if (data.width() > std::numeric_limits<uint32_t>::max())
		{
			PSAPI_LOG_ERROR(
				"Channel",
				"Channel initialized with a width greater than what a uint32_t could store. Received %zu", data.width());
		}
		if (data.height() > std::numeric_limits<uint32_t>::max())
		{
			PSAPI_LOG_ERROR(
				"Channel",
				"Channel initialized with a height greater than what a uint32_t could store. Received %zu", data.height());
		}

		m_Channel = std::move(data);
		m_PhotoshopCompression = compression;
		m_ChannelID = channel_id;
		m_XCoord = x_coord;
		m_YCoord = y_coord;
	}

	template <typename T>
		requires is_bitdepth<T>
	channel_wrapper(
		Enum::Compression compression,
		std::vector<T>& data,
		Enum::ChannelIDInfo channel_id,
		uint32_t width,
		uint32_t height,
		float x_coord,
		float y_coord
	)
	{
		m_Channel = compressed::channel<T>(
			std::span<const T>(data.begin(), data.end()),
			static_cast<size_t>(width),
			static_cast<size_t>(height)
		);
		m_PhotoshopCompression = compression;
		m_ChannelID = channel_id;
		m_XCoord = x_coord;
		m_YCoord = y_coord;
	}

	template <typename T>
		requires is_bitdepth<T>
	channel_wrapper(
		Enum::Compression compression,
		const std::span<const T> data,
		Enum::ChannelIDInfo channel_id,
		uint32_t width,
		uint32_t height,
		float x_coord,
		float y_coord
	)
	{
		m_Channel = compressed::channel<T>(
			data,
			static_cast<size_t>(width),
			static_cast<size_t>(height)
		);
		m_PhotoshopCompression = compression;
		m_ChannelID = channel_id;
		m_XCoord = x_coord;
		m_YCoord = y_coord;
	}

	Enum::Compression compression_codec() const noexcept
	{
		return m_PhotoshopCompression;
	}
	void compression_codec(Enum::Compression compcode)
	{
		m_PhotoshopCompression = compcode;
	}

	Enum::ChannelIDInfo channel_id_info() const noexcept
	{
		return m_ChannelID;
	}
	void channel_id_info(Enum::ChannelIDInfo id_info)
	{
		m_ChannelID = id_info;
	}

	/// Get the width of the uncompressed ImageChannel
	uint32_t width() const 
	{ 
		return std::visit([](const auto& var) -> uint32_t
		{
			if constexpr (std::is_same_v<std::remove_cvref_t<decltype(var)>, std::monostate>)
			{
				throw std::runtime_error(
					"compressed channel is in an empty state, unable to access properties on it. Please ensure it has"
					" not already been extracted."
				);
			}
			else
			{
				auto width = var.width();
				assert(width <= std::numeric_limits<uint32_t>::max());
				return static_cast<uint32_t>(width);
			}
		}, m_Channel); 
	};
	// /// Get the height of the uncompressed ImageChannel
	uint32_t height() const 
	{ 
		return std::visit([](const auto& var) -> uint32_t
		{
			if constexpr (std::is_same_v<std::remove_cvref_t<decltype(var)>, std::monostate>)
			{
				throw std::runtime_error(
					"compressed channel is in an empty state, unable to access properties on it. Please ensure it has"
					" not already been extracted."
				);
			}
			else
			{
				auto height = var.height();
				assert(height <= std::numeric_limits<uint32_t>::max());
				return static_cast<uint32_t>(height);
			}
		}, m_Channel); 
	};
	/// Get the x-coordinate of the uncompressed ImageChannel
	float center_x() const { return m_XCoord; };
	void center_x(float value) { m_XCoord = value; }
	/// Get the y-coordinate of the uncompressed ImageChannel
	float center_y() const { return m_YCoord; };
	void center_y(float value) { m_YCoord = value; }
	/// Get the total number of chunks held in the ImageChannel
	size_t num_chunks() const 
	{
		return std::visit([](const auto& var) -> size_t
		{
			if constexpr (std::is_same_v<std::remove_cvref_t<decltype(var)>, std::monostate>)
			{
				throw std::runtime_error(
					"compressed channel is in an empty state, unable to access properties on it. Please ensure it has"
					" not already been extracted."
				);
				return 0;
			}
			else
			{
				return var.num_chunks();
			}
		}, m_Channel);
	};

	size_t byte_size() const
	{
		return std::visit([](const auto& var) -> size_t
		{
			if constexpr (std::is_same_v<std::remove_cvref_t<decltype(var)>, std::monostate>)
			{
				throw std::runtime_error(
					"compressed channel is in an empty state, unable to access properties on it. Please ensure it has"
					" not already been extracted."
				);
			}
			else
			{
				using channel_t = std::remove_cvref_t<decltype(var)>;
				return var.uncompressed_size() * sizeof(typename channel_t::value_type);
			}
		}, m_Channel);
	}

	size_t element_size() const
	{
		return std::visit([](const auto& var) -> size_t
		{
			if constexpr (std::is_same_v<std::remove_cvref_t<decltype(var)>, std::monostate>)
			{
				throw std::runtime_error(
					"compressed channel is in an empty state, unable to access properties on it. Please ensure it has"
					" not already been extracted."
				);
			}
			else
			{
				return var.uncompressed_size();
			}
		}, m_Channel);
	}
	
	/// \brief Extract the channel permanently from the struct, invalidating it
	/// \tparam T the type of the channel
	/// 
	/// \throws std::bad_variant_access if the requested type is not that of the channel
	template <typename T>
		requires is_bitdepth<T>
	compressed::channel<T> extract_channel()
	{
		auto channel = std::move(std::get<compressed::channel<T>>(m_Channel));
		m_Channel.emplace<std::monostate>(); // reset to empty state
		return channel;
	}

	template <typename T>
		requires is_bitdepth<T>
	std::vector<T> get_data() const
	{
		const auto& channel = std::get<compressed::channel<T>>(m_Channel);
		return channel.get_decompressed();
	}

	template <typename T>
		requires is_bitdepth<T>
	std::vector<T> extract_data()
	{
		// Extract the channel
		auto channel = this->extract_channel<T>();
		return channel.get_decompressed();
		// Now let it exit scope and get destructed, freeing the underlying compressed data
	}

	template <typename T>
		requires is_bitdepth<T>
	void get_data(std::span<T> buffer) const
	{
		const auto& channel = std::get<compressed::channel<T>>(m_Channel);
		if (buffer.size() != channel.uncompressed_size())
		{
			throw std::invalid_argument(
				std::format(
					"Unable to retrieve image data from compressed channel as input size does not match output size."
					" Expected exactly {} elements in the passed buffer but instead received {} elements", 
					channel.uncompressed_size(), buffer.size()
				)
			);
		}

		// Extract the buffer chunk by chunk, reduces the number of total memory allocations necessary.
		size_t offset = 0;
		for (size_t chunk_idx = 0; chunk_idx < channel.num_chunks(); ++chunk_idx)
		{
			auto subspan = std::span<T>(buffer.data() + offset, channel.chunk_elems(chunk_idx));

			channel.get_chunk(subspan, chunk_idx);
			
			offset += channel.chunk_elems(chunk_idx);
		}
	}


private:

	/// This does not indicate the compression method of the channel in memory 
	/// but rather the compression method it writes the PhotoshopFile with
	Enum::Compression m_PhotoshopCompression = Enum::Compression::ZipPrediction;
	/// Information about what channel this actually is
	Enum::ChannelIDInfo m_ChannelID = { Enum::ChannelID::Red, 1 };
	/// The underlying compressed channel. May only be uint8_t, uint16_t or float32_t.
	compressed_channel_variant m_Channel = {};

	float m_XCoord = 0.0f;
	float m_YCoord = 0.0f;
};


PSAPI_NAMESPACE_END