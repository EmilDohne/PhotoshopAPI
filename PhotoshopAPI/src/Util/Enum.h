#pragma once

#include "../Macros.h"
#include "Logger.h"

#include <unordered_map>
#include <optional>

PSAPI_NAMESPACE_BEGIN

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
}

// Header Enums
namespace Enum
{
	// File Format Version
	enum class Version
	{
		Psd = 1,
		Psb = 2
	};

	inline std::unordered_map<uint16_t, Version> versionMap
	{
		{1, Version::Psd},
		{2, Version::Psb}
	};

	// Bit Depth of the Document
	enum class BitDepth
	{
		BD_1 = 1,
		BD_8 = 8,
		BD_16 = 16,
		BD_32 = 32
	};

	inline std::unordered_map<uint16_t, BitDepth> bitDepthMap
	{
		{1, BitDepth::BD_1},
		{8, BitDepth::BD_8},
		{16, BitDepth::BD_16},
		{32, BitDepth::BD_32},
	};

	enum class ColorMode
	{
		Bitmap = 0,
		Grayscale = 1,
		Indexed = 2,
		RGB = 3,
		CMYK = 4,
		Multichannel = 7,
		Duotone = 8,
		Lab = 9
	};

	inline std::unordered_map<uint16_t, ColorMode> colorModeMap
	{
		{0, ColorMode::Bitmap},
		{1, ColorMode::Grayscale},
		{2, ColorMode::Indexed},
		{3, ColorMode::RGB},
		{4, ColorMode::CMYK},
		{7, ColorMode::Multichannel},
		{8, ColorMode::Duotone},
		{9, ColorMode::Lab}
	};
}

// Image Resource Enums
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


	inline ImageResource intToImageResource(const uint16_t value)
	{
		switch (value)
		{
		case 1005: return ImageResource::ResolutionInfo;
		case 1006: return ImageResource::AlphaChannelNames;
		case 1010: return ImageResource::BackgroundColor;
		case 1011: return ImageResource::PrintFlags;
		case 1013: return ImageResource::ColorHalftoningInfo;
		case 1016: return ImageResource::ColorTransferFunctions;
		case 1024: return ImageResource::LayerStateInformation;
		case 1026: return ImageResource::LayerGroupInformation;
		case 1028: return ImageResource::IPTCRecord;
		case 1032: return ImageResource::GridAndGuidesInformation;
		case 1036: return ImageResource::ThumbnailResource;
		case 1037: return ImageResource::GlobalAngle;
		case 1041: return ImageResource::ICCUntaggedProfile;
		case 1043: return ImageResource::SpotHalftone;
		case 1044: return ImageResource::IDSeed;
		case 1045: return ImageResource::UnicodeAlphaNames;
		case 1049: return ImageResource::GlobalAltitude;
		case 1050: return ImageResource::Slices;
		case 1053: return ImageResource::AlphaIdentifiers;
		case 1054: return ImageResource::URLList;
		case 1057: return ImageResource::VersionInfo;
		case 1058: return ImageResource::ExifData1;
		case 1060: return ImageResource::XMPMetadata;
		case 1061: return ImageResource::CaptionDigest;
		case 1062: return ImageResource::PrintScale;
		case 1064: return ImageResource::PixelAspectRatio;
		case 1067: return ImageResource::AlternateSpotColors;
		case 1069: return ImageResource::LayerSelectionID;
		case 1072: return ImageResource::LayerGroupEnabledID;
		case 1077: return ImageResource::DisplayInfo;
		case 1082: return ImageResource::PrintInformation;
		case 1083: return ImageResource::PrintStyle;
		case 10000: return ImageResource::PrintFlagsInfo;
		default: return ImageResource::NotImplemented;
		}
	}
}

// Layer and Mask Information Enums
namespace Enum
{
	enum class ChannelID
	{
		Red,
		Green,
		Blue,
		Custom,
		TransparencyMask,
		UserSuppliedLayerMask,
		RealUserSuppliedLayerMask
	};

	inline ChannelID intToChannelID(const int16_t value)
	{
		switch (value)
		{
		case 0: return ChannelID::Red;
		case 1: return ChannelID::Green;
		case 2: return ChannelID::Blue;
		case -1: return ChannelID::TransparencyMask;
		case -2: return ChannelID::UserSuppliedLayerMask;
		case -3: return ChannelID::RealUserSuppliedLayerMask;
		default: return ChannelID::Custom;	// These are channels set by the user
		}
	}

	enum class BlendMode
	{
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
		PSAPI_LOG_ERROR("getBlendMode", "No overload for the specific search type found")
	}

	template<>
	inline std::optional<BlendMode> getBlendMode(std::string key)
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
	inline std::optional<std::string> getBlendMode(BlendMode key)
	{
		return findByValue(blendModeMap, key);
	}
}



PSAPI_NAMESPACE_END