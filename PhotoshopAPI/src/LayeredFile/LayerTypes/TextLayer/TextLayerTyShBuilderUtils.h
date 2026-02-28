#pragma once

// =========================================================================
//  TextLayerTyShBuilderUtils.h
//
//  Build EngineData, text descriptor, warp descriptor, and complete TySh
//  tagged blocks for TextLayer construction.
// =========================================================================

#include "Macros.h"
#include "TextLayerDescriptorUtils.h"
#include "TextLayerEngineDataUtils.h"
#include "TextLayerU16Utils.h"

#include "Core/Endian/EndianByteSwap.h"
#include "Core/Struct/DescriptorStructure.h"
#include "Core/Struct/EngineDataStructure.h"
#include "Core/Struct/UnicodeString.h"
#include "Core/TaggedBlocks/TaggedBlock.h"
#include "Core/TaggedBlocks/TypeToolTaggedBlock.h"
#include "Util/Enum.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

PSAPI_NAMESPACE_BEGIN

namespace TextLayerDetail
{
template <typename T>
inline void push_be_encoded(std::vector<std::byte>& buf, T value)
{
	const T encoded = endian_encode_be<T>(value);
	const auto* bytes = reinterpret_cast<const std::byte*>(&encoded);
	buf.insert(buf.end(), bytes, bytes + sizeof(T));
}

inline void push_u16_be(std::vector<std::byte>& buf, uint16_t v)
{
	push_be_encoded<uint16_t>(buf, v);
}

inline void push_double_be(std::vector<std::byte>& buf, double v)
{
	push_be_encoded<double>(buf, v);
}

inline void push_u32_be(std::vector<std::byte>& buf, uint32_t v)
{
	push_be_encoded<uint32_t>(buf, v);
}

inline std::vector<std::byte> serialize_descriptor_with_version(const Descriptors::Descriptor& descriptor, uint32_t version)
{
	std::vector<std::byte> bytes{};
	bytes.reserve(4u);
	push_u32_be(bytes, version);
	const auto body = serialize_descriptor_body(descriptor);
	bytes.insert(bytes.end(), body.begin(), body.end());
	return bytes;
}

//  Build minimal EngineData blob
// -----------------------------------------------------------------------

/// Create a float Value that always serializes with a decimal point (e.g. "0.0" not "0").
inline EngineData::Value make_float(double v)
{
	auto val = EngineData::make_number(v);
	val.is_integer = false;
	return val;
}

/// Build a comprehensive EngineData tree matching Photoshop's expected format.
/// Returns serialized EngineData bytes.
inline std::vector<std::byte> build_engine_data(
	const std::string& text_utf8,
	const std::string& font_postscript_name,
	double font_size,
	const std::vector<double>& fill_color = { 1.0, 0.0, 0.0, 0.0 },  // AGBR: opaque black
	double box_width  = 360.0,
	double box_height = 72.0)
{
	using namespace EngineData;

	auto utf16 = UnicodeString::convertUTF8ToUTF16LE(text_utf8);
	// Photoshop uses \r (0x0D) for line breaks, not \n (0x0A)
	for (auto& ch : utf16)
		if (ch == u'\n') ch = u'\r';
	// EngineData /Text includes a trailing \r
	std::u16string text_with_cr = utf16;
	text_with_cr.push_back(static_cast<char16_t>('\r'));

	// Encode as UTF-16BE with BOM for the literal string.
	// Escape PostScript-special bytes ( ) \ so the parser handles them.
	auto text_be = encode_utf16be_bytes(text_with_cr, true);
	std::string text_literal;
	text_literal.reserve(text_be.size() + 8u);
	for (auto b : text_be)
	{
		const auto u = to_u8(b);
		if (u == 0x28u || u == 0x29u || u == 0x5Cu)
			text_literal.push_back('\\');
		text_literal.push_back(static_cast<char>(u));
	}

	// Text byte count: text_utf16 length + 1 (\r) = total code units in EngineData
	const int32_t text_run_length = static_cast<int32_t>(text_with_cr.size());

	// Compute leading from font size
	const double leading = std::round(font_size * 1.2 * 10.0) / 10.0;

	// Font name in BOM+UTF-16BE form for EngineData LiteralString
	auto font_name_encoded = encode_engine_literal_utf16be(font_postscript_name);
	auto sentinel_name = encode_engine_literal_utf16be("AdobeInvisFont");

	// Build the tree
	auto root = make_dict();

	// ====================================================================
	// EngineDict
	// ====================================================================
	auto engine_dict = make_dict();

	// ---------- Editor ----------
	auto editor = make_dict();
	insert_dict_value(editor, "Text", make_string(text_literal));
	insert_dict_value(engine_dict, "Editor", std::move(editor));

	// ---------- ParagraphRun ----------
	auto para_run = make_dict();

	// DefaultRunData
	auto para_default_rd = make_dict();
	{
		auto ps = make_dict();
		insert_dict_value(ps, "DefaultStyleSheet", make_integer(0));
		insert_dict_value(ps, "Properties", make_dict());
		insert_dict_value(para_default_rd, "ParagraphSheet", std::move(ps));
		auto adj = make_dict();
		auto axis = make_array();
		append_array_item(axis, make_float(1.0));
		append_array_item(axis, make_float(0.0));
		append_array_item(axis, make_float(1.0));
		insert_dict_value(adj, "Axis", std::move(axis));
		auto xy = make_array();
		append_array_item(xy, make_float(0.0));
		append_array_item(xy, make_float(0.0));
		insert_dict_value(adj, "XY", std::move(xy));
		insert_dict_value(para_default_rd, "Adjustments", std::move(adj));
	}
	insert_dict_value(para_run, "DefaultRunData", std::move(para_default_rd));

	// RunArray
	auto para_run_array = make_array();
	{
		auto entry = make_dict();
		auto ps = make_dict();
		insert_dict_value(ps, "DefaultStyleSheet", make_integer(0));
		auto props = make_dict();
		insert_dict_value(props, "Justification", make_integer(0));
		insert_dict_value(props, "FirstLineIndent", make_float(0.0));
		insert_dict_value(props, "StartIndent", make_float(0.0));
		insert_dict_value(props, "EndIndent", make_float(0.0));
		insert_dict_value(props, "SpaceBefore", make_float(0.0));
		insert_dict_value(props, "SpaceAfter", make_float(0.0));
		insert_dict_value(props, "AutoHyphenate", make_bool(false));
		insert_dict_value(props, "HyphenatedWordSize", make_integer(6));
		insert_dict_value(props, "PreHyphen", make_integer(2));
		insert_dict_value(props, "PostHyphen", make_integer(2));
		insert_dict_value(props, "ConsecutiveHyphens", make_integer(8));
		insert_dict_value(props, "Zone", make_float(36.0));
		auto ws = make_array();
		append_array_item(ws, make_float(.8));
		append_array_item(ws, make_float(1.0));
		append_array_item(ws, make_float(1.33));
		insert_dict_value(props, "WordSpacing", std::move(ws));
		auto ls = make_array();
		append_array_item(ls, make_float(0.0));
		append_array_item(ls, make_float(0.0));
		append_array_item(ls, make_float(0.0));
		insert_dict_value(props, "LetterSpacing", std::move(ls));
		auto gs = make_array();
		append_array_item(gs, make_float(1.0));
		append_array_item(gs, make_float(1.0));
		append_array_item(gs, make_float(1.0));
		insert_dict_value(props, "GlyphSpacing", std::move(gs));
		insert_dict_value(props, "AutoLeading", make_float(1.2));
		insert_dict_value(props, "LeadingType", make_integer(0));
		insert_dict_value(props, "Hanging", make_bool(false));
		insert_dict_value(props, "Burasagari", make_bool(false));
		insert_dict_value(props, "KinsokuOrder", make_integer(0));
		insert_dict_value(props, "EveryLineComposer", make_bool(false));
		insert_dict_value(ps, "Properties", std::move(props));
		insert_dict_value(entry, "ParagraphSheet", std::move(ps));
		auto adj = make_dict();
		auto axis = make_array();
		append_array_item(axis, make_float(1.0));
		append_array_item(axis, make_float(0.0));
		append_array_item(axis, make_float(1.0));
		insert_dict_value(adj, "Axis", std::move(axis));
		auto xy = make_array();
		append_array_item(xy, make_float(0.0));
		append_array_item(xy, make_float(0.0));
		insert_dict_value(adj, "XY", std::move(xy));
		insert_dict_value(entry, "Adjustments", std::move(adj));
		append_array_item(para_run_array, std::move(entry));
	}
	insert_dict_value(para_run, "RunArray", std::move(para_run_array));

	auto para_run_lengths = make_array();
	append_array_item(para_run_lengths, make_integer(text_run_length));
	insert_dict_value(para_run, "RunLengthArray", std::move(para_run_lengths));
	insert_dict_value(para_run, "IsJoinable", make_integer(1));
	insert_dict_value(engine_dict, "ParagraphRun", std::move(para_run));

	// ---------- StyleRun ----------
	auto style_run = make_dict();

	// DefaultRunData - must carry the base font/size so that Photoshop
	// shows the correct font size when the cursor sits at the end of
	// the text or on any unstyled insertion point.
	auto style_default_rd = make_dict();
	{
		auto ss = make_dict();
		auto ssd = make_dict();
		insert_dict_value(ssd, "Font", make_integer(0));
		insert_dict_value(ssd, "FontSize", make_float(font_size));
		insert_dict_value(ssd, "FauxBold", make_bool(false));
		insert_dict_value(ssd, "FauxItalic", make_bool(false));
		insert_dict_value(ssd, "AutoLeading", make_bool(true));
		insert_dict_value(ssd, "Leading", make_float(font_size * 1.2));
		insert_dict_value(ssd, "HorizontalScale", make_integer(1));
		insert_dict_value(ssd, "VerticalScale", make_integer(1));
		insert_dict_value(ssd, "Tracking", make_integer(0));
		insert_dict_value(ssd, "AutoKerning", make_bool(true));
		insert_dict_value(ssd, "BaselineDirection", make_integer(1));
		// FillColor: same as fill_color parameter
		auto fc = make_dict();
		insert_dict_value(fc, "Type", make_integer(1));
		auto fv = make_array();
		for (auto v : fill_color) append_array_item(fv, make_float(v));
		insert_dict_value(fc, "Values", std::move(fv));
		insert_dict_value(ssd, "FillColor", std::move(fc));
		insert_dict_value(ss, "StyleSheetData", std::move(ssd));
		insert_dict_value(style_default_rd, "StyleSheet", std::move(ss));
	}
	insert_dict_value(style_run, "DefaultRunData", std::move(style_default_rd));

	// RunArray
	auto style_run_array = make_array();
	{
		auto srd = make_dict();
		auto ss = make_dict();
		auto ssd = make_dict();
		insert_dict_value(ssd, "Font", make_integer(0));
		insert_dict_value(ssd, "FontSize", make_float(font_size));
		insert_dict_value(ssd, "FauxBold", make_bool(false));
		insert_dict_value(ssd, "FauxItalic", make_bool(false));
		insert_dict_value(ssd, "AutoLeading", make_bool(true));
		insert_dict_value(ssd, "Leading", make_float(leading));
		insert_dict_value(ssd, "HorizontalScale", make_float(1.0));
		insert_dict_value(ssd, "VerticalScale", make_float(1.0));
		insert_dict_value(ssd, "Tracking", make_integer(0));
		insert_dict_value(ssd, "AutoKerning", make_bool(true));
		insert_dict_value(ssd, "Kerning", make_integer(0));
		insert_dict_value(ssd, "BaselineShift", make_float(0.0));
		insert_dict_value(ssd, "FontCaps", make_integer(0));
		insert_dict_value(ssd, "FontBaseline", make_integer(0));
		insert_dict_value(ssd, "Underline", make_bool(false));
		insert_dict_value(ssd, "Strikethrough", make_bool(false));
		insert_dict_value(ssd, "Ligatures", make_bool(true));
		insert_dict_value(ssd, "DLigatures", make_bool(false));
		insert_dict_value(ssd, "BaselineDirection", make_integer(1));
		insert_dict_value(ssd, "Tsume", make_float(0.0));
		insert_dict_value(ssd, "StyleRunAlignment", make_integer(2));
		insert_dict_value(ssd, "Language", make_integer(14));
		insert_dict_value(ssd, "NoBreak", make_bool(false));
		// FillColor
		auto fc = make_dict();
		insert_dict_value(fc, "Type", make_integer(1));
		auto fv = make_array();
		for (double v : fill_color) append_array_item(fv, make_float(v));
		insert_dict_value(fc, "Values", std::move(fv));
		insert_dict_value(ssd, "FillColor", std::move(fc));
		// StrokeColor
		auto sc = make_dict();
		insert_dict_value(sc, "Type", make_integer(1));
		auto sv = make_array();
		append_array_item(sv, make_float(1.0));
		append_array_item(sv, make_float(0.0));
		append_array_item(sv, make_float(0.0));
		append_array_item(sv, make_float(0.0));
		insert_dict_value(sc, "Values", std::move(sv));
		insert_dict_value(ssd, "StrokeColor", std::move(sc));
		insert_dict_value(ssd, "FillFlag", make_bool(true));
		insert_dict_value(ssd, "StrokeFlag", make_bool(false));
		insert_dict_value(ssd, "FillFirst", make_bool(false));
		insert_dict_value(ssd, "YUnderline", make_integer(1));
		insert_dict_value(ssd, "OutlineWidth", make_float(1.0));
		insert_dict_value(ssd, "HindiNumbers", make_bool(false));
		insert_dict_value(ssd, "Kashida", make_integer(1));

		insert_dict_value(ss, "StyleSheetData", std::move(ssd));
		insert_dict_value(srd, "StyleSheet", std::move(ss));
		append_array_item(style_run_array, std::move(srd));
	}
	insert_dict_value(style_run, "RunArray", std::move(style_run_array));

	auto style_run_lengths = make_array();
	append_array_item(style_run_lengths, make_integer(text_run_length));
	insert_dict_value(style_run, "RunLengthArray", std::move(style_run_lengths));
	insert_dict_value(style_run, "IsJoinable", make_integer(2));
	insert_dict_value(engine_dict, "StyleRun", std::move(style_run));

	// ---------- GridInfo ----------
	auto grid_info = make_dict();
	insert_dict_value(grid_info, "GridIsOn", make_bool(false));
	insert_dict_value(grid_info, "ShowGrid", make_bool(false));
	insert_dict_value(grid_info, "GridSize", make_float(18.0));
	insert_dict_value(grid_info, "GridLeading", make_float(22.0));
	{
		auto gc = make_dict();
		insert_dict_value(gc, "Type", make_integer(1));
		auto gv = make_array();
		append_array_item(gv, make_float(0.0));
		append_array_item(gv, make_float(0.0));
		append_array_item(gv, make_float(0.0));
		append_array_item(gv, make_float(1.0));
		insert_dict_value(gc, "Values", std::move(gv));
		insert_dict_value(grid_info, "GridColor", std::move(gc));
	}
	{
		auto gc2 = make_dict();
		insert_dict_value(gc2, "Type", make_integer(1));
		auto gv2 = make_array();
		append_array_item(gv2, make_float(0.0));
		append_array_item(gv2, make_float(0.0));
		append_array_item(gv2, make_float(0.0));
		append_array_item(gv2, make_float(1.0));
		insert_dict_value(gc2, "Values", std::move(gv2));
		insert_dict_value(grid_info, "GridLeadingFillColor", std::move(gc2));
	}
	insert_dict_value(grid_info, "AlignLineHeightToGridFlags", make_bool(false));
	insert_dict_value(engine_dict, "GridInfo", std::move(grid_info));

	// ---------- AntiAlias & UseFractionalGlyphWidths ----------
	insert_dict_value(engine_dict, "AntiAlias", make_integer(1));
	insert_dict_value(engine_dict, "UseFractionalGlyphWidths", make_bool(true));

	// ---------- Rendered ----------
	auto rendered = make_dict();
	insert_dict_value(rendered, "Version", make_integer(1));
	auto shapes = make_dict();
	insert_dict_value(shapes, "WritingDirection", make_integer(0));
	auto children = make_array();
	{
		auto child = make_dict();
		insert_dict_value(child, "ShapeType", make_integer(1));  // box text
		insert_dict_value(child, "Procession", make_integer(0));
		auto lines_dict = make_dict();
		insert_dict_value(lines_dict, "WritingDirection", make_integer(0));
		insert_dict_value(lines_dict, "Children", make_array());
		insert_dict_value(child, "Lines", std::move(lines_dict));
		// Cookie with Photoshop sub-dict
		auto cookie = make_dict();
		auto ps_cookie = make_dict();
		insert_dict_value(ps_cookie, "ShapeType", make_integer(1));
		auto bb = make_array();
		append_array_item(bb, make_float(0.0));        // top
		append_array_item(bb, make_float(0.0));        // left
		append_array_item(bb, make_float(box_width));  // right
		append_array_item(bb, make_float(box_height)); // bottom
		insert_dict_value(ps_cookie, "BoxBounds", std::move(bb));
		auto base = make_dict();
		insert_dict_value(base, "ShapeType", make_integer(1));
		auto tp0 = make_array();
		append_array_item(tp0, make_float(1.0));
		append_array_item(tp0, make_float(0.0));
		insert_dict_value(base, "TransformPoint0", std::move(tp0));
		auto tp1 = make_array();
		append_array_item(tp1, make_float(0.0));
		append_array_item(tp1, make_float(1.0));
		insert_dict_value(base, "TransformPoint1", std::move(tp1));
		auto tp2 = make_array();
		append_array_item(tp2, make_float(0.0));
		append_array_item(tp2, make_float(0.0));
		insert_dict_value(base, "TransformPoint2", std::move(tp2));
		insert_dict_value(ps_cookie, "Base", std::move(base));
		insert_dict_value(cookie, "Photoshop", std::move(ps_cookie));
		insert_dict_value(child, "Cookie", std::move(cookie));
		append_array_item(children, std::move(child));
	}
	insert_dict_value(shapes, "Children", std::move(children));
	insert_dict_value(rendered, "Shapes", std::move(shapes));
	insert_dict_value(engine_dict, "Rendered", std::move(rendered));

	insert_dict_value(root, "EngineDict", std::move(engine_dict));

	// ====================================================================
	// ResourceDict
	// ====================================================================
	auto resource_dict = make_dict();

	// ---------- KinsokuSet ----------
	// Helper: build a UTF-16BE literal string directly from Unicode code points
	// (avoids source-encoding issues with CJK characters)
	auto make_u16str = [](const std::initializer_list<char16_t>& cps) -> std::string {
		std::string result;
		result.push_back(static_cast<char>(0xFE));
		result.push_back(static_cast<char>(0xFF));
		for (char16_t cp : cps) {
			const auto hi = static_cast<char>((cp >> 8) & 0xFF);
			const auto lo = static_cast<char>(cp & 0xFF);
			auto push_esc = [&](char b) {
				auto u = static_cast<uint8_t>(b);
				if (u == 0x28u || u == 0x29u || u == 0x5Cu) result.push_back('\\');
				result.push_back(b);
			};
			push_esc(hi);
			push_esc(lo);
		}
		return result;
	};

	// PhotoshopKinsokuHard NoStart set encoded via code points below.
	const std::string hard_no_start = make_u16str({
		0x3001,0x3002,0xFF0C,0xFF0E,0x30FB,0xFF1A,0xFF1B,0xFF1F,0xFF01,
		0x30FC,0x2015,0x2019,0x201D,0xFF09,0x3015,0xFF3D,0xFF5D,0x3009,
		0x300B,0x300D,0x300F,0x3011,0x30FD,0x30FE,0x309D,0x309E,0x3005,
		0x3041,0x3043,0x3045,0x3047,0x3049,0x3063,0x3083,0x3085,0x3087,
		0x308E,0x30A1,0x30A3,0x30A5,0x30A7,0x30A9,0x30C3,0x30E3,0x30E5,
		0x30E7,0x30EE,0x30F5,0x30F6,0x309B,0x309C,
		0x003F,0x0021,0x0029,0x005D,0x007D,0x002C,0x002E,0x003A,0x003B,
		0x2103,0x2109,0x00A2,0xFF05,0x2030});
	// PhotoshopKinsokuHard NoEnd set encoded via code points below.
	const std::string hard_no_end = make_u16str({
		0x2018,0x201C,0xFF08,0x3014,0xFF3B,0xFF5B,0x3008,0x300A,0x300C,
		0x300E,0x3010,
		0x0028,0x005B,0x007B,0xFFE5,0xFF04,0x00A3,0xFF20,0x00A7,0x3012,0xFF03});
	const std::string hard_keep = make_u16str({0x2015,0x2025});       // horizontal bar + two-dot leader
	const std::string hard_hang = make_u16str({0x3001,0x3002,0x002E,0x002C}); // ideographic comma/period + ASCII .,
	// PhotoshopKinsokuSoft
	const std::string soft_no_start = make_u16str({
		0x3001,0x3002,0xFF0C,0xFF0E,0x30FB,0xFF1A,0xFF1B,0xFF1F,0xFF01,
		0x2019,0x201D,0xFF09,0x3015,0xFF3D,0xFF5D,0x3009,
		0x300B,0x300D,0x300F,0x3011,0x30FD,0x30FE,0x309D,0x309E,0x3005});
	const std::string soft_no_end = make_u16str({
		0x2018,0x201C,0xFF08,0x3014,0xFF3B,0xFF5B,0x3008,0x300A,0x300C,
		0x300E,0x3010});

	auto kinsoku_set = make_array();
	{
		auto k0 = make_dict();
		insert_dict_value(k0, "Name", make_string(encode_engine_literal_utf16be("PhotoshopKinsokuHard")));
		insert_dict_value(k0, "NoStart", make_string(hard_no_start));
		insert_dict_value(k0, "NoEnd", make_string(hard_no_end));
		insert_dict_value(k0, "Keep", make_string(hard_keep));
		insert_dict_value(k0, "Hanging", make_string(hard_hang));
		append_array_item(kinsoku_set, std::move(k0));
		auto k1 = make_dict();
		insert_dict_value(k1, "Name", make_string(encode_engine_literal_utf16be("PhotoshopKinsokuSoft")));
		insert_dict_value(k1, "NoStart", make_string(soft_no_start));
		insert_dict_value(k1, "NoEnd", make_string(soft_no_end));
		insert_dict_value(k1, "Keep", make_string(hard_keep));      // same as Hard
		insert_dict_value(k1, "Hanging", make_string(hard_hang));   // same as Hard
		append_array_item(kinsoku_set, std::move(k1));
	}
	insert_dict_value(resource_dict, "KinsokuSet", std::move(kinsoku_set));

	// ---------- MojiKumiSet ----------
	auto mojikumi_set = make_array();
	{
		auto m0 = make_dict();
		insert_dict_value(m0, "InternalName", make_string(encode_engine_literal_utf16be("Photoshop6MojiKumiSet1")));
		append_array_item(mojikumi_set, std::move(m0));
		auto m1 = make_dict();
		insert_dict_value(m1, "InternalName", make_string(encode_engine_literal_utf16be("Photoshop6MojiKumiSet2")));
		append_array_item(mojikumi_set, std::move(m1));
		auto m2 = make_dict();
		insert_dict_value(m2, "InternalName", make_string(encode_engine_literal_utf16be("Photoshop6MojiKumiSet3")));
		append_array_item(mojikumi_set, std::move(m2));
		auto m3 = make_dict();
		insert_dict_value(m3, "InternalName", make_string(encode_engine_literal_utf16be("Photoshop6MojiKumiSet4")));
		append_array_item(mojikumi_set, std::move(m3));
	}
	insert_dict_value(resource_dict, "MojiKumiSet", std::move(mojikumi_set));

	// ---------- TheNormal*Sheet ----------
	insert_dict_value(resource_dict, "TheNormalStyleSheet", make_integer(0));
	insert_dict_value(resource_dict, "TheNormalParagraphSheet", make_integer(0));

	// ---------- ParagraphSheetSet ----------
	auto ps_set = make_array();
	{
		auto ps0 = make_dict();
		insert_dict_value(ps0, "Name", make_string(encode_engine_literal_utf16be("Normal RGB")));
		insert_dict_value(ps0, "DefaultStyleSheet", make_integer(0));
		auto pps = make_dict();
		insert_dict_value(pps, "Justification", make_integer(0));
		insert_dict_value(pps, "FirstLineIndent", make_float(0.0));
		insert_dict_value(pps, "StartIndent", make_float(0.0));
		insert_dict_value(pps, "EndIndent", make_float(0.0));
		insert_dict_value(pps, "SpaceBefore", make_float(0.0));
		insert_dict_value(pps, "SpaceAfter", make_float(0.0));
		insert_dict_value(pps, "AutoHyphenate", make_bool(true));
		insert_dict_value(pps, "HyphenatedWordSize", make_integer(6));
		insert_dict_value(pps, "PreHyphen", make_integer(2));
		insert_dict_value(pps, "PostHyphen", make_integer(2));
		insert_dict_value(pps, "ConsecutiveHyphens", make_integer(8));
		insert_dict_value(pps, "Zone", make_float(36.0));
		auto ws = make_array();
		append_array_item(ws, make_float(.8));
		append_array_item(ws, make_float(1.0));
		append_array_item(ws, make_float(1.33));
		insert_dict_value(pps, "WordSpacing", std::move(ws));
		auto ls2 = make_array();
		append_array_item(ls2, make_float(0.0));
		append_array_item(ls2, make_float(0.0));
		append_array_item(ls2, make_float(0.0));
		insert_dict_value(pps, "LetterSpacing", std::move(ls2));
		auto gs2 = make_array();
		append_array_item(gs2, make_float(1.0));
		append_array_item(gs2, make_float(1.0));
		append_array_item(gs2, make_float(1.0));
		insert_dict_value(pps, "GlyphSpacing", std::move(gs2));
		insert_dict_value(pps, "AutoLeading", make_float(1.2));
		insert_dict_value(pps, "LeadingType", make_integer(0));
		insert_dict_value(pps, "Hanging", make_bool(false));
		insert_dict_value(pps, "Burasagari", make_bool(false));
		insert_dict_value(pps, "KinsokuOrder", make_integer(0));
		insert_dict_value(pps, "EveryLineComposer", make_bool(false));
		insert_dict_value(ps0, "Properties", std::move(pps));
		append_array_item(ps_set, std::move(ps0));
	}
	insert_dict_value(resource_dict, "ParagraphSheetSet", std::move(ps_set));

	// ---------- StyleSheetSet ----------
	auto ss_set = make_array();
	{
		auto ss0 = make_dict();
		insert_dict_value(ss0, "Name", make_string(encode_engine_literal_utf16be("Normal RGB")));
		auto ssd = make_dict();
		insert_dict_value(ssd, "Font", make_integer(0));
		insert_dict_value(ssd, "FontSize", make_float(12.0));
		insert_dict_value(ssd, "FauxBold", make_bool(false));
		insert_dict_value(ssd, "FauxItalic", make_bool(false));
		insert_dict_value(ssd, "AutoLeading", make_bool(true));
		insert_dict_value(ssd, "Leading", make_float(0.0));
		insert_dict_value(ssd, "HorizontalScale", make_float(1.0));
		insert_dict_value(ssd, "VerticalScale", make_float(1.0));
		insert_dict_value(ssd, "Tracking", make_integer(0));
		insert_dict_value(ssd, "BaselineShift", make_float(0.0));
		insert_dict_value(ssd, "AutoKerning", make_bool(true));
		insert_dict_value(ssd, "Kerning", make_integer(0));
		insert_dict_value(ssd, "FontCaps", make_integer(0));
		insert_dict_value(ssd, "FontBaseline", make_integer(0));
		insert_dict_value(ssd, "Underline", make_bool(false));
		insert_dict_value(ssd, "Strikethrough", make_bool(false));
		insert_dict_value(ssd, "Ligatures", make_bool(true));
		insert_dict_value(ssd, "DLigatures", make_bool(false));
		insert_dict_value(ssd, "BaselineDirection", make_integer(2));
		insert_dict_value(ssd, "Tsume", make_float(0.0));
		insert_dict_value(ssd, "StyleRunAlignment", make_integer(2));
		insert_dict_value(ssd, "Language", make_integer(0));
		insert_dict_value(ssd, "NoBreak", make_bool(false));
		insert_dict_value(ssd, "CharacterDirection", make_integer(0));
		insert_dict_value(ssd, "HindiNumbers", make_bool(false));
		insert_dict_value(ssd, "Kashida", make_integer(1));
		insert_dict_value(ssd, "DiacriticPos", make_integer(2));
		auto fc = make_dict();
		insert_dict_value(fc, "Type", make_integer(1));
		auto fv = make_array();
		for (double v : fill_color) append_array_item(fv, make_float(v));
		insert_dict_value(fc, "Values", std::move(fv));
		insert_dict_value(ssd, "FillColor", std::move(fc));
		auto sc = make_dict();
		insert_dict_value(sc, "Type", make_integer(1));
		auto sv = make_array();
		append_array_item(sv, make_float(1.0));
		append_array_item(sv, make_float(0.0));
		append_array_item(sv, make_float(0.0));
		append_array_item(sv, make_float(0.0));
		insert_dict_value(sc, "Values", std::move(sv));
		insert_dict_value(ssd, "StrokeColor", std::move(sc));
		insert_dict_value(ssd, "FillFlag", make_bool(true));
		insert_dict_value(ssd, "StrokeFlag", make_bool(false));
		insert_dict_value(ssd, "FillFirst", make_bool(false));
		insert_dict_value(ssd, "YUnderline", make_integer(1));
		insert_dict_value(ssd, "OutlineWidth", make_float(1.0));
		insert_dict_value(ss0, "StyleSheetData", std::move(ssd));
		append_array_item(ss_set, std::move(ss0));
	}
	insert_dict_value(resource_dict, "StyleSheetSet", std::move(ss_set));

	// ---------- FontSet ----------
	auto font_set = make_array();
	{
		auto f0 = make_dict();
		insert_dict_value(f0, "Name", make_string(font_name_encoded));
		insert_dict_value(f0, "Script", make_integer(0));
		insert_dict_value(f0, "FontType", make_integer(1));
		insert_dict_value(f0, "Synthetic", make_integer(0));
		append_array_item(font_set, std::move(f0));
		auto f1 = make_dict();
		insert_dict_value(f1, "Name", make_string(sentinel_name));
		insert_dict_value(f1, "Script", make_integer(0));
		insert_dict_value(f1, "FontType", make_integer(0));
		insert_dict_value(f1, "Synthetic", make_integer(0));
		append_array_item(font_set, std::move(f1));
	}
	insert_dict_value(resource_dict, "FontSet", std::move(font_set));

	// ---------- Superscript / Subscript / SmallCap (flat keys, matching Photoshop) ----------
	insert_dict_value(resource_dict, "SuperscriptSize", make_float(0.583));
	insert_dict_value(resource_dict, "SuperscriptPosition", make_float(0.333));
	insert_dict_value(resource_dict, "SubscriptSize", make_float(0.583));
	insert_dict_value(resource_dict, "SubscriptPosition", make_float(0.333));
	insert_dict_value(resource_dict, "SmallCapSize", make_float(0.7));

	insert_dict_value(root, "ResourceDict", std::move(resource_dict));

	// ---------- DocumentResources (root-level, sibling of EngineDict/ResourceDict) ----------
	auto doc_res = make_dict();

	// -- DocumentResources KinsokuSet (identical to ResourceDict) --
	{
		auto dr_ks = make_array();
		auto drk0 = make_dict();
		insert_dict_value(drk0, "Name", make_string(encode_engine_literal_utf16be("PhotoshopKinsokuHard")));
		insert_dict_value(drk0, "NoStart", make_string(hard_no_start));
		insert_dict_value(drk0, "NoEnd", make_string(hard_no_end));
		insert_dict_value(drk0, "Keep", make_string(hard_keep));
		insert_dict_value(drk0, "Hanging", make_string(hard_hang));
		append_array_item(dr_ks, std::move(drk0));
		auto drk1 = make_dict();
		insert_dict_value(drk1, "Name", make_string(encode_engine_literal_utf16be("PhotoshopKinsokuSoft")));
		insert_dict_value(drk1, "NoStart", make_string(soft_no_start));
		insert_dict_value(drk1, "NoEnd", make_string(soft_no_end));
		insert_dict_value(drk1, "Keep", make_string(hard_keep));
		insert_dict_value(drk1, "Hanging", make_string(hard_hang));
		append_array_item(dr_ks, std::move(drk1));
		insert_dict_value(doc_res, "KinsokuSet", std::move(dr_ks));
	}

	// -- DocumentResources MojiKumiSet --
	{
		auto dr_mk = make_array();
		for (int i = 1; i <= 4; ++i) {
			auto m = make_dict();
			insert_dict_value(m, "InternalName",
				make_string(encode_engine_literal_utf16be("Photoshop6MojiKumiSet" + std::to_string(i))));
			append_array_item(dr_mk, std::move(m));
		}
		insert_dict_value(doc_res, "MojiKumiSet", std::move(dr_mk));
	}

	// -- TheNormal* refs --
	insert_dict_value(doc_res, "TheNormalStyleSheet", make_integer(0));
	insert_dict_value(doc_res, "TheNormalParagraphSheet", make_integer(0));

	// -- DocumentResources ParagraphSheetSet --
	{
		auto dr_ps = make_array();
		auto drps0 = make_dict();
		insert_dict_value(drps0, "Name", make_string(encode_engine_literal_utf16be("Normal RGB")));
		insert_dict_value(drps0, "DefaultStyleSheet", make_integer(0));
		auto drpps = make_dict();
		insert_dict_value(drpps, "Justification", make_integer(0));
		insert_dict_value(drpps, "FirstLineIndent", make_float(0.0));
		insert_dict_value(drpps, "StartIndent", make_float(0.0));
		insert_dict_value(drpps, "EndIndent", make_float(0.0));
		insert_dict_value(drpps, "SpaceBefore", make_float(0.0));
		insert_dict_value(drpps, "SpaceAfter", make_float(0.0));
		insert_dict_value(drpps, "AutoHyphenate", make_bool(true));
		insert_dict_value(drpps, "HyphenatedWordSize", make_integer(6));
		insert_dict_value(drpps, "PreHyphen", make_integer(2));
		insert_dict_value(drpps, "PostHyphen", make_integer(2));
		insert_dict_value(drpps, "ConsecutiveHyphens", make_integer(8));
		insert_dict_value(drpps, "Zone", make_float(36.0));
		{
			auto ws = make_array();
			append_array_item(ws, make_float(.8));
			append_array_item(ws, make_float(1.0));
			append_array_item(ws, make_float(1.33));
			insert_dict_value(drpps, "WordSpacing", std::move(ws));
		}
		{
			auto ls = make_array();
			append_array_item(ls, make_float(0.0));
			append_array_item(ls, make_float(0.0));
			append_array_item(ls, make_float(0.0));
			insert_dict_value(drpps, "LetterSpacing", std::move(ls));
		}
		{
			auto gs = make_array();
			append_array_item(gs, make_float(1.0));
			append_array_item(gs, make_float(1.0));
			append_array_item(gs, make_float(1.0));
			insert_dict_value(drpps, "GlyphSpacing", std::move(gs));
		}
		insert_dict_value(drpps, "AutoLeading", make_float(1.2));
		insert_dict_value(drpps, "LeadingType", make_integer(0));
		insert_dict_value(drpps, "Hanging", make_bool(false));
		insert_dict_value(drpps, "Burasagari", make_bool(false));
		insert_dict_value(drpps, "KinsokuOrder", make_integer(0));
		insert_dict_value(drpps, "EveryLineComposer", make_bool(false));
		insert_dict_value(drps0, "Properties", std::move(drpps));
		append_array_item(dr_ps, std::move(drps0));
		insert_dict_value(doc_res, "ParagraphSheetSet", std::move(dr_ps));
	}

	// -- DocumentResources StyleSheetSet --
	{
		auto dr_ss = make_array();
		auto drss0 = make_dict();
		insert_dict_value(drss0, "Name", make_string(encode_engine_literal_utf16be("Normal RGB")));
		auto drssd = make_dict();
		insert_dict_value(drssd, "Font", make_integer(0));
		insert_dict_value(drssd, "FontSize", make_float(12.0));
		insert_dict_value(drssd, "FauxBold", make_bool(false));
		insert_dict_value(drssd, "FauxItalic", make_bool(false));
		insert_dict_value(drssd, "AutoLeading", make_bool(true));
		insert_dict_value(drssd, "Leading", make_float(0.0));
		insert_dict_value(drssd, "HorizontalScale", make_float(1.0));
		insert_dict_value(drssd, "VerticalScale", make_float(1.0));
		insert_dict_value(drssd, "Tracking", make_integer(0));
		insert_dict_value(drssd, "AutoKerning", make_bool(true));
		insert_dict_value(drssd, "Kerning", make_integer(0));
		insert_dict_value(drssd, "BaselineShift", make_float(0.0));
		insert_dict_value(drssd, "FontCaps", make_integer(0));
		insert_dict_value(drssd, "FontBaseline", make_integer(0));
		insert_dict_value(drssd, "Underline", make_bool(false));
		insert_dict_value(drssd, "Strikethrough", make_bool(false));
		insert_dict_value(drssd, "Ligatures", make_bool(true));
		insert_dict_value(drssd, "DLigatures", make_bool(false));
		insert_dict_value(drssd, "BaselineDirection", make_integer(2));
		insert_dict_value(drssd, "Tsume", make_float(0.0));
		insert_dict_value(drssd, "StyleRunAlignment", make_integer(2));
		insert_dict_value(drssd, "Language", make_integer(0));
		insert_dict_value(drssd, "NoBreak", make_bool(false));
		{
			auto fc = make_dict();
			insert_dict_value(fc, "Type", make_integer(1));
			auto fv = make_array();
			for (double v : fill_color) append_array_item(fv, make_float(v));
			insert_dict_value(fc, "Values", std::move(fv));
			insert_dict_value(drssd, "FillColor", std::move(fc));
		}
		{
			auto sc = make_dict();
			insert_dict_value(sc, "Type", make_integer(1));
			auto sv = make_array();
			append_array_item(sv, make_float(1.0));
			append_array_item(sv, make_float(0.0));
			append_array_item(sv, make_float(0.0));
			append_array_item(sv, make_float(0.0));
			insert_dict_value(sc, "Values", std::move(sv));
			insert_dict_value(drssd, "StrokeColor", std::move(sc));
		}
		insert_dict_value(drssd, "FillFlag", make_bool(true));
		insert_dict_value(drssd, "StrokeFlag", make_bool(false));
		insert_dict_value(drssd, "FillFirst", make_bool(false));
		insert_dict_value(drssd, "YUnderline", make_integer(1));
		insert_dict_value(drssd, "OutlineWidth", make_float(1.0));
		insert_dict_value(drssd, "CharacterDirection", make_integer(0));
		insert_dict_value(drssd, "HindiNumbers", make_bool(false));
		insert_dict_value(drssd, "Kashida", make_integer(1));
		insert_dict_value(drssd, "DiacriticPos", make_integer(2));
		insert_dict_value(drss0, "StyleSheetData", std::move(drssd));
		append_array_item(dr_ss, std::move(drss0));
		insert_dict_value(doc_res, "StyleSheetSet", std::move(dr_ss));
	}

	// -- DocumentResources FontSet --
	{
		auto dfs = make_array();
		auto df0 = make_dict();
		insert_dict_value(df0, "Name", make_string(font_name_encoded));
		insert_dict_value(df0, "Script", make_integer(0));
		insert_dict_value(df0, "FontType", make_integer(1));
		insert_dict_value(df0, "Synthetic", make_integer(0));
		append_array_item(dfs, std::move(df0));
		auto df1 = make_dict();
		insert_dict_value(df1, "Name", make_string(sentinel_name));
		insert_dict_value(df1, "Script", make_integer(0));
		insert_dict_value(df1, "FontType", make_integer(0));
		insert_dict_value(df1, "Synthetic", make_integer(0));
		append_array_item(dfs, std::move(df1));
		insert_dict_value(doc_res, "FontSet", std::move(dfs));
	}

	// -- Super/Sub/SmallCap --
	insert_dict_value(doc_res, "SuperscriptSize", make_float(0.583));
	insert_dict_value(doc_res, "SuperscriptPosition", make_float(0.333));
	insert_dict_value(doc_res, "SubscriptSize", make_float(0.583));
	insert_dict_value(doc_res, "SubscriptPosition", make_float(0.333));
	insert_dict_value(doc_res, "SmallCapSize", make_float(0.7));

	insert_dict_value(root, "DocumentResources", std::move(doc_res));

	return EngineData::serialize(root);
}

// -----------------------------------------------------------------------
//  Build the text descriptor  (Photoshop binary descriptor containing
//  "Txt " TEXT, "engineData" tdta, "textGridding" enum, etc.)
// -----------------------------------------------------------------------

/// Build a text descriptor for the TySh resource.
/// Returns raw descriptor bytes including the 4-byte descriptor format version prefix.
/// bounds_ltrb / bbox_ltrb are {Left, Top, Right, Bottom} in points.
inline std::vector<std::byte> build_text_descriptor(
	const std::u16string& text_utf16,
	const std::vector<std::byte>& engine_data,
	const std::array<double,4>& bounds_ltrb = {0,0,0,0},
	const std::array<double,4>& bbox_ltrb   = {0,0,0,0})
{
	const auto text_utf8 = UnicodeString::convertUTF16LEtoUTF8(text_utf16);
	const auto& os_enum = Descriptors::Impl::descriptorKeys.at(Descriptors::Impl::OSTypes::Enumerated);
	const auto& os_unitf = Descriptors::Impl::descriptorKeys.at(Descriptors::Impl::OSTypes::UnitFloat);
	const auto& os_raw = Descriptors::Impl::descriptorKeys.at(Descriptors::Impl::OSTypes::RawData);

	Descriptors::Descriptor descriptor("TxLr");
	descriptor.insert("Txt ", UnicodeString(text_utf8, 1u));
	descriptor.insert("textGridding", std::make_unique<Descriptors::Enumerated>(
		"textGridding", os_enum, "textGridding", "None"));
	descriptor.insert("Ornt", std::make_unique<Descriptors::Enumerated>(
		"Ornt", os_enum, "Ornt", "Hrzn"));
	descriptor.insert("AntA", std::make_unique<Descriptors::Enumerated>(
		"AntA", os_enum, "Annt", "AnCr"));

	auto bounds = std::make_unique<Descriptors::Descriptor>("bounds");
	bounds->insert("Left", std::make_unique<Descriptors::UnitFloat>(
		"Left", os_unitf, Descriptors::Impl::UnitFloatType::Points, bounds_ltrb[0]));
	bounds->insert("Top ", std::make_unique<Descriptors::UnitFloat>(
		"Top ", os_unitf, Descriptors::Impl::UnitFloatType::Points, bounds_ltrb[1]));
	bounds->insert("Rght", std::make_unique<Descriptors::UnitFloat>(
		"Rght", os_unitf, Descriptors::Impl::UnitFloatType::Points, bounds_ltrb[2]));
	bounds->insert("Btom", std::make_unique<Descriptors::UnitFloat>(
		"Btom", os_unitf, Descriptors::Impl::UnitFloatType::Points, bounds_ltrb[3]));
	descriptor.insert("bounds", std::move(bounds));

	auto bounding_box = std::make_unique<Descriptors::Descriptor>("boundingBox");
	bounding_box->insert("Left", std::make_unique<Descriptors::UnitFloat>(
		"Left", os_unitf, Descriptors::Impl::UnitFloatType::Points, bbox_ltrb[0]));
	bounding_box->insert("Top ", std::make_unique<Descriptors::UnitFloat>(
		"Top ", os_unitf, Descriptors::Impl::UnitFloatType::Points, bbox_ltrb[1]));
	bounding_box->insert("Rght", std::make_unique<Descriptors::UnitFloat>(
		"Rght", os_unitf, Descriptors::Impl::UnitFloatType::Points, bbox_ltrb[2]));
	bounding_box->insert("Btom", std::make_unique<Descriptors::UnitFloat>(
		"Btom", os_unitf, Descriptors::Impl::UnitFloatType::Points, bbox_ltrb[3]));
	descriptor.insert("boundingBox", std::move(bounding_box));

	descriptor.insert("TextIndex", static_cast<int32_t>(1));

	std::vector<uint8_t> engine_data_u8{};
	engine_data_u8.reserve(engine_data.size());
	for (const auto b : engine_data)
	{
		engine_data_u8.push_back(std::to_integer<uint8_t>(b));
	}
	descriptor.insert("EngineData", std::make_unique<Descriptors::RawData>("EngineData", os_raw, std::move(engine_data_u8)));

	return serialize_descriptor_with_version(descriptor, 16u);
}

// -----------------------------------------------------------------------
//  Build the warp descriptor
// -----------------------------------------------------------------------

/// Build a minimal "no-warp" descriptor.
inline std::vector<std::byte> build_warp_descriptor()
{
	std::vector<std::byte> buf;
	buf.reserve(200);

	// Warp version/type prefix: 2 bytes version (0x0001) + 4 bytes (descriptor version = 16)
	push_u16_be(buf, 1u);           // warp version
	const auto& os_enum = Descriptors::Impl::descriptorKeys.at(Descriptors::Impl::OSTypes::Enumerated);

	Descriptors::Descriptor warp_desc("warp");
	warp_desc.insert("warpStyle", std::make_unique<Descriptors::Enumerated>(
		"warpStyle", os_enum, "warpStyle", "warpNone"));
	warp_desc.insert("warpValue", 0.0);
	warp_desc.insert("warpPerspective", 0.0);
	warp_desc.insert("warpPerspectiveOther", 0.0);
	warp_desc.insert("warpRotate", std::make_unique<Descriptors::Enumerated>(
		"warpRotate", os_enum, "Ornt", "Hrzn"));

	const auto desc_bytes = serialize_descriptor_with_version(warp_desc, 16u);
	buf.insert(buf.end(), desc_bytes.begin(), desc_bytes.end());

	// After warp descriptor: Photoshop writes 19 zero-bytes as padding/bounds
	// (not 4 full doubles).  Match exactly for compatibility.
	for (int i = 0; i < 19; ++i)
		buf.push_back(static_cast<std::byte>(0));

	return buf;
}

// -----------------------------------------------------------------------
//  Build complete TySh TaggedBlock
// -----------------------------------------------------------------------

/// Build a complete TySh tagged block from scratch.
/// The result can be inserted into Layer<T>::m_UnparsedBlocks.
inline std::shared_ptr<TypeToolTaggedBlock> build_tysh_tagged_block(
	const std::string& text_utf8,
	const std::string& font_postscript_name,
	double font_size = 24.0,
	const std::vector<double>& fill_color = { 1.0, 0.0, 0.0, 0.0 },
	double position_x = 20.0,
	double position_y = 50.0,
	double box_width  = 0.0,
	double box_height = 0.0)
{
	auto utf16 = UnicodeString::convertUTF8ToUTF16LE(text_utf8);
	// Photoshop uses \r (0x0D) for line breaks, not \n (0x0A)
	for (auto& ch : utf16)
		if (ch == u'\n') ch = u'\r';

	// Estimate text bounds from font metrics (rough approximation;
	// Photoshop recalculates exact bounds on first edit).
	// For a typical font the ascent is ~0.85*fontSize and the
	// average glyph width is ~0.55*fontSize.
	const double ascent  = font_size * 0.85;
	const double descent = font_size * 0.15;

	// Find the longest line (count UTF-16 code units per line)
	size_t max_line_len = 0;
	size_t cur_line_len = 0;
	size_t num_lines = 1;
	for (const char16_t ch : utf16) {
		if (ch == u'\r' || ch == u'\n') {
			max_line_len = std::max(max_line_len, cur_line_len);
			cur_line_len = 0;
			++num_lines;
		} else {
			++cur_line_len;
		}
	}
	max_line_len = std::max(max_line_len, cur_line_len);

	const double line_height = font_size * 1.2;
	const double width  = (box_width  > 0.0) ? box_width  : max_line_len * font_size * 0.55;
	const double height = (box_height > 0.0) ? box_height : num_lines * line_height;

	// bounds: Left=0, Top=-ascent, Right=width, Bottom=height-ascent
	std::array<double,4> bounds_ltrb = {
		0.0,
		-ascent,
		width,
		height - ascent
	};
	// boundingBox: slight inset (typical Photoshop values)
	std::array<double,4> bbox_ltrb = {
		2.0,
		-(ascent - descent),
		width - 2.0,
		0.0
	};

	// 1. Build EngineData blob
	auto engine_data = build_engine_data(text_utf8, font_postscript_name, font_size, fill_color, width, height);

	// 2. Build text descriptor (with computed bounds)
	auto text_desc = build_text_descriptor(utf16, engine_data, bounds_ltrb, bbox_ltrb);

	// 3. Build warp descriptor
	auto warp_desc = build_warp_descriptor();

	// 4. Assemble TySh data
	std::vector<std::byte> data;
	data.reserve(56 + 2 + text_desc.size() + warp_desc.size());

	// Version
	push_u16_be(data, 1u);

	// Transform: 6 doubles (identity scaling + position via tx,ty)
	push_double_be(data, 1.0);          // xx
	push_double_be(data, 0.0);          // xy
	push_double_be(data, 0.0);          // yx
	push_double_be(data, 1.0);          // yy
	push_double_be(data, position_x);   // tx
	push_double_be(data, position_y);   // ty

	// Text descriptor version (50 = Photoshop 6.0+)
	push_u16_be(data, 50u);

	// Text descriptor body
	data.insert(data.end(), text_desc.begin(), text_desc.end());

	// Warp descriptor (includes own version prefix)
	data.insert(data.end(), warp_desc.begin(), warp_desc.end());

	auto block = std::make_shared<TypeToolTaggedBlock>();
	block->m_Data = std::move(data);
	[[maybe_unused]] auto _descriptor = block->parse_descriptors_from_data();
	return block;
}

} // namespace TextLayerDetail

PSAPI_NAMESPACE_END

