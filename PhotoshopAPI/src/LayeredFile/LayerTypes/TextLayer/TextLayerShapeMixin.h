#pragma once

// =========================================================================
//  TextLayerShapeMixin  –  Read/write the text frame (point text vs box text)
//
//  Photoshop stores the text frame shape inside the EngineData blob.
//
//  Three ShapeType locations (all must stay in sync):
//    Children[0] / ShapeType                          (top-level child)
//    Children[0] / Cookie / Photoshop / ShapeType     (inside Cookie)
//    Children[0] / Cookie / Photoshop / Base / ShapeType
//
//  Box text (ShapeType == 1) has:
//    Children[0] / Cookie / Photoshop / BoxBounds  [ top left right bottom ]
//
//  Point text (ShapeType == 0) has:
//    Children[0] / Cookie / Photoshop / PointBase  [ x y ]
//
//  This mixin exposes:
//    - is_box_text() / is_point_text()
//    - box_bounds()  → optional<array<double,4>>
//    - box_width()  / box_height()
//    - set_box_bounds(top, left, bottom, right)
//    - set_box_size(width, height)  (keeps top-left, adjusts bottom-right)
//    - set_box_width(width) / set_box_height(height)
//    - convert_to_box_text(width, height)  (point → box)
//    - convert_to_point_text()             (box → point)
// =========================================================================

#include "Macros.h"
#include "TextLayerFwd.h"
#include "TextLayerEnum.h"
#include "TextLayerEngineDataUtils.h"
#include "TextLayerParsingUtils.h"

#include "Core/Struct/EngineDataStructure.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

PSAPI_NAMESPACE_BEGIN

template <typename Derived>
struct TextLayerShapeMixin
{
protected:
	Derived* self() { return static_cast<Derived*>(this); }
	const Derived* self() const { return static_cast<const Derived*>(this); }

public:

	// -----------------------------------------------------------------------
	//  Read helpers
	// -----------------------------------------------------------------------

	/// Return the ShapeType value: 0 = point text, 1 = box text.
	/// Returns nullopt when the EngineData cannot be read.
	std::optional<TextLayerEnum::ShapeType> shape_type() const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;

			const auto val = find_cookie_shape_type(parsed.root);
			if (val == nullptr) continue;

			int32_t result = 0;
			if (TextLayerDetail::number_value_to_int32(*val, result))
				return static_cast<TextLayerEnum::ShapeType>(result);
		}
		return std::nullopt;
	}

	/// True when the text layer is area (box) text (ShapeType == 1).
	bool is_box_text() const
	{
		const auto st = shape_type();
		return st.has_value() && st.value() == TextLayerEnum::ShapeType::BoxText;
	}

	/// True when the text layer is point text (ShapeType == 0).
	bool is_point_text() const
	{
		const auto st = shape_type();
		return st.has_value() && st.value() == TextLayerEnum::ShapeType::PointText;
	}

	/// Return the box bounds as { top, left, bottom, right }.
	/// (Note: on-disk format is [ top left right bottom ]; we swap to the API order.)
	/// Returns nullopt for point text or if the data cannot be read.
	std::optional<std::array<double, 4>> box_bounds() const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;

			auto bounds = read_box_bounds(parsed.root);
			if (bounds.has_value()) return bounds;
		}
		return std::nullopt;
	}

	/// Returns the box width, or nullopt for point text.
	std::optional<double> box_width() const
	{
		auto b = box_bounds();
		if (!b.has_value()) return std::nullopt;
		return (*b)[3] - (*b)[1];   // right - left  (API order)
	}

	/// Returns the box height, or nullopt for point text.
	std::optional<double> box_height() const
	{
		auto b = box_bounds();
		if (!b.has_value()) return std::nullopt;
		return (*b)[2] - (*b)[0];   // bottom - top  (API order)
	}

	// -----------------------------------------------------------------------
	//  Write helpers
	// -----------------------------------------------------------------------

	/// Set the box bounds in the EngineData.  The layer must already be box text.
	/// Values are { top, left, bottom, right } in Photoshop's text-space coordinates.
	/// (Note: on-disk format is [ top left right bottom ]; we swap internally.)
	bool set_box_bounds(double top, double left, double bottom, double right)
	{
		if (!std::isfinite(top) || !std::isfinite(left) || !std::isfinite(bottom) || !std::isfinite(right))
			return false;

		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto engine_span_opt = TextLayerDetail::find_engine_data_span(*block);
			if (!engine_span_opt.has_value()) continue;

			std::vector<std::byte> payload(
				block->m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset),
				block->m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset + engine_span_opt->byte_count)
			);

			auto parsed = EngineData::parse(payload);
			if (!parsed.ok) continue;

			auto data_node = find_box_bounds_node(parsed.root);
			if (data_node == nullptr) return false;

			// On-disk order is [top, left, right, bottom]
			const std::vector<double> new_vals = { top, left, right, bottom };
			const size_t old_start = data_node->start_offset;
			const size_t old_end = data_node->end_offset;

			if (!EngineData::set_double_array(*data_node, new_vals))
				return false;
			// Mark all items as non-integer so they serialize with a decimal point
			for (auto& item : data_node->array_items) item.is_integer = false;

			auto new_bytes = EngineData::format_value_bytes(*data_node);
			EngineData::splice_payload(payload, old_start, old_end, new_bytes);
			return TextLayerDetail::write_engine_payload(*block, engine_span_opt.value(), payload);
		}
		return false;
	}

	/// Convenience: set box width and height, keeping the current top-left corner.
	/// The layer must already be box text with existing bounds.
	bool set_box_size(double width, double height)
	{
		if (!std::isfinite(width) || !std::isfinite(height) || width <= 0.0 || height <= 0.0)
			return false;

		auto b = box_bounds();
		if (!b.has_value()) return false;

		const double top = (*b)[0];
		const double left = (*b)[1];
		return set_box_bounds(top, left, top + height, left + width);
	}

	/// Set only the box width, keeping top, left, and height unchanged.
	/// The layer must already be box text with existing bounds.
	bool set_box_width(double width)
	{
		if (!std::isfinite(width) || width <= 0.0)
			return false;

		auto b = box_bounds();
		if (!b.has_value()) return false;

		return set_box_bounds((*b)[0], (*b)[1], (*b)[2], (*b)[1] + width);
	}

	/// Set only the box height, keeping top, left, and width unchanged.
	/// The layer must already be box text with existing bounds.
	bool set_box_height(double height)
	{
		if (!std::isfinite(height) || height <= 0.0)
			return false;

		auto b = box_bounds();
		if (!b.has_value()) return false;

		return set_box_bounds((*b)[0], (*b)[1], (*b)[0] + height, (*b)[3]);
	}

	/// Convert an existing point-text layer to box text with the given width and height.
	/// The box is placed at (0, 0) in text-space coordinates.
	/// Returns false if the layer is already box text or if writing fails.
	///
	/// This modifies the EngineData tree and re-serializes it:
	///   - All three ShapeType values are set to 1
	///   - The PointBase entry in Cookie/Photoshop is replaced with BoxBounds
	bool convert_to_box_text(double width, double height)
	{
		if (!std::isfinite(width) || !std::isfinite(height) || width <= 0.0 || height <= 0.0)
			return false;

		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto engine_span_opt = TextLayerDetail::find_engine_data_span(*block);
			if (!engine_span_opt.has_value()) continue;

			std::vector<std::byte> payload(
				block->m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset),
				block->m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset + engine_span_opt->byte_count)
			);

			auto parsed = EngineData::parse(payload);
			if (!parsed.ok) continue;

			// Verify it is currently point text
			auto cookie_st = find_cookie_shape_type(parsed.root);
			if (cookie_st == nullptr) return false;

			int32_t current_shape = 0;
			if (!TextLayerDetail::number_value_to_int32(*cookie_st, current_shape))
				return false;
			if (current_shape == 1) return false;   // already box text

			// --- Modify the tree ---

			// 1. Set all three ShapeType values to 1
			if (!set_all_shape_types(parsed.root, 1))
				return false;

			// 2. In the Photoshop dict: remove PointBase, insert BoxBounds
			auto* ps_dict = find_photoshop_dict(parsed.root);
			if (ps_dict == nullptr) return false;

			EngineData::remove_dict_value(*ps_dict, "PointBase");

			auto bounds_arr = EngineData::make_array();
			// On-disk order: [top, left, right, bottom]
			for (double v : { 0.0, 0.0, width, height })
			{
				auto num = EngineData::make_number(v);
				num.is_integer = false;
				EngineData::append_array_item(bounds_arr, std::move(num));
			}
			EngineData::insert_dict_value(*ps_dict, "BoxBounds", std::move(bounds_arr));

			// --- Re-serialize and write back ---
			auto new_payload = EngineData::serialize(parsed.root);
			return TextLayerDetail::write_engine_payload(*block, engine_span_opt.value(), new_payload);
		}
		return false;
	}

	/// Convert an existing box-text layer back to point text.
	/// Removes the bounding constraint so text is unbounded.
	/// Returns false if the layer is already point text or if writing fails.
	///
	/// This modifies the EngineData tree and re-serializes it:
	///   - All three ShapeType values are set to 0
	///   - The BoxBounds entry in Cookie/Photoshop is replaced with PointBase
	bool convert_to_point_text()
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto engine_span_opt = TextLayerDetail::find_engine_data_span(*block);
			if (!engine_span_opt.has_value()) continue;

			std::vector<std::byte> payload(
				block->m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset),
				block->m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset + engine_span_opt->byte_count)
			);

			auto parsed = EngineData::parse(payload);
			if (!parsed.ok) continue;

			auto cookie_st = find_cookie_shape_type(parsed.root);
			if (cookie_st == nullptr) return false;

			int32_t current_shape = 0;
			if (!TextLayerDetail::number_value_to_int32(*cookie_st, current_shape))
				return false;
			if (current_shape == 0) return false;   // already point text

			// --- Modify the tree ---

			// 1. Set all three ShapeType values to 0
			if (!set_all_shape_types(parsed.root, 0))
				return false;

			// 2. In the Photoshop dict: remove BoxBounds, insert PointBase
			auto* ps_dict = find_photoshop_dict(parsed.root);
			if (ps_dict == nullptr) return false;

			EngineData::remove_dict_value(*ps_dict, "BoxBounds");

			auto point_arr = EngineData::make_array();
			for (double v : { 0.0, 0.0 })
			{
				auto num = EngineData::make_number(v);
				num.is_integer = false;
				EngineData::append_array_item(point_arr, std::move(num));
			}
			EngineData::insert_dict_value(*ps_dict, "PointBase", std::move(point_arr));

			// --- Re-serialize and write back ---
			auto new_payload = EngineData::serialize(parsed.root);
			return TextLayerDetail::write_engine_payload(*block, engine_span_opt.value(), new_payload);
		}
		return false;
	}

private:

	// -----------------------------------------------------------------------
	//  Internal navigation
	// -----------------------------------------------------------------------

	/// Get the first child of EngineDict/Rendered/Shapes/Children.
	static EngineData::Value* find_first_shape_child(EngineData::Value& root)
	{
		auto children = EngineData::find_by_path(root, { "EngineDict", "Rendered", "Shapes", "Children" });
		if (children == nullptr || children->type != EngineData::ValueType::Array || children->array_items.empty())
			return nullptr;
		return &children->array_items[0];
	}

	static const EngineData::Value* find_first_shape_child(const EngineData::Value& root)
	{
		const auto children = EngineData::find_by_path(root, { "EngineDict", "Rendered", "Shapes", "Children" });
		if (children == nullptr || children->type != EngineData::ValueType::Array || children->array_items.empty())
			return nullptr;
		return &children->array_items[0];
	}

	/// Navigate to Cookie/Photoshop/ShapeType (the primary read location).
	static const EngineData::Value* find_cookie_shape_type(const EngineData::Value& root)
	{
		const auto child = find_first_shape_child(root);
		if (child == nullptr) return nullptr;
		return EngineData::find_by_path(*child, { "Cookie", "Photoshop", "ShapeType" });
	}

	static EngineData::Value* find_cookie_shape_type(EngineData::Value& root)
	{
		auto child = find_first_shape_child(root);
		if (child == nullptr) return nullptr;
		return EngineData::find_by_path(*child, { "Cookie", "Photoshop", "ShapeType" });
	}

	/// Navigate to Cookie/Photoshop dict.
	static EngineData::Value* find_photoshop_dict(EngineData::Value& root)
	{
		auto child = find_first_shape_child(root);
		if (child == nullptr) return nullptr;
		return EngineData::find_by_path(*child, { "Cookie", "Photoshop" });
	}

	/// Navigate to the BoxBounds array: Children[0] / Cookie / Photoshop / BoxBounds
	static EngineData::Value* find_box_bounds_node(EngineData::Value& root)
	{
		auto child = find_first_shape_child(root);
		if (child == nullptr) return nullptr;
		return EngineData::find_by_path(*child, { "Cookie", "Photoshop", "BoxBounds" });
	}

	static const EngineData::Value* find_box_bounds_node(const EngineData::Value& root)
	{
		const auto child = find_first_shape_child(root);
		if (child == nullptr) return nullptr;
		return EngineData::find_by_path(*child, { "Cookie", "Photoshop", "BoxBounds" });
	}

	/// Set all three ShapeType values (top-level, Cookie/Photoshop, Base) in the tree.
	/// Returns true if at least the Cookie/Photoshop one was set.
	static bool set_all_shape_types(EngineData::Value& root, int new_type)
	{
		auto child = find_first_shape_child(root);
		if (child == nullptr) return false;

		const double val = static_cast<double>(new_type);
		bool any_set = false;

		// 1. Children[0] / ShapeType
		auto* top_st = EngineData::find_dict_value(*child, "ShapeType");
		if (top_st != nullptr)
		{
			EngineData::set_number(*top_st, val);
			any_set = true;
		}

		// 2. Children[0] / Cookie / Photoshop / ShapeType
		auto* cookie_st = EngineData::find_by_path(*child, { "Cookie", "Photoshop", "ShapeType" });
		if (cookie_st != nullptr)
		{
			EngineData::set_number(*cookie_st, val);
			any_set = true;
		}

		// 3. Children[0] / Cookie / Photoshop / Base / ShapeType
		auto* base_st = EngineData::find_by_path(*child, { "Cookie", "Photoshop", "Base", "ShapeType" });
		if (base_st != nullptr)
		{
			EngineData::set_number(*base_st, val);
		}

		return any_set;
	}

	/// Read box bounds from the BoxBounds array.
	/// On-disk order is [top, left, right, bottom]; we return API order {top, left, bottom, right}.
	static std::optional<std::array<double, 4>> read_box_bounds(const EngineData::Value& root)
	{
		const auto data_node = find_box_bounds_node(root);
		if (data_node == nullptr || data_node->type != EngineData::ValueType::Array)
			return std::nullopt;

		std::vector<double> values;
		if (!EngineData::as_double_vector(*data_node, values) || values.size() < 4u)
			return std::nullopt;

		// Swap indices 2 and 3: on-disk [top, left, right, bottom] → API {top, left, bottom, right}
		return std::array<double, 4>{ values[0], values[1], values[3], values[2] };
	}
};

PSAPI_NAMESPACE_END
