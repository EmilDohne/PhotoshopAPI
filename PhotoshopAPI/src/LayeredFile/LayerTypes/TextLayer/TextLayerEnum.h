#pragma once

// =========================================================================
//  TextLayerEnum.h  –  Strongly-typed enums for text-layer properties
//
//  These replace the raw int32_t values that Photoshop stores in EngineData
//  and descriptor blobs, giving both C++ and Python callers readable names
//  instead of magic numbers.
//
//  Every enum is backed by int32_t so it can be cast to/from the values
//  that EngineData actually stores.
// =========================================================================

#include "Macros.h"
#include <cstdint>
#include <optional>

PSAPI_NAMESPACE_BEGIN

namespace TextLayerEnum
{

	// -- Writing direction (orientation) ----------------------------------
	// EngineData key: EngineDict/Rendered/Shapes/WritingDirection
	enum class WritingDirection : int32_t
	{
		Horizontal = 0,
		Vertical   = 2,
	};


	// -- Shape type (point text vs box / area text) -----------------------
	// EngineData key: Children[0]/Cookie/Photoshop/ShapeType
	enum class ShapeType : int32_t
	{
		PointText = 0,
		BoxText   = 1,
	};


	// -- Warp style -------------------------------------------------------
	// TySh descriptor key: warpStyle  (string enum in the PSD format)
	// Order follows the Photoshop UI menu (Edit > Transform > Warp).
	enum class WarpStyle : int32_t
	{
		NoWarp          = 0,
		Arc             = 1,
		ArcLower        = 2,
		ArcUpper        = 3,
		Arch            = 4,
		Bulge           = 5,
		ShellLower      = 6,
		ShellUpper      = 7,
		Flag            = 8,
		Wave            = 9,
		Fish            = 10,
		Rise            = 11,
		FishEye         = 12,
		Inflate         = 13,
		Squeeze         = 14,
		Twist           = 15,
		Custom          = 16,
	};


	// -- Warp rotation ----------------------------------------------------
	// TySh descriptor key: warpRotate
	enum class WarpRotation : int32_t
	{
		Horizontal = 0,
		Vertical   = 1,
	};


	// -- Font type --------------------------------------------------------
	// EngineData key: FontType
	enum class FontType : int32_t
	{
		OpenType = 0,
		TrueType = 1,
	};


	// -- Font script family -----------------------------------------------
	// EngineData key: Script
	enum class FontScript : int32_t
	{
		Roman = 0,
		CJK   = 1,
	};


	// -- Character capitalisation -----------------------------------------
	// EngineData key: FontCaps
	enum class FontCaps : int32_t
	{
		Normal    = 0,
		SmallCaps = 1,
		AllCaps   = 2,
	};


	// -- Font baseline position -------------------------------------------
	// EngineData key: FontBaseline
	enum class FontBaseline : int32_t
	{
		Normal      = 0,
		Superscript = 1,
		Subscript   = 2,
	};


	// -- Character direction (bidi) ---------------------------------------
	// EngineData key: CharacterDirection
	enum class CharacterDirection : int32_t
	{
		Default     = 0,
		LeftToRight = 1,
		RightToLeft = 2,
	};


	// -- Baseline direction -----------------------------------------------
	// EngineData key: BaselineDirection
	enum class BaselineDirection : int32_t
	{
		Default     = 0,
		Vertical    = 1,
		CrossStream = 2,
	};


	// -- Diacritic positioning (Arabic/Hebrew) ----------------------------
	// EngineData key: DiacriticPos
	enum class DiacriticPosition : int32_t
	{
		OpenType = 0,
		Loose    = 1,
		Medium   = 2,
		Tight    = 3,
	};


	// -- Paragraph justification ------------------------------------------
	// EngineData key: Justification
	enum class Justification : int32_t
	{
		Left             = 0,
		Right            = 1,
		Center           = 2,
		JustifyLastLeft   = 3,
		JustifyLastRight  = 4,
		JustifyLastCenter = 5,
		JustifyAll       = 6,
	};


	// -- Leading type -----------------------------------------------------
	// EngineData key: LeadingType
	enum class LeadingType : int32_t
	{
		BottomToBottom = 0,
		TopToTop       = 1,
	};


	// -- Kinsoku processing order (Japanese) ------------------------------
	// EngineData key: KinsokuOrder
	enum class KinsokuOrder : int32_t
	{
		PushInFirst  = 0,
		PushOutFirst = 1,
	};


	// -- Anti-aliasing method ---------------------------------------------
	// TySh descriptor key: "AntA" (enum type "Annt")
	// These correspond to the Photoshop anti-aliasing options in the
	// character panel / options bar.
	//
	// Discovered via Photoshop 2026 probing:
	//   charID "Anno" = antiAliasNone     (None)
	//   charID "AnCr" = antiAliasCrisp    (Crisp)   <-- PS default
	//   charID "AnSt" = antiAliasStrong   (Strong)
	//   charID "AnSm" = antiAliasSmooth   (Smooth)
	//         (none)  = antiAliasSharp    (Sharp – NO 4-byte charID, stringID only)
	//
	// Note: Photoshop's internal charID table also contains AnLo/AnMd/AnHi
	// (Low/Medium/High) but these are not accessible from the UI or DOM and
	// are not exposed here. The reader returns nullopt for those values.
	enum class AntiAliasMethod : int32_t
	{
		None   = 0,   // charID "Anno"  / stringID "antiAliasNone"
		Crisp  = 1,   // charID "AnCr"  / stringID "antiAliasCrisp"
		Strong = 2,   // charID "AnSt"  / stringID "antiAliasStrong"
		Smooth = 3,   // charID "AnSm"  / stringID "antiAliasSmooth"
		Sharp  = 4,   // stringID "antiAliasSharp" (no charID)
	};

	// -- Utility: convert optional<int32_t> to optional<Enum> ----------------
	template <typename E>
	std::optional<E> opt_enum(std::optional<int32_t> v)
	{
		if (!v.has_value()) return std::nullopt;
		return static_cast<E>(*v);
	}

} // namespace TextLayerEnum

PSAPI_NAMESPACE_END
