#pragma once

// =========================================================================
//  TextLayerRunSplitUtils.h
//
//  EngineData run splitting and run-length query helpers for text layers.
// =========================================================================

#include "Macros.h"
#include "TextLayerEngineDataUtils.h"
#include "TextLayerParsingUtils.h"

#include "Core/Struct/EngineDataStructure.h"
#include "Core/TaggedBlocks/TaggedBlock.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

PSAPI_NAMESPACE_BEGIN

namespace TextLayerDetail
{
// -----------------------------------------------------------------------
//  Split a style run at a character offset
// -----------------------------------------------------------------------

/// Split the style run at run_index at the given character offset (relative
/// to the start of that run).  After splitting, the original run covers
/// [0, char_offset) and the new run covers [char_offset, run_length).
/// The new run inherits all style properties from the original.
/// Returns true on success.
inline bool split_style_run(TaggedBlock& block, size_t run_index, size_t char_offset)
{
	const auto engine_span_opt = find_engine_data_span(block);
	if (!engine_span_opt.has_value()) return false;

	std::vector<std::byte> payload(
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset),
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset + engine_span_opt->byte_count)
	);

	auto parsed = EngineData::parse(payload);
	if (!parsed.ok) return false;

	// Get style run arrays
	auto style_run = EngineData::find_by_path(parsed.root, { "EngineDict", "StyleRun" });
	if (style_run == nullptr) return false;

	auto run_array = EngineData::find_dict_value(*style_run, "RunArray");
	if (run_array == nullptr || run_array->type != EngineData::ValueType::Array) return false;

	auto run_length_array = EngineData::find_dict_value(*style_run, "RunLengthArray");
	if (run_length_array == nullptr || run_length_array->type != EngineData::ValueType::Array) return false;

	if (run_index >= run_array->array_items.size()) return false;
	if (run_index >= run_length_array->array_items.size()) return false;

	// Get current run length
	std::vector<int32_t> run_lengths;
	if (!EngineData::as_int32_vector(*run_length_array, run_lengths)) return false;
	if (run_index >= run_lengths.size()) return false;

	const int32_t current_length = run_lengths[run_index];
	if (char_offset == 0 || static_cast<int32_t>(char_offset) >= current_length) return false;

	// Deep-copy the run entry for the new (right) half
	auto new_run = run_array->array_items[run_index];  // copy

	// Update run lengths: split current_length into two
	const int32_t left_length = static_cast<int32_t>(char_offset);
	const int32_t right_length = current_length - left_length;

	// Modify the tree
	run_lengths[run_index] = left_length;
	run_lengths.insert(run_lengths.begin() + static_cast<std::ptrdiff_t>(run_index + 1), right_length);

	// Insert the new run entry after the current one
	run_array->array_items.insert(
		run_array->array_items.begin() + static_cast<std::ptrdiff_t>(run_index + 1),
		std::move(new_run)
	);

	// Update run_length_array
	EngineData::set_int32_array(*run_length_array, run_lengths);

	// Re-serialize the entire EngineData tree
	auto new_payload = EngineData::serialize(parsed.root);
	return write_engine_payload(block, engine_span_opt.value(), new_payload);
}

/// Split a paragraph run similarly.
inline bool split_paragraph_run(TaggedBlock& block, size_t run_index, size_t char_offset)
{
	const auto engine_span_opt = find_engine_data_span(block);
	if (!engine_span_opt.has_value()) return false;

	std::vector<std::byte> payload(
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset),
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset + engine_span_opt->byte_count)
	);

	auto parsed = EngineData::parse(payload);
	if (!parsed.ok) return false;

	auto para_run = EngineData::find_by_path(parsed.root, { "EngineDict", "ParagraphRun" });
	if (para_run == nullptr) return false;

	auto run_array = EngineData::find_dict_value(*para_run, "RunArray");
	if (run_array == nullptr || run_array->type != EngineData::ValueType::Array) return false;

	auto run_length_array = EngineData::find_dict_value(*para_run, "RunLengthArray");
	if (run_length_array == nullptr || run_length_array->type != EngineData::ValueType::Array) return false;

	if (run_index >= run_array->array_items.size()) return false;
	if (run_index >= run_length_array->array_items.size()) return false;

	std::vector<int32_t> run_lengths;
	if (!EngineData::as_int32_vector(*run_length_array, run_lengths)) return false;
	if (run_index >= run_lengths.size()) return false;

	const int32_t current_length = run_lengths[run_index];
	if (char_offset == 0 || static_cast<int32_t>(char_offset) >= current_length) return false;

	auto new_run = run_array->array_items[run_index];

	const int32_t left_length = static_cast<int32_t>(char_offset);
	const int32_t right_length = current_length - left_length;

	run_lengths[run_index] = left_length;
	run_lengths.insert(run_lengths.begin() + static_cast<std::ptrdiff_t>(run_index + 1), right_length);

	run_array->array_items.insert(
		run_array->array_items.begin() + static_cast<std::ptrdiff_t>(run_index + 1),
		std::move(new_run)
	);

	EngineData::set_int32_array(*run_length_array, run_lengths);

	auto new_payload = EngineData::serialize(parsed.root);
	return write_engine_payload(block, engine_span_opt.value(), new_payload);
}

/// Get the run lengths as a vector of int32.
inline std::optional<std::vector<int32_t>> get_style_run_lengths(const TaggedBlock& block)
{
	const auto payload = read_engine_payload(block);
	if (!payload.has_value()) return std::nullopt;

	auto parsed = EngineData::parse(*payload);
	if (!parsed.ok) return std::nullopt;

	auto run_length_array = EngineData::find_by_path(parsed.root, { "EngineDict", "StyleRun", "RunLengthArray" });
	if (run_length_array == nullptr || run_length_array->type != EngineData::ValueType::Array) return std::nullopt;

	std::vector<int32_t> run_lengths;
	if (!EngineData::as_int32_vector(*run_length_array, run_lengths)) return std::nullopt;
	return run_lengths;
}

} // namespace TextLayerDetail

PSAPI_NAMESPACE_END

