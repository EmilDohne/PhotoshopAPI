#pragma once

#include "Macros.h"
#include "TextLayerFwd.h"
#include "TextLayerEnum.h"
#include "TextLayerEngineDataUtils.h"

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>

PSAPI_NAMESPACE_BEGIN

template <typename Derived>
struct TextLayerWarpMixin
{
protected:
	Derived* self() { return static_cast<Derived*>(this); }
	const Derived* self() const { return static_cast<const Derived*>(this); }

public:

	static std::optional<TextLayerEnum::WarpStyle> warp_style_from_ps(const std::string& value)
	{
		if (value == "warpNone")       return TextLayerEnum::WarpStyle::NoWarp;
		if (value == "warpArc")        return TextLayerEnum::WarpStyle::Arc;
		if (value == "warpArcLower")   return TextLayerEnum::WarpStyle::ArcLower;
		if (value == "warpArcUpper")   return TextLayerEnum::WarpStyle::ArcUpper;
		if (value == "warpArch")       return TextLayerEnum::WarpStyle::Arch;
		if (value == "warpBulge")      return TextLayerEnum::WarpStyle::Bulge;
		if (value == "warpShellLower") return TextLayerEnum::WarpStyle::ShellLower;
		if (value == "warpShellUpper") return TextLayerEnum::WarpStyle::ShellUpper;
		if (value == "warpFlag")       return TextLayerEnum::WarpStyle::Flag;
		if (value == "warpWave")       return TextLayerEnum::WarpStyle::Wave;
		if (value == "warpFish")       return TextLayerEnum::WarpStyle::Fish;
		if (value == "warpRise")       return TextLayerEnum::WarpStyle::Rise;
		if (value == "warpFishEye")    return TextLayerEnum::WarpStyle::FishEye;
		if (value == "warpInflate")    return TextLayerEnum::WarpStyle::Inflate;
		if (value == "warpSqueeze")    return TextLayerEnum::WarpStyle::Squeeze;
		if (value == "warpTwist")      return TextLayerEnum::WarpStyle::Twist;
		if (value == "warpCustom")     return TextLayerEnum::WarpStyle::Custom;
		return std::nullopt;
	}

	static std::optional<std::string> warp_style_to_ps(const TextLayerEnum::WarpStyle style)
	{
		switch (style)
		{
			case TextLayerEnum::WarpStyle::NoWarp:     return "warpNone";
			case TextLayerEnum::WarpStyle::Arc:        return "warpArc";
			case TextLayerEnum::WarpStyle::ArcLower:   return "warpArcLower";
			case TextLayerEnum::WarpStyle::ArcUpper:   return "warpArcUpper";
			case TextLayerEnum::WarpStyle::Arch:       return "warpArch";
			case TextLayerEnum::WarpStyle::Bulge:      return "warpBulge";
			case TextLayerEnum::WarpStyle::ShellLower: return "warpShellLower";
			case TextLayerEnum::WarpStyle::ShellUpper: return "warpShellUpper";
			case TextLayerEnum::WarpStyle::Flag:       return "warpFlag";
			case TextLayerEnum::WarpStyle::Wave:       return "warpWave";
			case TextLayerEnum::WarpStyle::Fish:       return "warpFish";
			case TextLayerEnum::WarpStyle::Rise:       return "warpRise";
			case TextLayerEnum::WarpStyle::FishEye:    return "warpFishEye";
			case TextLayerEnum::WarpStyle::Inflate:    return "warpInflate";
			case TextLayerEnum::WarpStyle::Squeeze:    return "warpSqueeze";
			case TextLayerEnum::WarpStyle::Twist:      return "warpTwist";
			case TextLayerEnum::WarpStyle::Custom:     return "warpCustom";
			default: return std::nullopt;
		}
	}

	static std::optional<TextLayerEnum::WarpRotation> warp_rotation_from_ps(const std::string& value)
	{
		if (value == "Hrzn") return TextLayerEnum::WarpRotation::Horizontal;
		if (value == "Vrtc") return TextLayerEnum::WarpRotation::Vertical;
		return std::nullopt;
	}

	static std::optional<std::string> warp_rotation_to_ps(const TextLayerEnum::WarpRotation rotation)
	{
		switch (rotation)
		{
			case TextLayerEnum::WarpRotation::Horizontal: return "Hrzn";
			case TextLayerEnum::WarpRotation::Vertical:   return "Vrtc";
			default: return std::nullopt;
		}
	}

	bool has_warp() const
	{
		const auto style = warp_style();
		return style.has_value() && style.value() != TextLayerEnum::WarpStyle::NoWarp;
	}

	std::optional<TextLayerEnum::WarpStyle> warp_style() const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			auto result = TextLayerDetail::read_warp_enum(*block, "warpStyle");
			if (result.has_value())
			{
				return warp_style_from_ps(result.value());
			}
		}
		return std::nullopt;
	}

	std::optional<double> warp_value() const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			auto result = TextLayerDetail::read_warp_double(*block, "warpValue");
			if (result.has_value()) return result;
		}
		return std::nullopt;
	}

	std::optional<double> warp_horizontal_distortion() const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			auto result = TextLayerDetail::read_warp_double(*block, "warpPerspective");
			if (result.has_value()) return result;
		}
		return std::nullopt;
	}

	std::optional<double> warp_vertical_distortion() const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			auto result = TextLayerDetail::read_warp_double(*block, "warpPerspectiveOther");
			if (result.has_value()) return result;
		}
		return std::nullopt;
	}

	std::optional<TextLayerEnum::WarpRotation> warp_rotation() const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			auto result = TextLayerDetail::read_warp_enum(*block, "warpRotate");
			if (result.has_value())
			{
				return warp_rotation_from_ps(result.value());
			}
		}
		return std::nullopt;
	}

	void set_warp_style(const TextLayerEnum::WarpStyle style)
	{
		const auto warp_style_code = warp_style_to_ps(style);
		if (!warp_style_code.has_value())
		{
			throw std::invalid_argument("TextLayer::set_warp_style() received an unsupported WarpStyle enum value");
		}

		auto blocks = self()->text_tagged_blocks();
		if (blocks.empty())
		{
			throw std::runtime_error("TextLayer::set_warp_style() failed: no lrTypeTool tagged block found on this layer");
		}

		size_t modified = 0u;
		for (const auto& block : blocks)
		{
			if (TextLayerDetail::write_warp_enum(*block, "warpStyle", warp_style_code.value()))
			{
				++modified;
			}
		}
		if (modified == 0u)
		{
			throw std::runtime_error("TextLayer::set_warp_style() failed: unable to write warpStyle in TySh warp descriptor");
		}
	}

	void set_warp_value(const double value)
	{
		auto blocks = self()->text_tagged_blocks();
		if (blocks.empty())
		{
			throw std::runtime_error("TextLayer::set_warp_value() failed: no lrTypeTool tagged block found on this layer");
		}

		size_t modified = 0u;
		for (const auto& block : blocks)
		{
			if (TextLayerDetail::write_warp_double(*block, "warpValue", value))
			{
				++modified;
			}
		}
		if (modified == 0u)
		{
			throw std::runtime_error("TextLayer::set_warp_value() failed: unable to write warpValue in TySh warp descriptor");
		}
	}

	void set_warp_horizontal_distortion(const double value)
	{
		auto blocks = self()->text_tagged_blocks();
		if (blocks.empty())
		{
			throw std::runtime_error("TextLayer::set_warp_horizontal_distortion() failed: no lrTypeTool tagged block found on this layer");
		}

		size_t modified = 0u;
		for (const auto& block : blocks)
		{
			if (TextLayerDetail::write_warp_double(*block, "warpPerspective", value))
			{
				++modified;
			}
		}
		if (modified == 0u)
		{
			throw std::runtime_error("TextLayer::set_warp_horizontal_distortion() failed: unable to write warpPerspective in TySh warp descriptor");
		}
	}

	void set_warp_vertical_distortion(const double value)
	{
		auto blocks = self()->text_tagged_blocks();
		if (blocks.empty())
		{
			throw std::runtime_error("TextLayer::set_warp_vertical_distortion() failed: no lrTypeTool tagged block found on this layer");
		}

		size_t modified = 0u;
		for (const auto& block : blocks)
		{
			if (TextLayerDetail::write_warp_double(*block, "warpPerspectiveOther", value))
			{
				++modified;
			}
		}
		if (modified == 0u)
		{
			throw std::runtime_error("TextLayer::set_warp_vertical_distortion() failed: unable to write warpPerspectiveOther in TySh warp descriptor");
		}
	}

	void set_warp_rotation(const TextLayerEnum::WarpRotation rotation)
	{
		const auto warp_rotation_code = warp_rotation_to_ps(rotation);
		if (!warp_rotation_code.has_value())
		{
			throw std::invalid_argument("TextLayer::set_warp_rotation() received an unsupported WarpRotation enum value");
		}

		auto blocks = self()->text_tagged_blocks();
		if (blocks.empty())
		{
			throw std::runtime_error("TextLayer::set_warp_rotation() failed: no lrTypeTool tagged block found on this layer");
		}

		size_t modified = 0u;
		for (const auto& block : blocks)
		{
			if (TextLayerDetail::write_warp_enum(*block, "warpRotate", warp_rotation_code.value()))
			{
				++modified;
			}
		}
		if (modified == 0u)
		{
			throw std::runtime_error("TextLayer::set_warp_rotation() failed: unable to write warpRotate in TySh warp descriptor");
		}
	}
};

PSAPI_NAMESPACE_END