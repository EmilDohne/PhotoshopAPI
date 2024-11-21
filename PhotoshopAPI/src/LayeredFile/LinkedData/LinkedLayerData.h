#pragma once


#include "Macros.h"

#include "Core/Render/Render.h"
#include "Core/Render/ImageBuffer.h"
#include "Core/Struct/ImageChannel.h"

#include "Util/StringUtil.h"


#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebufalgo.h>
#include <OpenImageIO/filesystem.h>


PSAPI_NAMESPACE_BEGIN


enum class LinkedLayerType
{
	data,
	external
};


template <typename T>
struct LinkedLayerData
{
	/// Alias for our storage data type
	using storage_type = std::unordered_map<Enum::ChannelIDInfo, std::unique_ptr<ImageChannel>, Enum::ChannelIDInfoHasher>;
	using data_type = std::unordered_map<Enum::ChannelIDInfo, std::vector<T>, Enum::ChannelIDInfoHasher>;

	/// initialize a linked layer from a filepath, parsing the file 
	LinkedLayerData() = default;

	// Explicitly delete copy ctor as storage_type is not copyable
	LinkedLayerData(const LinkedLayerData<T>&) = delete;

	LinkedLayerData(std::filesystem::path filepath, std::string hash)
	{
		if (!std::filesystem::exists(filepath))
		{
			PSAPI_LOG_ERROR("LinkedLayer", "Unable to construct linked layer with invalid path, got path '%s'", filepath.string().c_str());
		}
		if (!filepath.has_filename())
		{
			PSAPI_LOG_ERROR("LinkedLayer", "Unable to construct linked layer without filename in path, got path '%s'", filepath.string().c_str());
		}

		m_FilePath = filepath;
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
			parse_oiio_input(std::move(oiio_in));
		}
		else
		{
			PSAPI_LOG_DEBUG("LinkedLayer", "Unable to construct file '%s' from memory as OpenImageIO doesn't support it. Falling back to reading the file again",
				filepath.string().c_str());
			auto oiio_in = OIIO::ImageInput::open(filepath);
			parse_oiio_input(std::move(oiio_in));
		}
	}

	/// Get a view over the raw file data associated with this linked layer 
	const std::span<const uint8_t> raw_data() const
	{
		return std::span<const uint8_t>(m_RawData.begin(), m_RawData.end());
	}

	/// Get a const view over the image data.
	const storage_type& image_data() const
	{
		return m_ImageData;
	}


	data_type get_image_data() const
	{
		PSAPI_PROFILE_FUNCTION();
		data_type out;

		size_t threads = std::thread::hardware_concurrency() / m_ImageData.size();
		threads = std::max(threads, static_cast<size_t>(1));
		std::mutex mutex;

		std::for_each(std::execution::par_unseq , m_ImageData.begin(), m_ImageData.end(), [&](const auto& pair)
			{
				const auto& key = pair.first;
				const auto& channel = pair.second;
				std::vector<T> data = channel->getData(threads);
				{
					std::lock_guard<std::mutex> lock(mutex);
					out[key] = std::move(data);
				}
			});

		return out;
	}

	/// Return the image data held by this struct as a rescaled version of itself
	///
	/// This interpolates the data to the given `width` and `height` where those do not necessarily
	/// need to match the aspect ratio of the original image (but will be stretched if they do not match).
	/// We provide several interpolation methods but not all that photoshop recognizes.
	/// 
	/// \param width The horizontal resolution to resample to
	/// \param height The vertical resolution to resample to
	/// \param interpolation The interpolation algorithm to use. This may be `nearest_neighbour`, 
	///						 `bilinear` or `bicubic` (default). Each of these has their advantages and disadvantages.
	///						 `nearest_neighbour` for example is the fastest but will create blocky results while `bicubic`
	///						 will create the best result but may be slowest.
	/// 
	/// \returns The resampled image data
	data_type get_image_data(size_t width, size_t height, Render::Interpolation interpolation = Render::Interpolation::bicubic)
	{
		data_type data = get_image_data();
		data_type resampled_data{};

		for (auto& [key, channel] : data)
		{
			Render::ImageBuffer<T> buffer(channel, m_Width, m_Height);
			// Interpolate the output channel
			if (interpolation == Render::Interpolation::nearest_neighbour)
			{
				resampled_data[key] = buffer.rescale_nearest_neighbour(width, height);
			}
			else if (interpolation == Render::Interpolation::bilinear)
			{
				resampled_data[key] = buffer.rescale_bilinear(width, height);
			}
			else if (interpolation == Render::Interpolation::bicubic)
			{
				constexpr T min = std::numeric_limits<T>::lowest();
				constexpr T max = std::numeric_limits<T>::max();
				resampled_data[key] = buffer.rescale_bicubic(width, height, min, max);
			}
		}

		return resampled_data;
	}

	/// Get the width and height of the image data stored on the linked layer.
	std::array<size_t, 2> size() const noexcept { return { m_Width, m_Height }; }

	/// Return the width of the image data held by this struct
	size_t width() const noexcept { return m_Width; }

	/// Return the height of the image data held by this struct
	size_t height() const noexcept { return m_Height; }

	/// Get the full path to the file stored on the LinkedLayerData.
	std::filesystem::path path() const noexcept { return m_FilePath; }

	/// Get the hash associated with the LinkedLayerData
	std::string hash() const noexcept { return m_Hash; }

	/// Get the filename associated with the LinkedLayerData
	std::string filename() const noexcept { return m_Filename; }

	/// Get the type of linkage this file has, whether that may be data (on the file itself) or external
	LinkedLayerType type() const noexcept { return m_Type; }

	/// Set the type of linkage this file has, whether that may be data (on the file itself) or external.
	///
	/// As we only apply this on write this can be changed as many times as wanted.
	void type(LinkedLayerType type_) noexcept{ m_Type = type_; }


private:
	/// Store the image data as a per-channel map to be used later using a custom hash function
	storage_type m_ImageData;

	/// Raw file data
	std::vector<uint8_t> m_RawData;

	size_t m_Width = 1;
	size_t m_Height = 1;

	/// The whole filepath, this doesnt get stored on the photoshop file but 
	/// we use it for better identification
	std::filesystem::path m_FilePath;

	std::string m_Filename;
	std::string m_Hash;
	LinkedLayerType m_Type;

	/// Parse the imageinput from the given filepath into our m_ImageData populating it
	/// \param input The imageinput to read from, either 
	void parse_oiio_input(std::unique_ptr<OIIO::ImageInput> input, std::string filepath)
	{
		if (!input)
		{
			PSAPI_LOG_ERROR("LinkedLayer", "Unable to construct LinkedLayer from filepath '%s', error:",
				filepath.c_str(),
				OIIO::geterror().c_str());
		}
		const OIIO::ImageSpec& spec = input->spec();
		m_Width = spec.width();
		m_Height = spec.height();

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
					input->read_image(0, 0, idx, idx + 1, type_desc, pixels.data());
					auto channel = std::make_unique<ImageChannel>(Enum::Compression::ZipPrediction, pixels, channelIDs[idx], spec.width, spec.height, 0, 0);
					m_ImageData[channelIDs[idx]] = std::move(channel);
				}
				else if (idx == alpha_channel)
				{
					input->read_image(0, 0, idx, idx + 1, type_desc, pixels.data());
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


/// Global storage for linked layers. This exists for deduplication purposes to ensure we can create
/// multiple smart objects with the same image data without having to worry about performance impact.
/// The imaged stored under the `LinkedLayerData` are all in full resolution and get warped/rescaled
/// only on the actual `SmartObjectLayer` itself. For all intents and purposes users won't have to 
/// directly work with this struct as it is all abstracted away when using a smart object.
template <typename T>
struct LinkedLayers
{


	LinkedLayers() = default;
	LinkedLayers(const AdditionalLayerInfo& globalLayerInfo);


	/// Retrieve the LinkedLayer data at the given hash. Throws an error if the hash doesnt exist
	std::shared_ptr<LinkedLayerData<T>> at(std::string hash)
	{
		if (m_LinkedLayerData.contains(hash))
		{
			return m_LinkedLayerData.at(hash);
		}
		PSAPI_LOG_ERROR("LinkedLayers", "Unknown linked layer hash '%s' encountered", hash.c_str());
	}

	/// Retrieve the LinkedLayer data at the given hash. Throws an error if the hash doesnt exist
	const std::shared_ptr<LinkedLayerData<T>> at(std::string hash) const
	{
		if (m_LinkedLayerData.contains(hash))
		{
			return m_LinkedLayerData.at(hash);
		}
		PSAPI_LOG_ERROR("LinkedLayers", "Unknown linked layer hash '%s' encountered", hash.c_str());
	}


	bool contains(std::string hash)
	{
		return m_LinkedLayerData.contains(hash);
	}


	bool contains_path(std::filesystem::path path)
	{
		for (const auto& [_hash, item] : m_LinkedLayerData)
		{
			if (item.path() == path)
			{
				return true;
			}
		}
		return false;
	}

	/// Insert a linked layer from the given filepath returning a reference to it. This will read the file and either store or
	/// link the data. To be used when a user replaces the image data inside a smart object or creates a new smart object layer.
	/// If the data already exists we return a reference to an existing `LinkedLayerData` struct 
	/// 
	/// \param filePath The path of the file to link the data with, if the layer type is `LinkedLayerType::Data` this file does
	///					 not have to be present on the machine opening it
	/// \param hash		A unique hash for the `LinkedLayerData`, this hash has to be identical to the one stored on the `SmartObjectLayer`
	/// \param type		The type of LinkedLayer to create, a photoshop file may have differing values
	/// 
	/// \returns A reference to the LinkedLayerData we just created or an existing one
	std::shared_ptr<LinkedLayerData<T>>& insert(const std::filesystem::path& filePath, const std::string& hash, LinkedLayerType type = LinkedLayerType::Data)
	{
		if (m_LinkedLayerData.contains(hash))
		{
			// Increment the reference count for the given hash
			++m_ReferenceCount[hash];
		}
		else
		{
			m_LinkedLayerData[hash] = std::make_shared<LinkedLayerData<T>>(filePath, hash);
			// Increment the reference count for the given hash
			++m_ReferenceCount[hash];
		}
		// Return a reference to the inserted/updated linked layer data
		return m_LinkedLayerData[hash];
	}

	/// Insert a linked layer from the given filepath returning a reference to it. This will read the file and either store or
	/// link the data. To be used when a user replaces the image data inside a smart object or creates a new smart object layer.
	/// If the data already exists we return a reference to an existing `LinkedLayerData` struct 
	/// 
	/// \param filePath The path of the file to link the data with, if the layer type is `LinkedLayerType::Data` this file does
	///					 not have to be present on the machine opening it
	/// \param type		The type of LinkedLayer to create, a photoshop file may have differing values
	/// 
	/// \returns A reference to the LinkedLayerData we just created or an existing one
	std::shared_ptr<LinkedLayerData<T>>& insert(const std::filesystem::path& filePath, LinkedLayerType type = LinkedLayerType::Data)
	{
		// Try and find the reference based on the filepath first, if this fails we insert a new one
		std::string hash;
		for (const auto& [_hash, item] : m_LinkedLayerData)
		{
			if (item.path() == filePath)
			{
				if (item.m_Type != type)
				{
					PSAPI_LOG_WARNING("LinkedLayers",
						"Found existing LinkedLayerData for filepath '%s' but with a differing type, keeping the current type and ignoring passed argument",
						filePath.string().c_str());
				}
				hash = _hash;
			}
		}
		if (hash.empty())
		{
			hash = generateUUID();
		}
		return insert(filePath, hash, type);
	}

	/// Remove a reference for the given hash, to be called if a smart object layer changes their content or 
	/// gets removed from the file entirely. If the given string hash is not valid we raise an error.
	/// 
	/// \param hash The hash for the layer we are trying to remove the reference for
	void remove_reference(std::string hash)
	{
		if (!m_ReferenceCount.contains(hash))
		{
			PSAPI_LOG_ERROR("LinkedLayers", "remove_reference called with invalid hash: '%s'", hash.c_str());
		}
		// Decrement the ref count and erase the items if we no longer hold any references to it
		--m_ReferenceCount[hash];
		if (m_ReferenceCount[hash] == 0)
		{
			m_LinkedLayerData.erase(hash);
			m_ReferenceCount.erase(hash);
		}
	}



private: 
	/// Reference count for every layer, once this reaches zero we remove
	/// the layer from m_LinkedLayerData
	std::unordered_map<std::string, size_t> m_ReferenceCount;
	std::unordered_map<std::string, std::shared_ptr<LinkedLayerData<T>>> m_LinkedLayerData;

};


PSAPI_NAMESPACE_END