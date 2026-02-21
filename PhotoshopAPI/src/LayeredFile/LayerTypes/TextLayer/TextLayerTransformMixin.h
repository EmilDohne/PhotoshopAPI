#pragma once

#include "Macros.h"
#include "TextLayerFwd.h"
#include "TextLayerEnum.h"
#include "TextLayerEngineDataUtils.h"
#include "TextLayerParsingUtils.h"

#include "Core/Struct/EngineDataStructure.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

PSAPI_NAMESPACE_BEGIN

template <typename Derived>
struct TextLayerTransformMixin
{
protected:
	Derived* self() { return static_cast<Derived*>(this); }
	const Derived* self() const { return static_cast<const Derived*>(this); }

public:

	// -----------------------------------------------------------------------
	//  Orientation / WritingDirection
	// -----------------------------------------------------------------------

	std::optional<TextLayerEnum::WritingDirection> orientation() const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto wd = EngineData::find_by_path(parsed.root, { "EngineDict", "Rendered", "Shapes", "WritingDirection" });
			if (wd != nullptr && wd->type == EngineData::ValueType::Number)
			{
				int32_t raw = static_cast<int32_t>(wd->is_integer ? wd->integer_value : static_cast<int64_t>(wd->number_value));
				return static_cast<TextLayerEnum::WritingDirection>(raw);
			}
		}
		return std::nullopt;
	}

	bool is_vertical() const
	{
		const auto wd = orientation();
		return wd.has_value() && wd.value() == TextLayerEnum::WritingDirection::Vertical;
	}

	bool set_orientation(const TextLayerEnum::WritingDirection writing_direction)
	{
		// Map the enum to the TySh descriptor value for the "Ornt" key.
		const std::string ornt_value =
			(writing_direction == TextLayerEnum::WritingDirection::Vertical) ? "Vrtc" : "Hrzn";
		const double procession_value =
			(writing_direction == TextLayerEnum::WritingDirection::Vertical) ? 1.0 : 0.0;

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

			auto shapes_wd = EngineData::find_by_path(parsed.root, { "EngineDict", "Rendered", "Shapes", "WritingDirection" });
			if (shapes_wd == nullptr) return false;

			std::vector<EngineData::PayloadPatch> patches;

			{
				const size_t old_start = shapes_wd->start_offset;
				const size_t old_end = shapes_wd->end_offset;
				if (!EngineData::set_number(*shapes_wd, static_cast<double>(writing_direction)))
					return false;
				patches.push_back({ old_start, old_end, EngineData::format_value_bytes(*shapes_wd) });
			}

			auto children = EngineData::find_by_path(parsed.root, { "EngineDict", "Rendered", "Shapes", "Children" });
			if (children != nullptr && children->type == EngineData::ValueType::Array)
			{
				for (auto& child : children->array_items)
				{
					// Photoshop-authored vertical text keeps Procession=1 on shape children.
					// Keep this in sync with WritingDirection to avoid UI/orientation mismatches.
					auto procession = EngineData::find_by_path(child, { "Procession" });
					if (procession != nullptr && procession->type == EngineData::ValueType::Number)
					{
						const size_t old_start = procession->start_offset;
						const size_t old_end = procession->end_offset;
						if (!EngineData::set_number(*procession, procession_value))
							return false;
						patches.push_back({ old_start, old_end, EngineData::format_value_bytes(*procession) });
					}

					auto lines_wd = EngineData::find_by_path(child, { "Lines", "WritingDirection" });
					if (lines_wd != nullptr)
					{
						const size_t old_start = lines_wd->start_offset;
						const size_t old_end = lines_wd->end_offset;
						if (!EngineData::set_number(*lines_wd, static_cast<double>(writing_direction)))
							return false;
						patches.push_back({ old_start, old_end, EngineData::format_value_bytes(*lines_wd) });
					}
				}
			}

			EngineData::apply_patches(payload, patches);
			if (!TextLayerDetail::write_engine_payload(*block, engine_span_opt.value(), payload))
				return false;

			// Also update the "Ornt" enum in the TySh binary descriptor so
			// Photoshop renders the correct orientation on open.
			TextLayerDetail::write_text_desc_enum(*block, "Ornt", ornt_value);

			return true;
		}
		return false;
	}

	// =======================================================================
	//  Transform Read/Write APIs
	// =======================================================================

	static constexpr size_t kTransformOffset = 2u;
	static constexpr size_t kTransformDoubles = 6u;
	static constexpr size_t kTransformBytes = kTransformDoubles * 8u;

	std::vector<double> transform() const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			if (block->getKey() != Enum::TaggedBlockKey::lrTypeTool) continue;
			if (block->m_Data.size() < kTransformOffset + kTransformBytes) continue;
			std::vector<double> result(kTransformDoubles);
			for (size_t i = 0u; i < kTransformDoubles; ++i)
				result[i] = TextLayerDetail::read_double_be(block->m_Data, kTransformOffset + i * 8u);
			return result;
		}
		return {};
	}

	std::optional<double> transform_component(const size_t index) const
	{
		if (index >= kTransformDoubles) return std::nullopt;
		for (const auto& block : self()->text_tagged_blocks())
		{
			if (block->getKey() != Enum::TaggedBlockKey::lrTypeTool) continue;
			if (block->m_Data.size() < kTransformOffset + kTransformBytes) continue;
			return TextLayerDetail::read_double_be(block->m_Data, kTransformOffset + index * 8u);
		}
		return std::nullopt;
	}

	std::optional<double> transform_xx() const { return transform_component(0); }
	std::optional<double> transform_xy() const { return transform_component(1); }
	std::optional<double> transform_yx() const { return transform_component(2); }
	std::optional<double> transform_yy() const { return transform_component(3); }
	std::optional<double> transform_tx() const { return transform_component(4); }
	std::optional<double> transform_ty() const { return transform_component(5); }

	bool set_transform(const std::vector<double>& values)
	{
		if (values.size() != kTransformDoubles) return false;
		for (const auto& block : self()->text_tagged_blocks())
		{
			if (block->getKey() != Enum::TaggedBlockKey::lrTypeTool) continue;
			if (block->m_Data.size() < kTransformOffset + kTransformBytes) continue;
			for (size_t i = 0u; i < kTransformDoubles; ++i)
				TextLayerDetail::write_double_be(block->m_Data, kTransformOffset + i * 8u, values[i]);
			return true;
		}
		return false;
	}

	bool set_transform_component(const size_t index, const double value)
	{
		if (index >= kTransformDoubles) return false;
		for (const auto& block : self()->text_tagged_blocks())
		{
			if (block->getKey() != Enum::TaggedBlockKey::lrTypeTool) continue;
			if (block->m_Data.size() < kTransformOffset + kTransformBytes) continue;
			TextLayerDetail::write_double_be(block->m_Data, kTransformOffset + index * 8u, value);
			return true;
		}
		return false;
	}

	bool set_transform_xx(const double value) { return set_transform_component(0, value); }
	bool set_transform_xy(const double value) { return set_transform_component(1, value); }
	bool set_transform_yx(const double value) { return set_transform_component(2, value); }
	bool set_transform_yy(const double value) { return set_transform_component(3, value); }
	bool set_transform_tx(const double value) { return set_transform_component(4, value); }
	bool set_transform_ty(const double value) { return set_transform_component(5, value); }

	// -----------------------------------------------------------------------
	//  High-level transform helpers
	// -----------------------------------------------------------------------

	std::optional<double> rotation_angle() const
	{
		const auto xform = transform();
		if (xform.size() < 4u) return std::nullopt;
		return std::atan2(xform[1], xform[0]) * (180.0 / 3.14159265358979323846);
	}

	std::optional<double> scale_x() const
	{
		const auto xform = transform();
		if (xform.size() < 4u) return std::nullopt;
		return std::sqrt(xform[0] * xform[0] + xform[1] * xform[1]);
	}

	std::optional<double> scale_y() const
	{
		const auto xform = transform();
		if (xform.size() < 4u) return std::nullopt;
		return std::sqrt(xform[2] * xform[2] + xform[3] * xform[3]);
	}

	bool set_rotation_angle(const double degrees)
	{
		auto xform = transform();
		if (xform.size() != kTransformDoubles) return false;
		const double sx = std::sqrt(xform[0] * xform[0] + xform[1] * xform[1]);
		const double sy = std::sqrt(xform[2] * xform[2] + xform[3] * xform[3]);
		const double rad = degrees * (3.14159265358979323846 / 180.0);
		const double c = std::cos(rad);
		const double s = std::sin(rad);
		xform[0] = c * sx; xform[1] = s * sx;
		xform[2] = -s * sy; xform[3] = c * sy;
		return set_transform(xform);
	}

	bool set_scale_x(const double sx)
	{
		auto xform = transform();
		if (xform.size() != kTransformDoubles) return false;
		const double old_sx = std::sqrt(xform[0] * xform[0] + xform[1] * xform[1]);
		if (old_sx < 1e-15) return false;
		const double ratio = sx / old_sx;
		xform[0] *= ratio; xform[1] *= ratio;
		return set_transform(xform);
	}

	bool set_scale_y(const double sy)
	{
		auto xform = transform();
		if (xform.size() != kTransformDoubles) return false;
		const double old_sy = std::sqrt(xform[2] * xform[2] + xform[3] * xform[3]);
		if (old_sy < 1e-15) return false;
		const double ratio = sy / old_sy;
		xform[2] *= ratio; xform[3] *= ratio;
		return set_transform(xform);
	}

	bool set_scale(const double sx, const double sy)
	{
		auto xform = transform();
		if (xform.size() != kTransformDoubles) return false;
		const double old_sx = std::sqrt(xform[0] * xform[0] + xform[1] * xform[1]);
		const double old_sy = std::sqrt(xform[2] * xform[2] + xform[3] * xform[3]);
		if (old_sx < 1e-15 || old_sy < 1e-15) return false;
		const double rx = sx / old_sx; const double ry = sy / old_sy;
		xform[0] *= rx; xform[1] *= rx;
		xform[2] *= ry; xform[3] *= ry;
		return set_transform(xform);
	}

	std::pair<double, double> position() const
	{
		auto xform = transform();
		if (xform.size() == kTransformDoubles) return { xform[4], xform[5] };
		return { 0.0, 0.0 };
	}

	bool set_position(const double x, const double y)
	{
		auto xform = transform();
		if (xform.size() != kTransformDoubles) return false;
		xform[4] = x; xform[5] = y;
		return set_transform(xform);
	}

	bool reset_transform()
	{
		auto xform = transform();
		if (xform.size() != kTransformDoubles) return false;
		xform[0] = 1.0; xform[1] = 0.0;
		xform[2] = 0.0; xform[3] = 1.0;
		return set_transform(xform);
	}
};

PSAPI_NAMESPACE_END
