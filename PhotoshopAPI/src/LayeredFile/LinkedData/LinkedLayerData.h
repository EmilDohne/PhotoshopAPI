#pragma once


#include "Macros.h"

#include "Core/Render/Render.h"
#include "Core/Struct/ImageChannel.h"

#include "Util/StringUtil.h"


#include <vector>
#include <unordered_map>
#include <string>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/filesystem.h>


PSAPI_NAMESPACE_BEGIN


enum class LinkedLayerType
{
	Data,
	External
};


template <typename T>
struct LinkedLayerData
{
	/// Alias for our storage data type
	using storage_type = std::unordered_map<Enum::ChannelIDInfo, std::unique_ptr<ImageChannel>, Enum::ChannelIDInfoHasher>;
	using data_type = std::unordered_map<Enum::ChannelIDInfo, std::vector<T>, Enum::ChannelIDInfoHasher>;

	std::string m_Filename;
	std::string m_Hash;
	LinkedLayerType m_Type;

	/// initialize a linked layer from a filepath, parsing the file 
	LinkedLayerData() = default;

	LinkedLayerData(std::filesystem::path filepath, std::string hash)
	{
		if (!filepath.has_filename())
		{
			PSAPI_LOG_ERROR("LinkedLayer", "Unable to construct linked layer without filename in path, got path '%s'", filepath.string().c_str());
		}

		m_Filename = filepath.filename();
		m_Hash = hash;
		File data(filepath);
		m_RawData = ReadBinaryArray<uint8_t>(data, data.getSize());

		// Create and read the file, if the input supports it we read it from the buffer we just read rather than from 
		// disk again.
		auto _in = OIIO::ImageInput::create(filepath);
		if (_in && static_cast<bool>(_in->supports("ioproxy")))
		{
			OIIO::Filesystem::IOMemReader memreader(m_RawData.data(), m_RawData.size());
			auto oiio_in = OIIO::ImageInput::open(filepath.string(), nullptr, &memreader);
			parseOpenImageIoInput(std::move(oiio_in));
		}
		else
		{
			PSAPI_LOG_DEBUG("LinkedLayer", "Unable to construct file '%s' from memory as OpenImageIO doesn't support it. Falling back to reading the file again",
				filepath.string().c_str());
			auto oiio_in = OIIO::ImageInput::open(filepath);
			parseOpenImageIoInput(std::move(oiio_in));
		}
	}

	/// Get a view over the raw file data associated with this linked layer 
	const std::span<const uint8_t> getRawData() const
	{
		return std::span<const uint8_t>(m_RawData.begin(), m_RawData.end());
	}

	const storage_type& getImageData() const
	{
		return m_ImageData;
	}

	storage_type extractImageData()
	{
		return std::move(m_ImageData);
	}

private:
	/// Store the image data as a per-channel map to be used later using a custom hash function
	storage_type m_ImageData;

	/// Raw file data
	std::vector<uint8_t> m_RawData;

	/// Parse the imageinput from the given filepath into our m_ImageData populating it
	/// \param input The imageinput to read from, either 
	void parseOpenImageIoInput(std::unique_ptr<OIIO::ImageInput> input, std::string filepath)
	{
		if (!input)
		{
			PSAPI_LOG_ERROR("LinkedLayer", "Unable to construct LinkedLayer from filepath '%s', error:",
				filepath.c_str(),
				OIIO::geterror().c_str());
		}
		const OIIO::ImageSpec& spec = input->spec();
		const auto& channelnames = spec.channelnames;
		int alpha_channel = spec.alpha_channel;
		// Get the image data as OIIO type from our T template param
		constexpr auto type_desc = Render::get_type_desc<T>();

		/// Initialize our pixels, we reuse this buffer as this data gets discarded at the end
		std::vector<T> pixels(spec.width * spec.height);

		// TODO: add support for non-rgb image data
		std::array<Enum::ChannelIDInfo, 4> channelIDs = {
			Enum::toChannelIDInfo(Enum::ChannelID::Red),
			Enum::toChannelIDInfo(Enum::ChannelID::Green),
			Enum::toChannelIDInfo(Enum::ChannelID::Blue),
			Enum::toChannelIDInfo(Enum::ChannelID::Alpha)
		};

		/// Extract the image data and store it in our m_ImageData
		std::for_each(std::execution::par_unseq, channelnames.begin(), channelnames.end(), [&](std::string name)
			{
				/// Extract all indices from 0-2 as these will represent our RGB channels,
				/// we handle alpha separately
				int idx = spec.channelindex(name);
				if (idx != alpha_channel && idx >= 0 && idx <= 2)
				{
					input->read_image(0, 0, idx, ++idx, type_desc, pixels.data());
					auto channel = std::make_unique<ImageChannel>(Enum::Compression::ZipPrediction, pixels, channelIDs[idx], spec.width, spec.height, 0, 0);
					m_ImageData[channelIDs[idx]] = std::move(channel);
				}
				else if (idx == alpha_channel)
				{
					input->read_image(0, 0, idx, ++idx, type_desc, pixels.data());
					auto channel = std::make_unique<ImageChannel>(Enum::Compression::ZipPrediction, pixels, channelIDs[3], spec.width, spec.height, 0, 0);
					m_ImageData[channelIDs[idx]] = std::move(channel);
				}
				else
				{
					PSAPI_LOG_WARNING("LinkedLayer", "Skipping channel { %d : '%s' } in file '%s' as it is not part of our default channels we currently support.",
						idx, name.c_str(), filepath.c_str());
				}
			});
	}
};


template <typename T>
struct LinkedLayers
{
	std::unordered_map<std::string, LinkedLayerData<T>> m_LinkedLayerData;

	/// Insert a linked layer from the given filepath returning a reference to it. This will read the file
	LinkedLayerData<T>& insert(std::filesystem::path& filePath, std::string hash, LinkedLayerType type = LinkedLayerType::Data)
	{
		m_LinkedLayerData[hash] = LinkedLayerData<T>(filePath, hash);
		return m_LinkedLayerData[hash];
	}

	LinkedLayers() = default;
	LinkedLayers(const AdditionalLayerInfo& globalLayerInfo);
};


PSAPI_NAMESPACE_END