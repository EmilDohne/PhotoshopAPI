#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Layer.h"
#include "Core/Struct/ImageChannel.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"

#include <vector>
#include <unordered_map>
#include <optional>
#include <iostream>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>


PSAPI_NAMESPACE_BEGIN


namespace
{
	// Check that the map of channels has the required amount of keys. For example for an RGB image R, G and B must be present.
	// Returns false if a specific key is not found
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool checkChannelKeys(const std::unordered_map<Enum::ChannelIDInfo, std::unique_ptr<ImageChannel>, Enum::ChannelIDInfoHasher>& data, const std::vector<Enum::ChannelIDInfo>& requiredKeys)
	{
		for (const auto& requiredKey : requiredKeys)
		{
			const auto& it = data.find(requiredKey);
			if (it == data.end())
			{
				return false;
			}
		}
		return true;
	}

}


// A pixel based image layer
template <typename T>
struct ImageLayer : public Layer<T>
{
	/// Alias for our storage data type
	using storage_type = std::unordered_map<Enum::ChannelIDInfo, std::unique_ptr<ImageChannel>, Enum::ChannelIDInfoHasher>;
	using data_type = std::unordered_map<Enum::ChannelIDInfo, std::vector<T>, Enum::ChannelIDInfoHasher>;

	/// Store the image data as a per-channel map to be used later using a custom hash function
	storage_type m_ImageData;

public:

	/// Generate an ImageLayer instance ready to be used in a LayeredFile document. 
	/// 
	/// \tparam ExecutionPolicy the execution policy to generate the image layer with, at most parallelizes to data.size()
	/// 
	/// \param data the ImageData to associate with the layer
	/// \param parameters The parameters dictating layer name, width, height, mask etc.
	/// \param policy The execution policy for the image data compression
	template <typename  ExecutionPolicy = std::execution::parallel_policy, std::enable_if_t<std::is_execution_policy_v<ExecutionPolicy>, int> = 0>
	ImageLayer(std::unordered_map<Enum::ChannelID, std::vector<T>>&& data, Layer<T>::Params& parameters, const ExecutionPolicy policy = std::execution::par)
	{
		// Change the data from being int16_t mapped to instead being mapped to a ChannelIDInfo so we can forward it
		// to the other constructor
		data_type remapped;
		for (auto& [key, value] : data)
		{
			Enum::ChannelIDInfo info = {};
			if (parameters.colorMode == Enum::ColorMode::RGB)
			{
				info = Enum::rgbChannelIDToChannelIDInfo(key);
			}
			else if (parameters.colorMode == Enum::ColorMode::CMYK)
			{
				info = Enum::cmykChannelIDToChannelIDInfo(key);
			}
			else if (parameters.colorMode == Enum::ColorMode::Grayscale)
			{
				info = Enum::grayscaleChannelIDToChannelIDInfo(key);
			}
			else
			{
				PSAPI_LOG_ERROR("ImageLayer", "Currently PhotoshopAPI only supports RGB, CMYK and Grayscale ColorMode");
			}
			remapped[info] = std::move(value);
		}
		construct(std::move(remapped), parameters, policy);
	}

	/// Generate an ImageLayer instance ready to be used in a LayeredFile document.
	/// 
	/// \tparam ExecutionPolicy the execution policy to generate the image layer with, at most parallelizes to imageData.size()
	/// 
	/// \param imageData the ImageData to associate with the channel
	/// \param layerParameters The parameters dictating layer name, width, height, mask etc.
	/// \param policy The execution policy for the image data compression
	template <typename  ExecutionPolicy = std::execution::parallel_policy, std::enable_if_t<std::is_execution_policy_v<ExecutionPolicy>, int> = 0>
	ImageLayer(std::unordered_map<int16_t, std::vector<T>>&& data, Layer<T>::Params& parameters, const ExecutionPolicy policy = std::execution::par) 
	{
		// Change the data from being int16_t mapped to instead being mapped to a ChannelIDInfo so we can forward it
		// to construct
		data_type remapped;
		for (auto& [key, value] : data)
		{
			Enum::ChannelIDInfo info = {};
			if (parameters.colorMode == Enum::ColorMode::RGB)
			{
				info = Enum::rgbIntToChannelID(key);
			}
			else if (parameters.colorMode == Enum::ColorMode::CMYK)
			{
				info = Enum::cmykIntToChannelID(key);
			}
			else if (parameters.colorMode == Enum::ColorMode::Grayscale)
			{
				info = Enum::grayscaleIntToChannelID(key);
			}
			else
			{
				PSAPI_LOG_ERROR("ImageLayer", "Currently PhotoshopAPI only supports RGB, CMYK and Grayscale ColorMode");
			}
			remapped[info] = std::move(value);
		}
		construct(std::move(remapped), parameters, policy);
	}

	/// Initialize our imageLayer by first parsing the base Layer instance and then moving
	/// the additional channels into our representation. This constructor is primarily for internal usage and it is 
	/// encouraged to use the other constructors taking image data directly instead.
	ImageLayer(const LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header) :
		Layer<T>(layerRecord, channelImageData, header)
	{
		// Move the layers into our own layer representation
		for (int i = 0; i < layerRecord.m_ChannelCount; ++i)
		{
			auto& channelInfo = layerRecord.m_ChannelInformation[i];

			// We already extract masks ahead of time and skip them here to avoid raising warnings
			if (channelInfo.m_ChannelID.id == Enum::ChannelID::UserSuppliedLayerMask) continue;

			auto channelPtr = channelImageData.extractImagePtr(channelInfo.m_ChannelID);
			// Pointers might have already been released previously
			if (!channelPtr) continue;

			// Insert any valid pointers to channels we have. We move to avoid having 
			// to uncompress / recompress
			m_ImageData[channelInfo.m_ChannelID] = std::move(channelPtr);
		}
	}


	/// Extract a specified channel from the layer given its channel ID. This also works for masks
	///
	/// \param channelID the channel ID to extract
	/// \param doCopy whether to extract the channel by copying the data. If this is false the channel will no longer hold any image data!
	std::vector<T> getChannel(const Enum::ChannelID channelID, bool doCopy = true)
	{
		if (channelID == Enum::ChannelID::UserSuppliedLayerMask)
		{
			return this->getMaskData(doCopy);
		}
		for (auto& [key, value] : m_ImageData)
		{
			if (key.id == channelID)
			{
				if (doCopy)
					return value->template getData<T>();
				else
					return value->template extractData<T>();
			}
		}
		PSAPI_LOG_WARNING("ImageLayer", "Unable to find channel in ImageData, returning an empty vector");
		return std::vector<T>();
	}

	/// Extract a specified channel from the layer given its channel ID. This also works for masks
	///
	/// \param channelIndex the channel index to extract
	/// \param doCopy whether to extract the channel by copying the data. If this is false the channel will no longer hold any image data!
	std::vector<T> getChannel(const int16_t channelIndex, bool doCopy = true)
	{
		if (channelIndex == -2)
		{
			return this->getMaskData(doCopy);
		}
		for (auto& [key, value] : m_ImageData)
		{
			if (key.index == channelIndex)
			{
				if (doCopy)
					return value->template getData<T>();
				else
					return value->template extractData<T>();
			}
		}
		PSAPI_LOG_WARNING("ImageLayer", "Unable to find channel in ImageData, returning an empty vector");
		return std::vector<T>();
	}

	/// Extract all the channels of the ImageLayer into an unordered_map. Includes the mask channel
	/// 
	/// \tparam ExecutionPolicy the execution policy to get the image data with
	/// 
	/// \param doCopy whether to extract the image data by copying the data. If this is false the channel will no longer hold any image data!
	/// \param policy The execution policy for the image data decompression
	template <typename  ExecutionPolicy = std::execution::parallel_policy, std::enable_if_t<std::is_execution_policy_v<ExecutionPolicy>, int> = 0 >
	data_type getImageData(bool doCopy = true, const ExecutionPolicy policy = std::execution::par)
	{
		PROFILE_FUNCTION();
		std::unordered_map<Enum::ChannelIDInfo, std::vector<T>, Enum::ChannelIDInfoHasher> imgData;

		if (Layer<T>::m_LayerMask.has_value())
		{
			Enum::ChannelIDInfo maskInfo;
			maskInfo.id = Enum::ChannelID::UserSuppliedLayerMask;
			maskInfo.index = -2;
			imgData[maskInfo] = Layer<T>::getMaskData(doCopy);
		}

		// Preallocate the data in parallel for some slight speedups
		std::mutex imgDataMutex;
		std::for_each(policy, m_ImageData.begin(), m_ImageData.end(),
			[&](auto& pair) {
				auto& [key, value] = pair;
				auto vec = std::vector<T>(value->m_OrigByteSize / sizeof(T));
				{
					std::lock_guard<std::mutex> lock(imgDataMutex);
					imgData[key] = std::move(vec);
				}
			});

		// We want to let the imagedata decode in as many threads as we have left over given our channels.
		// This is because images that are smaller than the blosc2 block size or images that dont have enough
		// blocks to parallelize across with all of our threads.
		size_t numThreads = std::thread::hardware_concurrency() / m_ImageData.size();
		numThreads = std::min(static_cast<size_t>(1), numThreads);
		if constexpr (std::is_same_v<ExecutionPolicy, std::execution::sequenced_policy>)
		{
			numThreads = 1;
		}

		// Construct variables for correct exception stack unwinding
		std::atomic<bool> exceptionOccurred = false;
		std::mutex exceptionMutex;
		std::vector<std::string> exceptionMessages;

		if (doCopy)
		{
			std::for_each(policy, m_ImageData.begin(), m_ImageData.end(),
				[&](auto& pair)
				{
					try
					{
						auto& [key, value] = pair;
						// Get the data using the preallocated buffer
						value->template getData<T>(std::span<T>(imgData[key]), numThreads);
					}
					catch (std::runtime_error& e)
					{
						exceptionOccurred = true;
						std::lock_guard<std::mutex> lock(exceptionMutex);
						exceptionMessages.push_back(e.what());
					}
					catch (...)
					{
						exceptionOccurred = true;
						std::lock_guard<std::mutex> lock(exceptionMutex);
						exceptionMessages.push_back("Unknown exception caught.");
					}
				});
		}
		else
		{
			std::for_each(policy, m_ImageData.begin(), m_ImageData.end(),
				[&](auto& pair)
				{
					try
					{
						// Get the data using the preallocated buffer
						pair.second->template extractData<T>(std::span<T>(imgData[pair.first]), numThreads);
					}
					catch (std::runtime_error& e)
					{
						exceptionOccurred = true;
						std::lock_guard<std::mutex> lock(exceptionMutex);
						exceptionMessages.push_back(e.what());
					}
					catch (...)
					{
						exceptionOccurred = true;
						std::lock_guard<std::mutex> lock(exceptionMutex);
						exceptionMessages.push_back("Unknown exception caught.");
					}
				});
		}


		if (exceptionOccurred)
		{
			for (const auto& msg : exceptionMessages)
			{
				PSAPI_LOG_ERROR("ImageLayer", "Exception caught: %s", msg.c_str());
			}
		}

		return imgData;
	}

	/// Change the compression codec of all the image channels
	void setCompression(const Enum::Compression compCode) override
	{
		// Change the mask channels' compression codec
		Layer<T>::setCompression(compCode);
		// Change the image channel compression codecs
		for (const auto& [key, val] : m_ImageData)
		{
			m_ImageData[key]->m_Compression = compCode;
		}
	}


	/// Generate a photoshop layerRecord and imageData based on the current layer. Mostly for internal use
	std::tuple<LayerRecord, ChannelImageData> toPhotoshop(const Enum::ColorMode colorMode, const FileHeader& header) override
	{
		PascalString lrName = Layer<T>::generatePascalString();
		ChannelExtents extents = generateChannelExtents(ChannelCoordinates(Layer<T>::m_Width, Layer<T>::m_Height, Layer<T>::m_CenterX, Layer<T>::m_CenterY), header);
		uint16_t channelCount = m_ImageData.size() + static_cast<uint16_t>(Layer<T>::m_LayerMask.has_value());

		uint8_t clipping = 0u;	// No clipping mask for now
		LayerRecords::BitFlags bitFlags(false, !Layer<T>::m_IsVisible, false);
		std::optional<LayerRecords::LayerMaskData> lrMaskData = Layer<T>::generateMaskData(header);
		LayerRecords::LayerBlendingRanges blendingRanges = Layer<T>::generateBlendingRanges(colorMode);

		// Generate our AdditionalLayerInfoSection. We dont need any special Tagged Blocks besides what is stored by the generic layer
		auto blockVec = this->generateTaggedBlocks();
		std::optional<AdditionalLayerInfo> taggedBlocks = std::nullopt;
		if (blockVec.size() > 0)
		{
			TaggedBlockStorage blockStorage = { blockVec };
			taggedBlocks.emplace(blockStorage);
		}

		// Initialize the channel information as well as the channel image data, the size held in the channelInfo might change depending on
		// the compression mode chosen on export and must therefore be updated later. This step is done last as generateChannelImageData() invalidates
		// all image data which we might need for operations above
		auto channelData = this->generateChannelImageData();
		auto& channelInfoVec = std::get<0>(channelData);
		ChannelImageData channelImgData = std::move(std::get<1>(channelData));

		LayerRecord lrRecord = LayerRecord(
			lrName,
			extents.top,
			extents.left,
			extents.bottom,
			extents.right,
			channelCount,
			channelInfoVec,
			Layer<T>::m_BlendMode,
			Layer<T>::m_Opacity,
			clipping,
			bitFlags,
			lrMaskData,
			blendingRanges,
			std::move(taggedBlocks)
		);

		return std::make_tuple(std::move(lrRecord), std::move(channelImgData));
	}

private:
		
	/// Construct the ImageLayer, to be called by the individual constructors
	template <typename  ExecutionPolicy = std::execution::parallel_policy, std::enable_if_t<std::is_execution_policy_v<ExecutionPolicy>, int> = 0>
	void construct(data_type&& data, Layer<T>::Params& parameters, const ExecutionPolicy policy)
	{
		PROFILE_FUNCTION();
		Layer<T>::m_LayerName = parameters.layerName;
		if (parameters.blendMode == Enum::BlendMode::Passthrough)
		{
			PSAPI_LOG_WARNING("ImageLayer", "The Passthrough blend mode is reserved for groups, defaulting to 'Normal'");
			Layer<T>::m_BlendMode = Enum::BlendMode::Normal;
		}
		else
		{
			Layer<T>::m_BlendMode = parameters.blendMode;
		}
		Layer<T>::m_Opacity = parameters.opacity;
		Layer<T>::m_IsVisible = parameters.isVisible;
		Layer<T>::m_CenterX = parameters.posX;
		Layer<T>::m_CenterY = parameters.posY;
		Layer<T>::m_Width = parameters.width;
		Layer<T>::m_Height = parameters.height;

		// Forward the mask channel if it was passed as part of the image data to the layer mask
		// The actual populating of the mask channel will be done further down
		const auto maskID = Enum::ChannelIDInfo{ .id = Enum::ChannelID::UserSuppliedLayerMask, .index = -2 };
		if (data.contains(maskID))
		{
			if (parameters.layerMask)
			{
				PSAPI_LOG_ERROR("ImageLayer",
					"Got mask from both the ImageData as index -2 and as part of the layer parameter, please only pass it as one of these");
			}

			PSAPI_LOG_DEBUG("ImageLayer", "Forwarding mask channel passed as part of image data to m_LayerMask");
			parameters.layerMask = std::move(data[maskID]);
			data.erase(maskID);
		}

		// Construct an ImageChannel instance for each of the passed channels according to the given execution policy
		std::for_each(policy, data.begin(), data.end(), [&](auto& pair)
			{
				auto& [info, value] = pair;
				// Channel sizes must match the size of the layer
				if (static_cast<uint64_t>(parameters.width) * parameters.height > value.size()) [[unlikely]]
				{
					PSAPI_LOG_ERROR("ImageLayer", "Size of ImageChannel does not match the size of width * height, got %" PRIu64 " but expected %" PRIu64 ".",
						value.size(),
						static_cast<uint64_t>(parameters.width)* parameters.height);
				}
				m_ImageData[info] = std::make_unique<ImageChannel>(
					parameters.compression,
					value,
					info,
					parameters.width,
					parameters.height,
					static_cast<float>(parameters.posX),
					static_cast<float>(parameters.posY)
				);
			});

		// Check that the required keys are actually present. e.g. for an RGB colorMode the channels R, G and B must be present
		if (parameters.colorMode == Enum::ColorMode::RGB)
		{
			Enum::ChannelIDInfo channelR = { .id = Enum::ChannelID::Red, .index = 0 };
			Enum::ChannelIDInfo channelG = { .id = Enum::ChannelID::Green, .index = 1 };
			Enum::ChannelIDInfo channelB = { .id = Enum::ChannelID::Blue, .index = 2 };
			std::vector<Enum::ChannelIDInfo> channelVec = { channelR, channelG, channelB };
			bool hasRequiredChannels = checkChannelKeys(m_ImageData, channelVec);
			if (!hasRequiredChannels)
			{
				PSAPI_LOG_ERROR("ImageLayer", "For RGB ColorMode R, G and B channels need to be specified");
			}
		}
		else if (parameters.colorMode == Enum::ColorMode::CMYK)
		{
			Enum::ChannelIDInfo channelC = { .id = Enum::ChannelID::Cyan, .index = 0 };
			Enum::ChannelIDInfo channelM = { .id = Enum::ChannelID::Magenta, .index = 1 };
			Enum::ChannelIDInfo channelY = { .id = Enum::ChannelID::Yellow, .index = 2 };
			Enum::ChannelIDInfo channelK = { .id = Enum::ChannelID::Black, .index = 3 };
			std::vector<Enum::ChannelIDInfo> channelVec = { channelC, channelM, channelY, channelK };
			bool hasRequiredChannels = checkChannelKeys(m_ImageData, channelVec);
			if (!hasRequiredChannels)
			{
				PSAPI_LOG_ERROR("ImageLayer", "For CMYK ColorMode C, M, Y and K channels need to be specified");
			}
		}
		else if (parameters.colorMode == Enum::ColorMode::Grayscale)
		{
			Enum::ChannelIDInfo channelG = { .id = Enum::ChannelID::Gray, .index = 0 };
			std::vector<Enum::ChannelIDInfo> channelVec = { channelG };
			bool hasRequiredChannels = checkChannelKeys(m_ImageData, { channelG });
			if (!hasRequiredChannels)
			{
				PSAPI_LOG_ERROR("ImageLayer", "For Grayscale ColorMode Gray channel needs to be specified");
			}
		}
		else
		{
			PSAPI_LOG_ERROR("ImageLayer", "The PhotoshopAPI currently only supports RGB, CMYK and Greyscale colour modes");
		}

		Layer<T>::parseLayerMask(parameters);
	}

	// Extracts the m_ImageData as well as the layer mask into two vectors holding channel information as well as the image data 
	// itself. This also takes care of generating our layer mask channel if it is present. Invalidates any data held by the ImageLayer
	std::tuple<std::vector<LayerRecords::ChannelInformation>, ChannelImageData> generateChannelImageData()
	{
		std::vector<LayerRecords::ChannelInformation> channelInfoVec;
		std::vector<std::unique_ptr<ImageChannel>> channelDataVec;

		// First extract our mask data, the order of our channels does not matter as long as the 
		// order of channelInfo and channelData is the same
		auto maskData = Layer<T>::extractLayerMask();
		if (maskData.has_value())
		{
			channelInfoVec.push_back(std::get<0>(maskData.value()));
			channelDataVec.push_back(std::move(std::get<1>(maskData.value())));
		}

		// Extract all the channels next and push them into our data representation
		for (auto& it : m_ImageData)
		{
			channelInfoVec.push_back(LayerRecords::ChannelInformation{ it.first, it.second->m_OrigByteSize });
			channelDataVec.push_back(std::move(it.second));
		}

		// Construct the channel image data from our vector of ptrs, moving gets handled by the constructor
		ChannelImageData channelData(std::move(channelDataVec));

		return std::make_tuple(channelInfoVec, std::move(channelData));
	}

};

PSAPI_NAMESPACE_END