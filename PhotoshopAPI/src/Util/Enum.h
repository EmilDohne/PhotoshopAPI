#pragma once

#include "Macros.h"
#include "Logger.h"

#include <unordered_map>
#include <vector>
#include <optional>
#include <algorithm>

PSAPI_NAMESPACE_BEGIN


// Utility
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
namespace 
{
	// Template function for finding any of the enums by  their value defined in the respective std::map
	template <typename KeyType, typename ValueType>
	inline std::optional<KeyType> findByValue(const std::unordered_map<KeyType, ValueType>& map, const ValueType searchValue)
	{
		for (const auto& pair : map)
		{
			if (pair.second == searchValue)
			{
				return pair.first;
			}
		}
		return std::nullopt;
	}

	// Template function for finding any of the enums by their value defined in the respective std::map
	template <typename KeyType, typename ValueType>
	inline std::optional<std::vector<KeyType>> findMultipleByValue(const std::unordered_map<KeyType, ValueType>& map, const ValueType searchValue)
	{
		std::vector<KeyType> results;
		for (const auto& pair : map)
		{
			if (pair.second == searchValue)
			{
				results.push_back(pair.first);
			}
		}
		if (results.size() == 0)
		{
			return std::nullopt;
		}
		return results;
	}
}


// Header Enums
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
namespace Enum
{
	// File Format Version
	enum class Version
	{
		Psd,
		Psb
	};

	inline std::unordered_map<uint16_t, Version> versionMap
	{
		{static_cast<uint16_t>(1u), Version::Psd},
		{static_cast<uint16_t>(2u), Version::Psb}
	};

	// Bit Depth of the Document
	enum class BitDepth
	{
		BD_1,
		BD_8,
		BD_16,
		BD_32
	};

	template <typename T>
		requires std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, float32_t> 
	BitDepth bit_depth_from_t()
	{
		if constexpr (std::is_same_v<T, uint8_t>)
		{
			return BitDepth::BD_8;
		}
		else if constexpr (std::is_same_v<T, uint16_t>)
		{
			return BitDepth::BD_16;
		}
		else
		{
			return BitDepth::BD_32;
		}
	}

	inline std::unordered_map<uint16_t, BitDepth> bitDepthMap
	{
		{static_cast<uint16_t>(1u), BitDepth::BD_1},
		{static_cast<uint16_t>(8u), BitDepth::BD_8},
		{static_cast<uint16_t>(16u), BitDepth::BD_16},
		{static_cast<uint16_t>(32u), BitDepth::BD_32},
	};

	inline uint16_t bitDepthToUint(Enum::BitDepth bitdepth)
	{
		switch (bitdepth)
		{
		case Enum::BitDepth::BD_1:
			return static_cast<uint16_t>(1u);
		case Enum::BitDepth::BD_8:
			return static_cast<uint16_t>(8u);
		case Enum::BitDepth::BD_16:
			return static_cast<uint16_t>(16u);
		case Enum::BitDepth::BD_32:
			return static_cast<uint16_t>(32u);
		default:
			return static_cast<uint16_t>(0u);
		}
	}

	enum class ColorMode
	{
		Bitmap,
		Grayscale,
		Indexed,
		RGB,
		CMYK,
		Multichannel,
		Duotone,
		Lab
	};

	inline std::unordered_map<uint16_t, ColorMode> colorModeMap
	{
		{static_cast<uint16_t>(0u), ColorMode::Bitmap},
		{static_cast<uint16_t>(1u), ColorMode::Grayscale},
		{static_cast<uint16_t>(2u), ColorMode::Indexed},
		{static_cast<uint16_t>(3u), ColorMode::RGB},
		{static_cast<uint16_t>(4u), ColorMode::CMYK},
		{static_cast<uint16_t>(7u), ColorMode::Multichannel},
		{static_cast<uint16_t>(8u), ColorMode::Duotone},
		{static_cast<uint16_t>(9u), ColorMode::Lab}
	};

	inline std::string colorModeToString (const Enum::ColorMode value)
	{
		switch (value)
		{
		case Enum::ColorMode::Bitmap: return "Bitmap";
		case Enum::ColorMode::Grayscale: return "Grayscale";
		case Enum::ColorMode::Indexed: return "Indexed";
		case Enum::ColorMode::RGB: return "RGB";
		case Enum::ColorMode::CMYK: return "CMYK";
		case Enum::ColorMode::Multichannel: return "Multichannel";
		case Enum::ColorMode::Duotone: return "Duotone";
		case Enum::ColorMode::Lab: return "Lab";
		default: return "Unknown";
		}
	}
}


// Image Resource Enums
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
namespace Enum
{
	enum class ImageResource
	{
		NotImplemented,
		ResolutionInfo,
		AlphaChannelNames,
		BackgroundColor,
		PrintFlags,
		ColorHalftoningInfo,
		ColorTransferFunctions,
		LayerStateInformation,
		LayerGroupInformation,
		IPTCRecord,
		GridAndGuidesInformation,
		ThumbnailResource,
		GlobalAngle,
		ICCProfile,
		ICCUntaggedProfile,
		SpotHalftone,
		IDSeed,
		UnicodeAlphaNames,
		GlobalAltitude,
		Slices,
		AlphaIdentifiers,
		URLList,
		VersionInfo,
		ExifData1,
		XMPMetadata,
		CaptionDigest,
		PrintScale,
		PixelAspectRatio,
		AlternateSpotColors,
		LayerSelectionID,
		LayerGroupEnabledID,
		DisplayInfo,
		PrintInformation,
		PrintStyle,
		PrintFlagsInfo
	};

	namespace {
		inline std::unordered_map<uint16_t, ImageResource> imageResourceMap = 
		{
			{static_cast<uint16_t>(1u), ImageResource::NotImplemented},
			{static_cast<uint16_t>(1005u), ImageResource::ResolutionInfo},
			{static_cast<uint16_t>(1006u), ImageResource::AlphaChannelNames},
			{static_cast<uint16_t>(1010u), ImageResource::BackgroundColor},
			{static_cast<uint16_t>(1011u), ImageResource::PrintFlags},
			{static_cast<uint16_t>(1013u), ImageResource::ColorHalftoningInfo},
			{static_cast<uint16_t>(1016u), ImageResource::ColorTransferFunctions},
			{static_cast<uint16_t>(1024u), ImageResource::LayerStateInformation},
			{static_cast<uint16_t>(1026u), ImageResource::LayerGroupInformation},
			{static_cast<uint16_t>(1028u), ImageResource::IPTCRecord},
			{static_cast<uint16_t>(1032u), ImageResource::GridAndGuidesInformation},
			{static_cast<uint16_t>(1036u), ImageResource::ThumbnailResource},
			{static_cast<uint16_t>(1037u), ImageResource::GlobalAngle},
			{static_cast<uint16_t>(1039u), ImageResource::ICCProfile},
			{static_cast<uint16_t>(1041u), ImageResource::ICCUntaggedProfile},
			{static_cast<uint16_t>(1043u), ImageResource::SpotHalftone},
			{static_cast<uint16_t>(1044u), ImageResource::IDSeed},
			{static_cast<uint16_t>(1045u), ImageResource::UnicodeAlphaNames},
			{static_cast<uint16_t>(1049u), ImageResource::GlobalAltitude},
			{static_cast<uint16_t>(1050u), ImageResource::Slices},
			{static_cast<uint16_t>(1053u), ImageResource::AlphaIdentifiers},
			{static_cast<uint16_t>(1054u), ImageResource::URLList},
			{static_cast<uint16_t>(1057u), ImageResource::VersionInfo},
			{static_cast<uint16_t>(1058u), ImageResource::ExifData1},
			{static_cast<uint16_t>(1060u), ImageResource::XMPMetadata},
			{static_cast<uint16_t>(1061u), ImageResource::CaptionDigest},
			{static_cast<uint16_t>(1062u), ImageResource::PrintScale},
			{static_cast<uint16_t>(1064u), ImageResource::PixelAspectRatio},
			{static_cast<uint16_t>(1067u), ImageResource::AlternateSpotColors},
			{static_cast<uint16_t>(1069u), ImageResource::LayerSelectionID},
			{static_cast<uint16_t>(1072u), ImageResource::LayerGroupEnabledID},
			{static_cast<uint16_t>(1077u), ImageResource::DisplayInfo},
			{static_cast<uint16_t>(1082u), ImageResource::PrintInformation},
			{static_cast<uint16_t>(1083u), ImageResource::PrintStyle},
			{static_cast<uint16_t>(10000u), ImageResource::PrintFlagsInfo}

		};
	}

	inline ImageResource intToImageResource(uint16_t key)
	{
		auto it = imageResourceMap.find(key);

		if (it != imageResourceMap.end()) {
			return it->second;
		}
		return ImageResource::NotImplemented;
	}

	inline uint16_t imageResourceToInt(ImageResource key)
	{
		return findByValue(imageResourceMap, key).value();
	}

	enum class ResolutionUnit
	{
		PixelsPerInch,
		PixelsPerCm
	};
	// Since this struct is so simple we do bidirectional mapping directly here
	inline std::unordered_map<uint16_t, ResolutionUnit> resolutionUnitMap =
	{
		{static_cast<uint16_t>(1u), ResolutionUnit::PixelsPerInch},
		{static_cast<uint16_t>(2u), ResolutionUnit::PixelsPerCm},
	};
	// Since this struct is so simple we do bidirectional mapping directly here
	inline std::unordered_map<ResolutionUnit, uint16_t> resolutionUnitMapRev =
	{
		{ResolutionUnit::PixelsPerInch, static_cast<uint16_t>(1u)},
		{ResolutionUnit::PixelsPerCm, static_cast<uint16_t>(2u)},
	};


	enum class DisplayUnit
	{
		Inches,
		Cm,
		Points,
		Picas,
		Columns
	};
	// Since this struct is so simple we do bidirectional mapping directly here
	inline std::unordered_map<uint16_t, DisplayUnit> displayUnitMap =
	{
		{static_cast<uint16_t>(1u), DisplayUnit::Inches},
		{static_cast<uint16_t>(2u), DisplayUnit::Cm},
		{static_cast<uint16_t>(3u), DisplayUnit::Points},
		{static_cast<uint16_t>(4u), DisplayUnit::Picas},
		{static_cast<uint16_t>(5u), DisplayUnit::Columns}
	};
	// Since this struct is so simple we do bidirectional mapping directly here
	inline std::unordered_map<DisplayUnit, uint16_t> displayUnitMapRev =
	{
		{DisplayUnit::Inches, static_cast<uint16_t>(1u)},
		{DisplayUnit::Cm, static_cast<uint16_t>(2u)},
		{DisplayUnit::Points, static_cast<uint16_t>(3u)},
		{DisplayUnit::Picas, static_cast<uint16_t>(4u)},
		{DisplayUnit::Columns, static_cast<uint16_t>(5u)}
	};

}


// Layer and Mask Information Enums
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
namespace Enum
{
	// Channel ID information
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	enum class ChannelID
	{
		/// Channel 0 in RGB Mode
		Red,
		/// Channel 1 in RGB Mode
		Green,
		/// Channel 2 in RGB Mode
		Blue,
		/// Channel 0 in CMYK Mode
		Cyan,		
		/// Channel 1 in CMYK Mode
		Magenta,	
		/// Channel 2 in CMYK Mode
		Yellow,		
		/// Channel 3 in CMYK Mode
		Black,	
		/// Channel 0 in Grayscale Mode
		Gray,		
		/// Any other channel
		Custom,		
		/// Alpha Channel
		Alpha,		
		/// Pixel Mask or Vector Mask
		UserSuppliedLayerMask,		
		/// Vector and Pixel mask combined
		RealUserSuppliedLayerMask,	

	};

	// Structure to hold both the unique identifier of a layer as well as its index 
	// allowing for much easier handling later on
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	struct ChannelIDInfo
	{
		ChannelID id{};
		int16_t index{};

		constexpr inline ChannelIDInfo() noexcept = default;
		constexpr inline ChannelIDInfo(ChannelID id, int16_t index) noexcept
			: id(id), index(index) {}

		constexpr inline bool operator==(const ChannelIDInfo& other) const
		{
			return (this->id == other.id && this->index == other.index);
		}
	};

	// Define a custom hasher to allow us to use the above struct. As the indices are unique it is perfectly legal to have the indices define the hash
	struct ChannelIDInfoHasher
	{
		std::size_t operator()(const ChannelIDInfo& k) const
		{
			using std::hash;

			return (hash<int>()(k.index));
		}
	};
	
	namespace Impl
	{
		inline ChannelIDInfo rgbChannelIDToChannelIDInfo(const Enum::ChannelID value)
		{
			switch (value)
			{
			case Enum::ChannelID::Red: return ChannelIDInfo{ value, 0 };
			case Enum::ChannelID::Green: return ChannelIDInfo{ value, 1 };
			case Enum::ChannelID::Blue: return ChannelIDInfo{ value, 2 };
			case Enum::ChannelID::Alpha: return ChannelIDInfo{ value, -1 };
			case Enum::ChannelID::UserSuppliedLayerMask: return ChannelIDInfo{ value, -2 };
			default: PSAPI_LOG_ERROR("ChannelID", "No suitable conversion found for the given channelID"); return ChannelIDInfo{};
			}
		}

		inline ChannelIDInfo rgbIntToChannelID(const int16_t value) noexcept
		{
			switch (value)
			{
			case 0: return ChannelIDInfo{ ChannelID::Red, value };
			case 1: return ChannelIDInfo{ ChannelID::Green, value };
			case 2: return ChannelIDInfo{ ChannelID::Blue, value };
			case -1: return ChannelIDInfo{ ChannelID::Alpha, value };
			case -2: return ChannelIDInfo{ ChannelID::UserSuppliedLayerMask, value };
			case -3: return ChannelIDInfo{ ChannelID::RealUserSuppliedLayerMask, value };
			default: return ChannelIDInfo{ ChannelID::Custom, value };	// These are channels set by the user
			}
		}

		inline ChannelIDInfo cmykChannelIDToChannelIDInfo(const Enum::ChannelID value)
		{
			switch (value)
			{
			case Enum::ChannelID::Cyan: return ChannelIDInfo{ value, 0 };
			case Enum::ChannelID::Magenta: return ChannelIDInfo{ value, 1 };
			case Enum::ChannelID::Yellow: return ChannelIDInfo{ value, 2 };
			case Enum::ChannelID::Black: return ChannelIDInfo{ value, 2 };
			case Enum::ChannelID::Alpha: return ChannelIDInfo{ value, -1 };
			case Enum::ChannelID::UserSuppliedLayerMask: return ChannelIDInfo{ value, -2 };
			default: PSAPI_LOG_ERROR("ChannelID", "No suitable conversion found for the given channelID"); return ChannelIDInfo{};
			}
		}

		inline ChannelIDInfo cmykIntToChannelID(const int16_t value) noexcept
		{
			switch (value)
			{
			case 0: return ChannelIDInfo{ ChannelID::Cyan, value };
			case 1: return ChannelIDInfo{ ChannelID::Magenta, value };
			case 2: return ChannelIDInfo{ ChannelID::Yellow, value };
			case 3: return ChannelIDInfo{ ChannelID::Black, value };
			case -1: return ChannelIDInfo{ ChannelID::Alpha, value };
			case -2: return ChannelIDInfo{ ChannelID::UserSuppliedLayerMask, value };
			case -3: return ChannelIDInfo{ ChannelID::RealUserSuppliedLayerMask, value };
			default: return ChannelIDInfo{ ChannelID::Custom, value };	// These are channels set by the user
			}
		}

		inline ChannelIDInfo grayscaleIntToChannelID(const int16_t value) noexcept
		{
			switch (value)
			{
			case 0: return ChannelIDInfo{ ChannelID::Gray, value };
			case -1: return ChannelIDInfo{ ChannelID::Alpha, value };
			case -2: return ChannelIDInfo{ ChannelID::UserSuppliedLayerMask, value };
			case -3: return ChannelIDInfo{ ChannelID::RealUserSuppliedLayerMask, value };
			default: return ChannelIDInfo{ ChannelID::Custom, value };	// These are channels set by the user
			}
		}

		inline ChannelIDInfo grayscaleChannelIDToChannelIDInfo(const Enum::ChannelID value)
		{
			switch (value)
			{
			case Enum::ChannelID::Gray: return ChannelIDInfo{ value, 0 };
			case Enum::ChannelID::Alpha: return ChannelIDInfo{ value, -1 };
			case Enum::ChannelID::UserSuppliedLayerMask: return ChannelIDInfo{ value, -2 };
			default: PSAPI_LOG_ERROR("ChannelID", "No suitable conversion found for the given channelID"); return ChannelIDInfo{};
			}
		}

		inline ChannelIDInfo channelIDToChannelIDInfo(const Enum::ChannelID value, const Enum::ColorMode colorMode)
		{
			switch (colorMode)
			{
			case Enum::ColorMode::RGB: return rgbChannelIDToChannelIDInfo(value);
			case Enum::ColorMode::CMYK: return cmykChannelIDToChannelIDInfo(value);
			case Enum::ColorMode::Grayscale: return grayscaleChannelIDToChannelIDInfo(value);
			default: PSAPI_LOG_ERROR("ChannelID", "No suitable conversion found for the given channelID"); return ChannelIDInfo{};
			}
		}

		inline ChannelIDInfo intToChannelIDInfo(const int16_t value, const Enum::ColorMode colorMode)
		{
			switch (colorMode)
			{
			case Enum::ColorMode::RGB: return rgbIntToChannelID(value);
			case Enum::ColorMode::CMYK: return cmykIntToChannelID(value);
			case Enum::ColorMode::Grayscale: return grayscaleIntToChannelID(value);
			default: PSAPI_LOG_ERROR("ChannelID", "No suitable conversion found for the given index"); return ChannelIDInfo{};
			}
		}

	}

	/// Convert a Enum::ChannelID or int16_t to a ChannelIDInfo representation
	template<typename T>
	ChannelIDInfo toChannelIDInfo(const T value, const Enum::ColorMode colorMode);

	// Specialization for Enum::ChannelID
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	template<>
	inline ChannelIDInfo toChannelIDInfo<Enum::ChannelID>(const Enum::ChannelID value, const Enum::ColorMode colorMode)
	{
		switch (colorMode)
		{
		case Enum::ColorMode::RGB:
			return Impl::rgbChannelIDToChannelIDInfo(value);
		case Enum::ColorMode::CMYK:
			return Impl::cmykChannelIDToChannelIDInfo(value);
		case Enum::ColorMode::Grayscale:
			return Impl::grayscaleChannelIDToChannelIDInfo(value);
		default:
			PSAPI_LOG_ERROR("ChannelID", "No suitable conversion found for the given channelID and colormode %s", Enum::colorModeToString(colorMode).c_str());
			return ChannelIDInfo{};
		}
	}

	// Specialization for int16_t
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	template<>
	inline ChannelIDInfo toChannelIDInfo<int16_t>(const int16_t value, const Enum::ColorMode colorMode)
	{
		switch (colorMode)
		{
		case Enum::ColorMode::RGB:
			return Impl::rgbIntToChannelID(value);
		case Enum::ColorMode::CMYK:
			return Impl::cmykIntToChannelID(value);
		case Enum::ColorMode::Grayscale:
			return Impl::grayscaleIntToChannelID(value);
		default:
			PSAPI_LOG_ERROR("ChannelID", "No suitable conversion found for the given index %d and colormode %s", value, Enum::colorModeToString(colorMode).c_str());
			return ChannelIDInfo{};
		}
	}
	

	// Specialization for int
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	template<>
	inline ChannelIDInfo toChannelIDInfo<int>(const int value, const Enum::ColorMode colorMode)
	{
		switch (colorMode)
		{
		case Enum::ColorMode::RGB:
			return Impl::rgbIntToChannelID(static_cast<int16_t>(value));
		case Enum::ColorMode::CMYK:
			return Impl::cmykIntToChannelID(static_cast<int16_t>(value));
		case Enum::ColorMode::Grayscale:
			return Impl::grayscaleIntToChannelID(static_cast<int16_t>(value));
		default:
			PSAPI_LOG_ERROR("ChannelID", "No suitable conversion found for the given index %d and colormode %s", value, Enum::colorModeToString(colorMode).c_str());
			return ChannelIDInfo{};
		}
	}


	/// Checks whether the channelid is valid for the given colormode, valid template instantiations are with
	/// T: Enum::ChannelID
	/// T: int16_t
	template <typename T>
	bool channelValidForColorMode(T channelid, const Enum::ColorMode colormode);


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <>
	inline bool channelValidForColorMode(const Enum::ChannelID channelid, const Enum::ColorMode colormode)
	{
		if (colormode == Enum::ColorMode::RGB)
		{
			static const std::vector<Enum::ChannelID> allowedIds =
			{
				Enum::ChannelID::Red,
				Enum::ChannelID::Green,
				Enum::ChannelID::Blue,
				Enum::ChannelID::Alpha,
				Enum::ChannelID::UserSuppliedLayerMask,
				Enum::ChannelID::RealUserSuppliedLayerMask,
			};
			if (std::find(allowedIds.begin(), allowedIds.end(), channelid) == allowedIds.end())
			{
				return false;
			}
			return true;
		}
		else if (colormode == Enum::ColorMode::CMYK)
		{
			static const std::vector<Enum::ChannelID> allowedIds =
			{
				Enum::ChannelID::Cyan,
				Enum::ChannelID::Magenta,
				Enum::ChannelID::Yellow,
				Enum::ChannelID::Black,
				Enum::ChannelID::Alpha,
				Enum::ChannelID::UserSuppliedLayerMask,
				Enum::ChannelID::RealUserSuppliedLayerMask,
			};
			if (std::find(allowedIds.begin(), allowedIds.end(), channelid) == allowedIds.end())
			{
				return false;
			}
			return true;
		}
		else if (colormode == Enum::ColorMode::Grayscale)
		{
			static const std::vector<Enum::ChannelID> allowedIds =
			{
				Enum::ChannelID::Gray,
				Enum::ChannelID::Alpha,
				Enum::ChannelID::UserSuppliedLayerMask,
				Enum::ChannelID::RealUserSuppliedLayerMask,
			};
			if (std::find(allowedIds.begin(), allowedIds.end(), channelid) == allowedIds.end())
			{
				return false;
			}
			return true;
		}
		PSAPI_LOG_ERROR("ImageLayer", "Unable to check ChannelID for the provided colormode as this is not currently implemented");
		return false;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <>
	inline bool channelValidForColorMode(const int16_t channelid, const Enum::ColorMode colormode)
	{
		auto idinfo = Enum::toChannelIDInfo(channelid, colormode);
		return channelValidForColorMode<Enum::ChannelID>(idinfo.id, colormode);
	}


	inline std::string channelIDToString(const Enum::ChannelID value) noexcept
	{
		switch (value)
		{
		case Enum::ChannelID::Red: return "red";
		case Enum::ChannelID::Green: return "green";
		case Enum::ChannelID::Blue: return "blue";
		case Enum::ChannelID::Gray: return "gray";
		case Enum::ChannelID::Custom: return "custom";
		case Enum::ChannelID::Cyan: return "cyan";
		case Enum::ChannelID::Magenta: return "magenta";
		case Enum::ChannelID::Yellow: return "yellow";
		case Enum::ChannelID::Black: return "black";
		case Enum::ChannelID::Alpha: return "alpha";
		case Enum::ChannelID::UserSuppliedLayerMask: return "pixelmask";
		case Enum::ChannelID::RealUserSuppliedLayerMask: return "mixed_mask";
		default: return "unknown";
		}
	}

	// This Enum represents an exact mapping of all of Photoshop blendmodes
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	enum class BlendMode
	{
		/// Only available on group layers
		Passthrough,
		Normal,
		Dissolve,
		Darken,
		Multiply,
		ColorBurn,
		LinearBurn,
		DarkerColor,
		Lighten,
		Screen,
		ColorDodge,
		LinearDodge,
		LighterColor,
		Overlay,
		SoftLight,
		HardLight,
		VividLight,
		LinearLight,
		PinLight,
		HardMix,
		Difference,
		Exclusion,
		Subtract,
		Divide,
		Hue,
		Saturation,
		Color,
		Luminosity
	};

	namespace {
		inline std::unordered_map<std::string, BlendMode> blendModeMap
		{
			{"pass", BlendMode::Passthrough},
			{"norm", BlendMode::Normal},
			{"diss", BlendMode::Dissolve},
			{"dark", BlendMode::Darken},
			{"mul ", BlendMode::Multiply},
			{"idiv", BlendMode::ColorBurn},
			{"lbrn", BlendMode::LinearBurn},
			{"dkCl", BlendMode::DarkerColor},
			{"lite", BlendMode::Lighten},
			{"scrn", BlendMode::Screen},
			{"div ", BlendMode::ColorDodge},
			{"lddg", BlendMode::LinearDodge},
			{"lgCl", BlendMode::LighterColor},
			{"over", BlendMode::Overlay},
			{"sLit", BlendMode::SoftLight},
			{"hLit", BlendMode::HardLight},
			{"vLit", BlendMode::VividLight},
			{"lLit", BlendMode::LinearLight},
			{"pLit", BlendMode::PinLight},
			{"hMix", BlendMode::HardMix},
			{"diff", BlendMode::Difference},
			{"smud", BlendMode::Exclusion},
			{"fsub", BlendMode::Subtract},
			{"fdiv", BlendMode::Divide},
			{"hue ", BlendMode::Hue},
			{"sat ", BlendMode::Saturation},
			{"colr", BlendMode::Color},
			{"lum ", BlendMode::Luminosity}
		};
	}

	// Bidirectional mapping of blend modes
	template<typename TKey, typename TValue>
	inline std::optional<TValue> getBlendMode(TKey key)
	{
		PSAPI_LOG_ERROR("getBlendMode", "No overload for the specific search type found");
	}

	template<>
	inline std::optional<BlendMode> getBlendMode(const std::string key)
	{
		auto it = blendModeMap.find(key);

		if (it != blendModeMap.end()) {
			return std::optional<BlendMode>(it->second);
		}
		else {
			return std::nullopt;
		}
	}

	template <>
	inline std::optional<std::string> getBlendMode(const BlendMode key)
	{
		return findByValue(blendModeMap, key);
	}


	// Tagged Block keys for Additional Layer information
	// --------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------
	enum class TaggedBlockKey
	{
		Unknown,
		// Adjustment Layers, for now unimplemented
		adjSolidColor,
		adjGradient,
		adjPattern,
		adjBrightnessContrast,
		adjLevels,
		adjCurves,
		adjExposure,
		adjVibrance,
		adjOldHueSat,	// This should never be encountered
		adjNewHueSat,	// New referring to post photoshop 5.0 here (1998)
		adjColorBalance,
		adjBlackandWhite,
		adjPhotoFilter,
		adjChannelMixer,
		adjColorLookup,
		adjInvert,
		adjPosterize,
		adjThreshold,
		adjGradientMap,
		adjSelectiveColor,
		// Effects Layer, not planned
		// Gets its own prefix as it encompasses many different types of other layers
		fxLayer,
		// Tagged blocks with information about the layer (likely only present for the additional layer info
		// at the end of each layer record. Not the one at the end of the layer and mask section
		lrUnicodeName,
		lrId,
		lrSectionDivider,	// This stores information about if it is a group layer and if its open or closed
		lrArtboard,			// Whether or not the layer is an Artboard layer. May be the keys 'artb', 'artd' or 'abdd'
		lrMetaData,
		lrAnnotations,
		// Non-Pixel layers
		lrTypeTool,		// This is the superseeded version 'TySh', not 'tySh' as that was phased out in 2000
		lrPatternData,
		lrLinked,
		lrLinked_8Byte,	// Same as lrLinked but for some reason 'lnk2' has a 8-byte wide length field
		lrPlaced,	// Represents the keys 'SoLd'
		lrPlacedData,	// Represents the keys 'PlLd' and 'plLd'
		// Additional layer specific data
		lrCompositorUsed,
		lrSavingMergedTransparency,	// Holds no data, just indicates channel Image data section includes transparency (needs to be tested)
		lrPixelSourceData,	// Data for 3d or video layers
		lrUserMask,
		// 16- and 32-bit files store their layer records under these tagged blocks at the end of the layer
		// and mask information section
		Lr16,
		Lr32,
		Layr,
		// Unknown 8-byte wide types
		Alph,
		lrFilterMask,
		lrFilterEffects,
		//
		lrBlendClippingElements,
		lrBlendInteriorElements,
		lrKnockoutSetting,
		lrProtectedSetting,
		lrSheetColorSetting,
		lrReferencePoint,
		// Shape Layer Tagged Blocks
		vecOriginData,
		vecMaskSettings,	// 'vmsk' for CS6 and up. We dont support the legacy 'vsms' option here 
		vecStrokeData,
		vecStrokeContentData,
	};

	namespace {
		inline std::unordered_map<std::string, TaggedBlockKey> taggedBlockMap
		{
			{"SoCo", TaggedBlockKey::adjSolidColor},
			{"GdFl", TaggedBlockKey::adjGradient},
			{"PtFl", TaggedBlockKey::adjPattern},
			{"brit", TaggedBlockKey::adjBrightnessContrast},
			{"levl", TaggedBlockKey::adjLevels},
			{"curv", TaggedBlockKey::adjCurves},
			{"expA", TaggedBlockKey::adjExposure},
			{"vibA", TaggedBlockKey::adjVibrance},
			{"hue ", TaggedBlockKey::adjOldHueSat},
			{"hue2", TaggedBlockKey::adjNewHueSat},
			{"blnc", TaggedBlockKey::adjColorBalance},
			{"blwh", TaggedBlockKey::adjBlackandWhite},
			{"phfl", TaggedBlockKey::adjPhotoFilter},
			{"mixr", TaggedBlockKey::adjChannelMixer},
			{"clrL", TaggedBlockKey::adjColorLookup},
			{"nvrt", TaggedBlockKey::adjInvert},
			{"post", TaggedBlockKey::adjPosterize},
			{"thrs", TaggedBlockKey::adjThreshold},
			{"grdm", TaggedBlockKey::adjGradientMap},
			{"selc", TaggedBlockKey::adjSelectiveColor},
			{"lrFX", TaggedBlockKey::fxLayer},
			{"luni", TaggedBlockKey::lrUnicodeName},
			{"lyid", TaggedBlockKey::lrId},
			{"lsct", TaggedBlockKey::lrSectionDivider},
			{"lsdk", TaggedBlockKey::lrSectionDivider},
			{"artb", TaggedBlockKey::lrArtboard},
			{"artd", TaggedBlockKey::lrArtboard},
			{"abdd", TaggedBlockKey::lrArtboard},
			{"shmd", TaggedBlockKey::lrMetaData},
			{"Anno", TaggedBlockKey::lrAnnotations},
			{"TySh", TaggedBlockKey::lrTypeTool},
			{"shpa", TaggedBlockKey::lrPatternData},
			{"lnkD", TaggedBlockKey::lrLinked},
			{"lnkE", TaggedBlockKey::lrLinked},
			{"lnk3", TaggedBlockKey::lrLinked},
			{"lnk2", TaggedBlockKey::lrLinked_8Byte},
			{"SoLd", TaggedBlockKey::lrPlacedData},
			{"SoLE", TaggedBlockKey::lrPlacedData},
			{"PlLd", TaggedBlockKey::lrPlaced},
			{"plLd", TaggedBlockKey::lrPlaced},
			{"lnk2", TaggedBlockKey::lrLinked_8Byte},
			{"cinf", TaggedBlockKey::lrCompositorUsed},
			{"Mtrn", TaggedBlockKey::lrSavingMergedTransparency},
			{"Mt16", TaggedBlockKey::lrSavingMergedTransparency},
			{"Mt32", TaggedBlockKey::lrSavingMergedTransparency},
			{"PxSD", TaggedBlockKey::lrPixelSourceData},
			{"LMsk", TaggedBlockKey::lrUserMask},
			{"Lr16", TaggedBlockKey::Lr16},
			{"Lr32", TaggedBlockKey::Lr32},
			{"Layr", TaggedBlockKey::Layr},
			{"Alph", TaggedBlockKey::Alph},
			{"FMsk", TaggedBlockKey::lrFilterMask},
			{"FXid", TaggedBlockKey::lrFilterEffects},
			{"FEid", TaggedBlockKey::lrFilterEffects},
			{"clbl", TaggedBlockKey::lrBlendClippingElements},
			{"infx", TaggedBlockKey::lrBlendInteriorElements},
			{"knko", TaggedBlockKey::lrKnockoutSetting},
			{"lspf", TaggedBlockKey::lrProtectedSetting},
			{"lclr", TaggedBlockKey::lrSheetColorSetting},
			{"fxrp", TaggedBlockKey::lrReferencePoint},
			// Vector Data for shape Layers
			{"vogk", TaggedBlockKey::vecOriginData},
			{"vmsk", TaggedBlockKey::vecMaskSettings},
			{"vstk", TaggedBlockKey::vecStrokeData},
			{"vscg", TaggedBlockKey::vecStrokeContentData},
		};
	}

	// Bidirectional mapping of Tagged block keys
	template<typename TKey, typename TValue>
	inline std::optional<TValue> getTaggedBlockKey(TKey key)
	{
		PSAPI_LOG_ERROR("getTaggedBlockKey", "No overload for the specific search type found");
		return std::nullopt;
	}

	template<>
	inline std::optional<TaggedBlockKey> getTaggedBlockKey(std::string key)
	{
		auto it = taggedBlockMap.find(key);

		if (it != taggedBlockMap.end()) {
			return std::optional<TaggedBlockKey>(it->second);
		}
		else {
			return std::optional<TaggedBlockKey>(TaggedBlockKey::Unknown);
		}
	}

	// Sometimes multiple strings can match a singular key, therefore we return a vector here
	template <>
	inline std::optional<std::vector<std::string>> getTaggedBlockKey(TaggedBlockKey key)
	{
		return findMultipleByValue(taggedBlockMap, key);
	}

	inline bool constexpr isTaggedBlockSizeUint64(TaggedBlockKey key)
	{
		if (
			key == TaggedBlockKey::lrUserMask
			|| key == TaggedBlockKey::Lr16
			|| key == TaggedBlockKey::Lr32
			|| key == TaggedBlockKey::Layr
			|| key == TaggedBlockKey::lrSavingMergedTransparency
			|| key == TaggedBlockKey::Alph
			|| key == TaggedBlockKey::lrFilterMask
			|| key == TaggedBlockKey::lrFilterEffects
			|| key == TaggedBlockKey::lrLinked_8Byte
			|| key == TaggedBlockKey::lrPixelSourceData
			|| key == TaggedBlockKey::lrCompositorUsed
			)
		{
			return true;
		}
		return false;
	}
}


// Image Data enums
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
namespace Enum
{
	enum class Compression
	{
		/// Raw bytes with no compression
		Raw,
		/// Run-length encoded data using the PackBits algorithm
		Rle,
		/// Regular ZIP compression
		Zip,
		/// ZIP compression with the difference encoded per scanline
		ZipPrediction	
	};


	namespace {
		inline std::unordered_map<uint16_t, Compression> compressionMap
		{
			{static_cast<uint16_t>(0u), Compression::Raw},
			{static_cast<uint16_t>(1u), Compression::Rle},
			{static_cast<uint16_t>(2u), Compression::Zip},
			{static_cast<uint16_t>(3u), Compression::ZipPrediction}
		};
	}

	// Bidirectional mapping of Tagged block keys
	template<typename TKey, typename TValue>
	inline std::optional<TValue> getCompression(TKey key)
	{
		PSAPI_LOG_ERROR("getCompression", "No overload for the specific search type found");
	}

	template<>
	inline std::optional<Compression> getCompression(uint16_t key)
	{
		auto it = compressionMap.find(key);

		if (it != compressionMap.end()) {
			return std::optional<Compression>(it->second);
		}
		return std::nullopt;
	}

	template <>
	inline std::optional<uint16_t> getCompression(Compression key)
	{
		return findByValue(compressionMap, key);
	}
}


// Tagged Block enums
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
namespace Enum
{
	enum class SectionDivider
	{
		Any,
		OpenFolder,
		ClosedFolder,
		BoundingSection
	};

	namespace
	{
		inline std::unordered_map<uint32_t, SectionDivider> sectionDividerMap
		{
			{static_cast<uint32_t>(0u), SectionDivider::Any},
			{static_cast<uint32_t>(1u), SectionDivider::OpenFolder},
			{static_cast<uint32_t>(2u), SectionDivider::ClosedFolder},
			{static_cast<uint32_t>(3u), SectionDivider::BoundingSection}
		};
	}

	// Bidirectional mapping of Section divider values
	template<typename TKey, typename TValue>
	inline std::optional<TValue> getSectionDivider(TKey key)
	{
		PSAPI_LOG_ERROR("getSectionDivider", "No overload for the specific search type found");
	}

	template<>
	inline std::optional<SectionDivider> getSectionDivider(const uint32_t key)
	{
		auto it = sectionDividerMap.find(key);

		if (it != sectionDividerMap.end()) {
			return std::optional<SectionDivider>(it->second);
		}
		else {
			return std::nullopt;
		}
	}

	template <>
	inline std::optional<uint32_t> getSectionDivider(const SectionDivider key)
	{
		return findByValue(sectionDividerMap, key);
	}
}


PSAPI_NAMESPACE_END