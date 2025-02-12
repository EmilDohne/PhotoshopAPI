#pragma once

#include "Macros.h"

#include "MaskDataMixin.h"

#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "PhotoshopFile/AdditionalLayerInfo.h"

#include "Core/Struct/ImageChannel.h"
#include "Core/Geometry/BoundingBox.h"

#include <unordered_map>
#include <vector>
#include <span>

#include <fmt/format.h>
#include <fmt/ranges.h>

PSAPI_NAMESPACE_BEGIN


/// \brief A mixin struct for handling read-only image data within layers.
/// 
/// This struct provides a standardized way to manage image data, including
/// storing and retrieving channel data and evaluating
/// image content dynamically. It is designed to be inherited by layer
/// classes that require image data storage and processing.
/// 
/// \tparam T The data type used for pixel values (e.g., uint8_t, uint16_t, float).
template <typename T>
struct ImageDataMixin
{
public:
	/// Type used for a single channel
	using channel_type = std::unique_ptr<ImageChannel>;
	/// Type used for a mapping of channels.
	using image_type = std::unordered_map<Enum::ChannelIDInfo, channel_type, Enum::ChannelIDInfoHasher>;
	/// Type used for data as it is passed back to the user.
	using data_type = std::unordered_map<int, std::vector<T>>;
	/// Type used for a view as it is passed back to the user.
	using view_type = std::unordered_map<int, std::span<T>>;

public:

	/// Get the channel indices held by this layer.
	virtual std::vector<int> channel_indices(bool include_mask) const = 0;

	/// Get the total number of channels held by this layer.
	virtual size_t num_channels(bool include_mask) const = 0;

	/// Convenience function for splitting a mask channel from an image data mapping. This is usually necessary
	/// to ensure the mask is handled separately and stored on e.g. the `MaskMixin<T>`.
	/// 
	/// This will extract the mask channel from `data` and optionally return it (if it exists)
	/// 
	/// \return an optional mask channel if the passed image data contains it.
	static std::optional<channel_type> split_mask(image_type& data)
	{
		auto node = data.extract(MaskMixin<T>::s_mask_index);
		if (!node.empty())
		{
			return std::move(node.mapped());
		}
		return std::nullopt;
	}

	/// Get the underlying storage of the image data held by this Layer. This may not always be up-to-date
	/// in the case of layers that require rendering such as SmartObject layers or Text layers.
	/// 
	/// Usually a user should not have to access image data this way and should instead use `get_image_data()`
	/// or `get_channel()`.
	const image_type& get_storage() const
	{
		return m_ImageData;
	}

	/// Get the image data held by this layer, this includes all channels as well as any masks.
	/// 
	/// Therefore not all channels are guaranteed to be the same. If `has_mask()` is true (or channel -2
	/// if in the data) the mask channel may be any size and does not have to overlap with the layer.
	/// 
	/// The other channels do however have to be the same size
	/// 
	/// \return The evaluated image data including mask.
	data_type get_image_data()
	{
		auto data = evaluate_image_data();
		return std::move(data);
	}

	/// Get the channel held at the given index
	///
	/// This channel will have the dimensions `width()` * `height()` unless you 
	/// are requesting the mask channel -2. This will instead hold the dimensions described by `mask_bbox()`.
	/// Calling `get_channel()` with index -2 is equivalent to calling `get_mask()`
	/// 
	/// Generally this will method will be slightly slower than calling `get_image_data` for multiple channels
	/// as `get_image_data` is parallelized.
	/// 
	/// \return The evaluated channel
	std::vector<T> get_channel(int _id)
	{
		auto data = evaluate_channel(_id);
		return std::move(data);
	}

	/// Get the channel held at the given index
	///
	/// This channel will have the dimensions `width()` * `height()` unless you 
	/// are requesting the mask channel -2. This will instead hold the dimensions described by `mask_bbox()`.
	/// Calling `get_channel()` with index -2 is equivalent to calling `get_mask()`
	/// 
	/// Generally this will method will be slightly slower than calling `get_image_data` for multiple channels
	/// as `get_image_data` is parallelized.
	/// 
	/// \return The evaluated channel
	std::vector<T> get_channel(Enum::ChannelID _id)
	{
		auto data = evaluate_channel(_id);
		return std::move(data);
	}

	/// Get the channel held at the given index
	///
	/// This channel will have the dimensions `width()` * `height()` unless you 
	/// are requesting the mask channel -2. This will instead hold the dimensions described by `mask_bbox()`.
	/// Calling `get_channel()` with index -2 is equivalent to calling `get_mask()`
	/// 
	/// Generally this will method will be slightly slower than calling `get_image_data` for multiple channels
	/// as `get_image_data` is parallelized.
	/// 
	/// \return The evaluated channel
	std::vector<T> get_channel(Enum::ChannelIDInfo _id)
	{
		auto data = evaluate_channel(_id);
		return std::move(data);
	}

	virtual ~ImageDataMixin() = default;

protected:

	/// Store the image data as a per-channel map to be used later using a custom hash function
	image_type m_ImageData;

	/// Evaluates the image data, this may not be exactly the same as what is held by m_ImageData. 
	/// The reason for this abstraction is that e.g. masks are not stored as part of this ImageDataMixin
	/// so a layer must take care to concatenate these channels together. Similarly, a smart object must apply the warp first
	/// before getting the image data. 
	/// 
	/// Implementations must ensure that a mask (if present) is part of the image data returned.
	/// 
	/// Implementations may cache these intermediate results in m_ImageData if they see fit but they they are then
	/// responsible for keeping track of this.
	/// 
	/// Each Layer type that implement ImageDataMixin must provide an implementation for evaluate_image_data().
	/// 
	/// \param document The document the layer is a part of, may be unused during evaluation.
	/// 
	/// \returns The evaluated image data for each channel 
	virtual data_type evaluate_image_data() = 0;


	/// Evaluates a single channel of the image data.
	/// The reason for this abstraction is that e.g. masks are not stored as part of this ImageDataMixin
	/// so a layer must take care to access these channels together. Similarly, a smart object must apply the warp first
	/// before getting the image data. 
	/// 
	/// \param document The document the layer is a part of, may be unused during evaluation.
	/// 
	/// \returns The evaluated image data for each channel 
	virtual std::vector<T> evaluate_channel(std::variant<int, Enum::ChannelID, Enum::ChannelIDInfo> _id) = 0;

	/// Validate the channels held by m_ImageData for the given colormode on whether they include all required channels
	///
	/// RGB for example requires at least r, g, and b channels to be present,
	/// similarly CMYK requires at least c, m, y, k etc.
	/// 
	/// Returns `false` If the channels do not fulfill these requirements
	/// 
	/// \param colormode The colormode to check the channels against
	/// \param no_warn	 Whether to skip emitting detailed warnings on failure to validate.
	/// 
	/// \throws std::runtime_error If the colormode is unsupported.
	bool validate_channels(Enum::ColorMode colormode, bool no_warn = false)
	{
		// Check that vector a is a subset of vector b
		auto is_subset = [](std::vector<Enum::ChannelIDInfo> a, std::vector<Enum::ChannelIDInfo> b) -> bool
			{
				return std::all_of(a.begin(), a.end(), [&b](Enum::ChannelIDInfo a_element)
					{
						return std::find(b.begin(), b.end(), a_element) != b.end();
					});
			};

		// Warn for all the missing required channels
		auto warn_missing_channels = [](std::vector<Enum::ChannelIDInfo> _expected, std::vector<Enum::ChannelIDInfo> _held, std::string colormode) -> std::vector<std::string>
			{
				std::vector<std::string> missing_channels;
				for (const auto& expected_channel : _expected)
				{
					if (std::find(_held.begin(), _held.end(), expected_channel) == _held.end())
					{
						missing_channels.push_back(Enum::channelIDToString(expected_channel.id));
					}
				}
				std::string warn_message = fmt::format("Warning: <{}> The following expected channels are missing in the image data:\n{}", colormode, fmt::join(missing_channels, ", "));
				PSAPI_LOG_WARNING("ImageData", "%s", warn_message.c_str());
				return missing_channels;
			};

		// Since a mask channel is never a requirement we can acces m_ImageData directly and don't need to 
		// go fetching the mask
		auto held_channels = this->key_vector_from_map(m_ImageData);

		if (colormode == Enum::ColorMode::RGB)
		{
			constexpr Enum::ChannelIDInfo channel_r = {Enum::ChannelID::Red, 0 };
			constexpr Enum::ChannelIDInfo channel_g = {Enum::ChannelID::Green, 1 };
			constexpr Enum::ChannelIDInfo channel_b = {Enum::ChannelID::Blue, 2 };
			std::vector<Enum::ChannelIDInfo> expected = { channel_r, channel_g, channel_b };

			if (!is_subset(expected, held_channels))
			{
				if (!no_warn)
				{
					warn_missing_channels(expected, held_channels, "rgb");
				}
				return false;
			}
		}
		else if (colormode == Enum::ColorMode::CMYK)
		{
			constexpr Enum::ChannelIDInfo channel_c = {Enum::ChannelID::Cyan, 0 };
			constexpr Enum::ChannelIDInfo channel_m = {Enum::ChannelID::Magenta, 1 };
			constexpr Enum::ChannelIDInfo channel_y = {Enum::ChannelID::Yellow, 2 };
			constexpr Enum::ChannelIDInfo channel_k = {Enum::ChannelID::Black, 3 };
			std::vector<Enum::ChannelIDInfo> expected = { channel_c, channel_m, channel_y, channel_k };

			if (!is_subset(expected, held_channels))
			{
				if (!no_warn)
				{
					warn_missing_channels(expected, held_channels, "cmyk");
				}
				return false;
			}
		}
		else if (colormode == Enum::ColorMode::Grayscale)
		{
			constexpr Enum::ChannelIDInfo channel_g = { Enum::ChannelID::Gray, 0 };
			std::vector<Enum::ChannelIDInfo> expected = { channel_g };

			if (!is_subset(expected, held_channels))
			{
				if (!no_warn)
				{
					warn_missing_channels(expected, held_channels, "grayscale");
				}
				return false;
			}
		}
		else
		{
			PSAPI_LOG_ERROR("ImageData", "The PhotoshopAPI currently only supports RGB, CMYK and Grayscale color modes");
		}
		return true;
	}


	/// Validate whether the channels held by m_ImageData all are of the same size.
	///
	/// This is a requirement imposed by photoshop itself for all channels except the mask channel!
	/// 
	/// \param no_warn	 Whether to skip emitting detailed warnings on failure to validate.
	/// 
	/// /// \return `true` if all channels have the same size, `false` otherwise.
	bool validate_channel_sizes(bool no_warn = false) const
	{
		std::unordered_map<Enum::ChannelIDInfo, size_t, Enum::ChannelIDInfoHasher> channel_sizes{};
		for (const auto& [key, channel_ptr] : m_ImageData)
		{
			channel_sizes[key] = channel_ptr->m_OrigByteSize / sizeof(T);
		}

		// If there is only one channel, we can skip size comparison
		if (channel_sizes.size() <= 1)
		{
			return true;
		}

		// Get the first channel size for comparison
		size_t first_size = channel_sizes.begin()->second;
		bool sizes_match = true;
		std::vector<std::string> mismatch_channels;

		// Check if all channels have the same size
		for (const auto& [key, size] : channel_sizes)
		{
			if (size != first_size)
			{
				sizes_match = false;
				mismatch_channels.push_back(Enum::channelIDToString(key.id));
			}
		}

		// If there's a mismatch, emit warning if not skipping
		if (!sizes_match && !no_warn)
		{
			std::string warn_message = fmt::format(
				"Warning: The following channels have mismatched sizes in the image data:\n"
				"{}",
				fmt::join(mismatch_channels, ", ")
			);
			PSAPI_LOG_WARNING("ImageData", "%s", warn_message.c_str());
		}

		return sizes_match;

	}

	/// Utility function to allocate image data in parallel for the all the given keys initializing each to data_size.
	/// 
	/// \param keys		 The keys to insert into the output map
	/// \param data_size The size of the data to allocate per-channel. This must be the same across all channels
	/// 
	/// \returns The allocated image data as an unordered_map.
	static data_type parallel_alloc_image_data(const std::vector<int>& keys, size_t data_size)
	{
		data_type out_map{};
		out_map.reserve(keys.size());

		std::mutex img_data_mutex;

		std::for_each(std::execution::par_unseq, keys.begin(), keys.end(), [&](auto key) 
			{
				auto vec = std::vector<T>(data_size);
				std::lock_guard<std::mutex> lock(img_data_mutex);
				out_map[key] = std::move(vec);
			});

		return std::move(out_map);
	}

	/// Utility function to extract the keys of a map as a std::vector.
	///
	/// This function will copy all the keys.
	template <typename KeyType, typename ValType, class _Hasher = std::hash<KeyType>>
	static std::vector<KeyType> key_vector_from_map(const std::unordered_map<KeyType, ValType, _Hasher>& map) noexcept
	{
		std::vector<KeyType> keys;
		keys.reserve(map.size());

		for (const auto& [key, _val] : map) 
		{
			keys.push_back(key);
		}

		return keys;
	}

	/// Utility function to extract the keys and values of a map as a std::vector.
	///
	/// This function will copy all the keys and values, they must therefore be copy constructible 
	template <typename KeyType, typename ValType, class _Hasher = std::hash<KeyType>>
	static std::tuple<std::vector<KeyType>, std::vector<ValType>> vectors_from_map(const std::unordered_map<KeyType, ValType, _Hasher>& map) noexcept
	{
		std::vector<KeyType> keys;
		keys.reserve(map.size());
		std::vector<ValType> vals;
		vals.reserve(map.size());

		for (const auto& [key, val] : map) 
		{
			keys.push_back(key);
			vals.push_back(val);
		}

		return std::make_tuple(keys, vals);
	}
	
	/// Utility function to generate a ChannelIDInfo from a variant of possible args for the given colormode.
	static Enum::ChannelIDInfo idinfo_from_variant(std::variant<int, Enum::ChannelID, Enum::ChannelIDInfo> _id, Enum::ColorMode colormode)
	{
		Enum::ChannelIDInfo idinfo{};
		if (std::holds_alternative<int>(_id))
		{
			idinfo = Enum::toChannelIDInfo(std::get<int>(_id), colormode);
		}
		else if (std::holds_alternative<Enum::ChannelID>(_id))
		{
			idinfo = Enum::toChannelIDInfo(std::get<Enum::ChannelID>(_id), colormode);
		}
		else
		{
			idinfo = std::get<Enum::ChannelIDInfo>(_id);
		}
		return idinfo;
	}
};


/// \brief A mixin struct for handling writable image data within layers.
///
/// This struct extends the functionality of `ImageDataMixin` by providing methods
/// for setting image and channel data, enabling modification of the image data
/// within a layer. It includes pure virtual methods for setting image data and
/// channels that must be implemented in the derived class. Additionally, it offers
/// an implementation for setting image data across multiple channels in parallel.
/// 
/// The struct is designed to be inherited by classes that require the ability to
/// modify image data, such as when working with layer-based image editing.
///
/// \tparam T The data type used for pixel values (e.g., uint8_t, uint16_t, float).
template<typename T>
struct WritableImageDataMixin : public ImageDataMixin<T>
{
public:

	/// Type alias for the base class.
	using Base = ImageDataMixin<T>;

	/// Type alias for the channel type.
	using typename Base::channel_type;

	/// Type alias for the image type.
	using typename Base::image_type;

	/// Type alias for the data type.
	using typename Base::data_type;

	/// Type alias for the view type.
	using typename Base::view_type;

public:

	
	/// \brief Sets the underlying storage of the image data held by this Layer.
	/// 
	/// Usually a user should not have to access image data this way and should instead use `set_image_data()`
	/// or `set_channel()`.
	void set_storage(image_type data)
	{
		ImageDataMixin<T>::m_ImageData = std::move(data);
	};

	/// \brief Sets the image data.
	///
	/// \param data The data to be set.
	virtual void set_image_data(const data_type& data) = 0;

	/// \brief Sets the image data with specified dimensions.
	///
	/// \param data The image data to be set.
	/// \param width The width of the image.
	/// \param height The height of the image.
	virtual void set_image_data(const data_type& data, int32_t width, int32_t height) = 0;

	/// \brief Sets the image data.
	///
	/// \param data The data to be set.
	virtual void set_image_data(const std::unordered_map<Enum::ChannelID, std::vector<T>>& data) = 0;

	/// \brief Sets the image data with specified dimensions.
	///
	/// \param data The image data to be set.
	/// \param width The width of the image.
	/// \param height The height of the image.
	virtual void set_image_data(const std::unordered_map<Enum::ChannelID, std::vector<T>>& data, int32_t width, int32_t height) = 0;

	/// \brief Sets the image data.
	///
	/// \param data The data to be set.
	virtual void set_image_data(const std::unordered_map<Enum::ChannelIDInfo, std::vector<T>>& data) = 0;

	/// \brief Sets the image data with specified dimensions.
	///
	/// \param data The image data to be set.
	/// \param width The width of the image.
	/// \param height The height of the image.
	virtual void set_image_data(const std::unordered_map<Enum::ChannelIDInfo, std::vector<T>>& data, int32_t width, int32_t height) = 0;

	/// \brief Sets the data for a specific channel.
	///
	/// \param _id The channel ID to set the data for.
	/// \param channel The channel data to be set.
	virtual void set_channel(int _id, const std::vector<T>& channel) = 0;

	/// \brief Sets the data for a specific channel.
	///
	/// \param _id The channel ID to set the data for.
	/// \param channel The channel data to be set.
	virtual void set_channel(Enum::ChannelID _id, const std::vector<T>& channel) = 0;

	/// \brief Sets the data for a specific channel, using a ChannelIDInfo.
	///
	/// \param _id The channel ID to set the data for.
	/// \param channel The channel data to be set.
	virtual void set_channel(Enum::ChannelIDInfo _id, const std::vector<T>& channel) = 0;

	/// \brief Sets the data for a specific channel.
	///
	/// \param _id The channel ID to set the data for.
	/// \param channel The channel data to be set.
	virtual void set_channel(int _id, const std::span<const T> channel) = 0;

	/// \brief Sets the data for a specific channel.
	///
	/// \param _id The channel ID to set the data for.
	/// \param channel The channel data to be set.
	virtual void set_channel(Enum::ChannelID _id, const std::span<const T> channel) = 0;

	/// \brief Sets the data for a specific channel, using a ChannelIDInfo.
	///
	/// \param _id The channel ID to set the data for.
	/// \param channel The channel data to be set.
	virtual void set_channel(Enum::ChannelIDInfo _id, const std::span<const T> channel) = 0;

	virtual ~WritableImageDataMixin() = default;

protected:

	/// \brief Internal helper method to set image data with advanced parameters.
	///
	/// This private method handles setting image data while managing exceptions
	/// during the process. It supports parallel processing across multiple channels.
	///
	/// \param data The image data to be set.
	/// \param width The width of the image.
	/// \param height The height of the image.
	/// \param center_x The center x-coordinate for the image data.
	/// \param center_y The center y-coordinate for the image data.
	/// \param colormode The color mode of the image.
	void impl_set_image_data(
		const data_type& data,
		int32_t width,
		int32_t height,
		float center_x,
		float center_y,
		Enum::ColorMode colormode
		)
	{
		// Construct variables for correct exception stack unwinding
		struct exception_info
		{
			typename data_type::key_type key;
			std::exception_ptr exception;
		};
		std::vector<exception_info> exceptions;
		std::mutex exceptions_mutex;

		// Clear image data before setting.
		ImageDataMixin<T>::m_ImageData.clear();

		std::for_each(std::execution::par_unseq, data.begin(), data.end(), [&](const auto& pair)
			{
				const auto& [key, data] = pair;
				auto id = Enum::toChannelIDInfo(static_cast<int16_t>(key), colormode);

				const auto data_span = std::span<const T>(data.begin(), data.end());
				try
				{
					this->impl_set_channel(id, data_span, width, height, center_x, center_y, colormode);
				}
				catch (...)
				{
					std::lock_guard guard(exceptions_mutex);
					exceptions.push_back({ key, std::current_exception() });
				}
			});


		// Unwind the exception stack and rethrow all as one big exception showing exactly what threw.
		std::string error_message = "Encountered the following errors while setting the image data:\n";
		for (auto& exception : exceptions)
		{
			try
			{
				std::rethrow_exception(exception.exception);
			}
			catch (const std::exception& e)
			{
				error_message += fmt::format("\t{{ channel : {} }}, {{ exception: {} }}\n", exception.key, e.what());
			}
			catch (...)
			{
				error_message += fmt::format("\t{{ channel : {} }}, {{ exception: unknown }}\n", exception.key);
			}
		}
		if (exceptions.size() > 0)
		{
			throw std::runtime_error(error_message);
		}
	}

	/// \brief Internal helper method to set image data with advanced parameters.
	///
	/// This private method handles setting image data while managing exceptions
	/// during the process. It supports parallel processing across multiple channels.
	///
	/// \param data The image data to be set.
	/// \param width The width of the image.
	/// \param height The height of the image.
	/// \param center_x The center x-coordinate for the image data.
	/// \param center_y The center y-coordinate for the image data.
	/// \param colormode The color mode of the image.
	void impl_set_image_data(
		const std::unordered_map<Enum::ChannelID, std::vector<T>>& data,
		int32_t width,
		int32_t height,
		float center_x,
		float center_y,
		Enum::ColorMode colormode
	)
	{
		data_type remapped{};
		for (const auto& [key, value] : data)
		{
			remapped[Enum::toChannelIDInfo(key, colormode).index] = std::move(value);
		}
		this->impl_set_image_data(
			remapped,
			width,
			height,
			center_x,
			center_y,
			colormode
		);
	}

	/// \brief Internal helper method to set image data with advanced parameters.
	///
	/// This private method handles setting image data while managing exceptions
	/// during the process. It supports parallel processing across multiple channels.
	///
	/// \param data The image data to be set.
	/// \param width The width of the image.
	/// \param height The height of the image.
	/// \param center_x The center x-coordinate for the image data.
	/// \param center_y The center y-coordinate for the image data.
	/// \param colormode The color mode of the image.
	void impl_set_image_data(
		const std::unordered_map<Enum::ChannelIDInfo, std::vector<T>>& data,
		int32_t width,
		int32_t height,
		float center_x,
		float center_y,
		Enum::ColorMode colormode
	)
	{
		data_type remapped{};
		for (const auto& [key, value] : data)
		{
			remapped[key.index] = std::move(value);
		}
		this->impl_set_image_data(
			remapped,
			width,
			height,
			center_x,
			center_y,
			colormode
		);
	}

	/// \brief Internal helper method to set data for a specific channel.
	///
	/// This method validates the channel and data size, then stores the data
	/// in the appropriate channel, handling exceptions if needed.
	///
	/// \param id The channel ID to set the data for.
	/// \param data The channel data to be set.
	/// \param width The width of the image.
	/// \param height The height of the image.
	/// \param center_x The center x-coordinate for the image data.
	/// \param center_y The center y-coordinate for the image data.
	/// \param colormode The color mode of the image.
	void impl_set_channel(
		Enum::ChannelIDInfo id,
		const std::span<const T> data,
		int32_t width,
		int32_t height,
		float center_x,
		float center_y,
		Enum::ColorMode colormode
	)
	{
		PSAPI_PROFILE_FUNCTION();
		if (!Enum::channelValidForColorMode(id.id, colormode))
		{
			throw std::invalid_argument(fmt::format("Unable to construct channel '{}' as it is not valid for the colormode '{}', skipping setting of this channel",
				Enum::channelIDToString(id.id), Enum::colorModeToString(colormode)));
		}
		if (data.size() != static_cast<size_t>(width) * height)
		{
			throw std::invalid_argument(
				fmt::format("Invalid data size encountered while calling set_channel(), expected <{}x{} = {:L}> but instead got <{:L}>",
				width, height, width * height, data.size()));
		}

		if (id.id == Enum::ChannelID::UserSuppliedLayerMask)
		{
			this->impl_set_mask(data, width, height, center_x, center_y);
		}
		else
		{
			ImageDataMixin<T>::m_ImageData[id] = std::make_unique<ImageChannel>(Enum::Compression::ZipPrediction, data, id, width, height, center_x, center_y);
		}
	}

	/// \brief Pure virtual method to set the mask data.
	///
	/// This method must be implemented by derived classes to handle setting
	/// mask data for specific image layers.
	///
	/// \param data The mask data to be set.
	/// \param width The width of the mask.
	/// \param height The height of the mask.
	/// \param center_x The center x-coordinate for the mask.
	/// \param center_y The center y-coordinate for the mask.
	virtual void impl_set_mask(const std::span<const T> data, int32_t width, int32_t height ,float center_x, float center_y) = 0;

};


extern template struct ImageDataMixin<bpp8_t>;
extern template struct ImageDataMixin<bpp16_t>;
extern template struct ImageDataMixin<bpp32_t>;

extern template struct WritableImageDataMixin<bpp8_t>;
extern template struct WritableImageDataMixin<bpp16_t>;
extern template struct WritableImageDataMixin<bpp32_t>;

PSAPI_NAMESPACE_END