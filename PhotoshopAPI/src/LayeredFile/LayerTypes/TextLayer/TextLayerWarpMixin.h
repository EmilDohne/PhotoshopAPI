#pragma once

#include "Macros.h"
#include "TextLayerFwd.h"
#include "TextLayerEnum.h"
#include "TextLayerEngineDataUtils.h"

#include <cstdint>
#include <optional>
#include <string>

PSAPI_NAMESPACE_BEGIN

template <typename Derived>
struct TextLayerWarpMixin
{
protected:
	Derived* self() { return static_cast<Derived*>(this); }
	const Derived* self() const { return static_cast<const Derived*>(this); }

public:

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
				const auto& s = result.value();
				if (s == "warpNone")       return TextLayerEnum::WarpStyle::NoWarp;
				if (s == "warpArc")        return TextLayerEnum::WarpStyle::Arc;
				if (s == "warpArcLower")   return TextLayerEnum::WarpStyle::ArcLower;
				if (s == "warpArcUpper")   return TextLayerEnum::WarpStyle::ArcUpper;
				if (s == "warpArch")       return TextLayerEnum::WarpStyle::Arch;
				if (s == "warpBulge")      return TextLayerEnum::WarpStyle::Bulge;
				if (s == "warpShellLower") return TextLayerEnum::WarpStyle::ShellLower;
				if (s == "warpShellUpper") return TextLayerEnum::WarpStyle::ShellUpper;
				if (s == "warpFlag")       return TextLayerEnum::WarpStyle::Flag;
				if (s == "warpWave")       return TextLayerEnum::WarpStyle::Wave;
				if (s == "warpFish")       return TextLayerEnum::WarpStyle::Fish;
				if (s == "warpRise")       return TextLayerEnum::WarpStyle::Rise;
				if (s == "warpFishEye")    return TextLayerEnum::WarpStyle::FishEye;
				if (s == "warpInflate")    return TextLayerEnum::WarpStyle::Inflate;
				if (s == "warpSqueeze")    return TextLayerEnum::WarpStyle::Squeeze;
				if (s == "warpTwist")      return TextLayerEnum::WarpStyle::Twist;
				if (s == "warpCustom")     return TextLayerEnum::WarpStyle::Custom;
				// Unknown warp style string — return nullopt
				return std::nullopt;
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
				if (result.value() == "Hrzn") return TextLayerEnum::WarpRotation::Horizontal;
				if (result.value() == "Vrtc") return TextLayerEnum::WarpRotation::Vertical;
			}
		}
		return std::nullopt;
	}
};

PSAPI_NAMESPACE_END
