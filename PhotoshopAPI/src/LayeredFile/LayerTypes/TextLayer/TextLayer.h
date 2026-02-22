#pragma once

// =========================================================================
//  TextLayer.h  –  Slim facade (CRTP mixin decomposition)
//
//  The heavy implementation lives in the TextLayer/ sub-headers.
//  This file keeps only:
//    - struct TextLayer<T> definition (inherits Layer<T> + 8 CRTP mixins)
//    - Core text operations (text / set_text / replace_text / …)
//    - constructor, to_photoshop(), and private helpers
//    - extern template declarations
// =========================================================================

#include "Macros.h"
#include "LayeredFile/LayerTypes/Layer.h"
#include "LayeredFile/concepts.h"
#include "Core/Struct/UnicodeString.h"

// Mixin headers (each brings in exactly what it needs)
#include "TextLayerFwd.h"
#include "TextLayerTypes.h"
#include "TextLayerParsingUtils.h"
#include "TextLayerU16Utils.h"
#include "TextLayerDescriptorUtils.h"
#include "TextLayerEngineDataUtils.h"
#include "TextLayerRemapUtils.h"
#include "TextLayerBuilder.h"
#include "TextLayerFontMixin.h"
#include "TextLayerStyleRunMixin.h"
#include "TextLayerParagraphRunMixin.h"
#include "TextLayerStyleNormalMixin.h"
#include "TextLayerParagraphNormalMixin.h"
#include "TextLayerTransformMixin.h"
#include "TextLayerShapeMixin.h"
#include "TextLayerWarpMixin.h"
#include "TextLayerProxiesMixin.h"
#include "TextLayerRangeStyleMixin.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

PSAPI_NAMESPACE_BEGIN

/// This struct holds no data, we just use it to identify its type
template <typename T>
	requires concepts::bit_depth<T>
struct TextLayer :
	Layer<T>,
	TextLayerFontMixin<TextLayer<T>>,
	TextLayerStyleRunMixin<TextLayer<T>>,
	TextLayerParagraphRunMixin<TextLayer<T>>,
	TextLayerStyleNormalMixin<TextLayer<T>>,
	TextLayerParagraphNormalMixin<TextLayer<T>>,
	TextLayerTransformMixin<TextLayer<T>>,
	TextLayerShapeMixin<TextLayer<T>>,
	TextLayerWarpMixin<TextLayer<T>>,
	TextLayerProxiesMixin<TextLayer<T>>,
	TextLayerRangeStyleMixin<TextLayer<T>>
{
	// Grant every mixin access to the private text_tagged_blocks() method.
	friend struct TextLayerFontMixin<TextLayer<T>>;
	friend struct TextLayerStyleRunMixin<TextLayer<T>>;
	friend struct TextLayerParagraphRunMixin<TextLayer<T>>;
	friend struct TextLayerStyleNormalMixin<TextLayer<T>>;
	friend struct TextLayerParagraphNormalMixin<TextLayer<T>>;
	friend struct TextLayerTransformMixin<TextLayer<T>>;
	friend struct TextLayerShapeMixin<TextLayer<T>>;
	friend struct TextLayerWarpMixin<TextLayer<T>>;
	friend struct TextLayerProxiesMixin<TextLayer<T>>;
	friend struct TextLayerRangeStyleMixin<TextLayer<T>>;

	using Layer<T>::Layer;

	TextLayer() = default;

	// =================================================================
	//  Constructor: create a TextLayer from scratch
	// =================================================================

	/// Construct a new TextLayer from scratch with the given layer parameters,
	/// text content and font settings. This constructs a minimal valid TySh
	/// tagged block so the layer can be added to a LayeredFile and written to disk.
	TextLayer(
		Layer<T>::Params parameters,
		const std::string& text,
		const std::string& font_postscript_name = "ArialMT",
		double font_size = 24.0,
		const std::vector<double>& fill_color = { 1.0, 0.0, 0.0, 0.0 },
		double position_x = 20.0,
		double position_y = 50.0,
		double box_width  = 0.0,
		double box_height = 0.0)
	{
		Layer<T>::m_ColorMode  = parameters.colormode;
		Layer<T>::m_LayerName  = parameters.name;
		Layer<T>::m_BlendMode  = parameters.blendmode;
		Layer<T>::m_Opacity    = parameters.opacity;
		Layer<T>::m_IsVisible  = parameters.visible;
		Layer<T>::m_IsLocked   = parameters.locked;
		Layer<T>::m_CenterX    = static_cast<float>(parameters.center_x);
		Layer<T>::m_CenterY    = static_cast<float>(parameters.center_y);
		Layer<T>::m_Width      = parameters.width;
		Layer<T>::m_Height     = parameters.height;
		Layer<T>::m_IsClippingMask = parameters.clipping_mask;

		Layer<T>::parse_mask(parameters);

		// Build and inject the TySh tagged block
		auto tysh_block = TextLayerDetail::build_tysh_tagged_block(
			text, font_postscript_name, font_size, fill_color,
			position_x, position_y, box_width, box_height);
		Layer<T>::m_UnparsedBlocks.push_back(std::move(tysh_block));
	}

	/// Convenience factory that builds a minimal TextLayer without a full Params struct.
	/// Prefer the constructor with \c Layer<T>::Params for new code.
	static std::shared_ptr<TextLayer<T>> create(
		const std::string& layer_name,
		const std::string& text,
		const std::string& font_postscript_name = "ArialMT",
		double font_size = 24.0,
		const std::vector<double>& fill_color = { 1.0, 0.0, 0.0, 0.0 },
		double position_x = 20.0,
		double position_y = 50.0,
		double box_width  = 0.0,
		double box_height = 0.0)
	{
		typename Layer<T>::Params params{};
		params.name = layer_name;
		return std::make_shared<TextLayer<T>>(
			std::move(params), text, font_postscript_name, font_size,
			fill_color, position_x, position_y, box_width, box_height);
	}

	// =================================================================
	//  Anti-aliasing
	// =================================================================

	/// Read the anti-aliasing method from the TySh text descriptor ("AntA" key).
	std::optional<TextLayerEnum::AntiAliasMethod> anti_alias() const
	{
		for (const auto& block : text_tagged_blocks())
		{
			auto val = TextLayerDetail::read_text_desc_enum(*block, "AntA");
			if (!val.has_value()) continue;
			const auto& s = val.value();
			// Accept both 4-byte charIDs and long stringIDs.
			// Sharp has no charID – Photoshop writes "antiAliasSharp".
			if (s == "Anno" || s == "antiAliasNone")     return TextLayerEnum::AntiAliasMethod::None;
			if (s == "AnCr" || s == "antiAliasCrisp")    return TextLayerEnum::AntiAliasMethod::Crisp;
			if (s == "AnSt" || s == "antiAliasStrong")   return TextLayerEnum::AntiAliasMethod::Strong;
			if (s == "AnSm" || s == "antiAliasSmooth")   return TextLayerEnum::AntiAliasMethod::Smooth;
			if (s == "antiAliasSharp")                    return TextLayerEnum::AntiAliasMethod::Sharp;
			// AnLo/AnMd/AnHi (Low/Medium/High) are Photoshop-internal values not
			// accessible from the UI. Return nullopt so callers treat them as unknown.
			return std::nullopt;
		}
		return std::nullopt;
	}

	/// Set the anti-aliasing method in the TySh text descriptor ("AntA" key).
	void set_anti_alias(TextLayerEnum::AntiAliasMethod method)
	{
		// Use the canonical code that Photoshop writes for each value.
		// Sharp has no charID – must use the long stringID "antiAliasSharp".
		std::string code;
		switch (method)
		{
			case TextLayerEnum::AntiAliasMethod::None:   code = "Anno"; break;
			case TextLayerEnum::AntiAliasMethod::Crisp:  code = "AnCr"; break;
			case TextLayerEnum::AntiAliasMethod::Strong: code = "AnSt"; break;
			case TextLayerEnum::AntiAliasMethod::Smooth: code = "AnSm"; break;
			case TextLayerEnum::AntiAliasMethod::Sharp:  code = "antiAliasSharp"; break;
			default:
				throw std::invalid_argument("TextLayer::set_anti_alias() received an unsupported AntiAliasMethod enum value");
		}
		auto blocks = text_tagged_blocks();
		if (blocks.empty())
		{
			throw std::runtime_error("TextLayer::set_anti_alias() failed: no lrTypeTool tagged block found on this layer");
		}

		size_t modified = 0u;
		for (const auto& block : blocks)
		{
			if (TextLayerDetail::write_text_desc_enum(*block, "AntA", code))
			{
				++modified;
			}
		}
		if (modified == 0u)
		{
			throw std::runtime_error("TextLayer::set_anti_alias() failed: unable to write AntA in TySh text descriptor");
		}
	}

	// =================================================================
	//  Style run splitting
	// =================================================================

	/// Split the style run at run_index at the given UTF-16 code-unit offset
	/// relative to the start of that run.  After splitting, the original run
	/// covers [0, char_offset) and the new run covers [char_offset, end).
	/// The new run inherits all style properties from the original.
	/// Both style and paragraph runs are split at the same offset.
	void split_style_run(size_t run_index, size_t char_offset)
	{
		auto blocks = text_tagged_blocks();
		if (blocks.empty())
		{
			throw std::runtime_error("TextLayer::split_style_run() failed: no lrTypeTool tagged block found on this layer");
		}

		for (size_t i = 0u; i < blocks.size(); ++i)
		{
			if (!TextLayerDetail::split_style_run(*blocks[i], run_index, char_offset))
			{
				throw std::runtime_error(
					"TextLayer::split_style_run() failed: invalid split or descriptor mutation failed for run_index/char_offset");
			}
		}
	}

	/// Get the style run lengths as a list of code-unit counts.
	std::optional<std::vector<int32_t>> style_run_lengths() const
	{
		for (const auto& block : text_tagged_blocks())
		{
			auto result = TextLayerDetail::get_style_run_lengths(*block);
			if (result.has_value()) return result;
		}
		return std::nullopt;
	}

	/// Split a paragraph run at the given char offset (analogous to split_style_run).
	void split_paragraph_run(size_t run_index, size_t char_offset)
	{
		auto blocks = text_tagged_blocks();
		if (blocks.empty())
		{
			throw std::runtime_error("TextLayer::split_paragraph_run() failed: no lrTypeTool tagged block found on this layer");
		}
		for (size_t i = 0u; i < blocks.size(); ++i)
		{
			if (!TextLayerDetail::split_paragraph_run(*blocks[i], run_index, char_offset))
			{
				throw std::runtime_error(
					"TextLayer::split_paragraph_run() failed: invalid split or descriptor mutation failed for run_index/char_offset");
			}
		}
	}

	/// Get the paragraph run lengths as a list of code-unit counts.
	std::optional<std::vector<int32_t>> paragraph_run_lengths() const
	{
		for (const auto& block : text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			auto run_length_array = EngineData::find_by_path(
				parsed.root, { "EngineDict", "ParagraphRun", "RunLengthArray" });
			if (run_length_array == nullptr || run_length_array->type != EngineData::ValueType::Array)
				continue;
			std::vector<int32_t> run_lengths;
			if (!EngineData::as_int32_vector(*run_length_array, run_lengths))
				continue;
			return run_lengths;
		}
		return std::nullopt;
	}

	// =================================================================
	//  Core text read / write
	// =================================================================

	/// Retrieve the text payload from the first available text descriptor.
	std::optional<std::string> text() const
	{
		for (const auto& block : text_tagged_blocks())
		{
			const auto span = TextLayerDetail::parse_text_span(*block);
			if (!span.has_value())
			{
				continue;
			}

			return span->text_utf8;
		}

		return std::nullopt;
	}

	/// Set text while preserving descriptor metadata and remapping style ranges.
	bool set_text(const std::string& new_text)
	{
		const auto new_utf16 = UnicodeString::convertUTF8ToUTF16LE(new_text);

		auto blocks = text_tagged_blocks();
		if (blocks.empty())
		{
			return false;
		}

		bool changed_any = false;
		bool found_any = false;

		for (const auto& block : blocks)
		{
			auto parsed = TextLayerDetail::parse_text_span(*block);
			if (!parsed.has_value())
			{
				continue;
			}
			found_any = true;

			if (parsed->text_utf16 == new_utf16)
			{
				continue;
			}

			std::vector<TextLayerDetail::TextReplacement> replacements{};
			replacements.push_back(TextLayerDetail::TextReplacement{ 0u, parsed->text_utf16.size(), new_utf16.size() });

			if (TextLayerDetail::apply_text_mutation(*block, parsed.value(), new_utf16, replacements))
			{
				changed_any = true;
			}
		}

		return found_any && changed_any;
	}

	/// Replace text occurrences while preserving descriptor metadata and remapping style ranges.
	bool replace_text(const std::string& old_text, const std::string& new_text, const bool replace_all = true)
	{
		const auto old_utf16 = UnicodeString::convertUTF8ToUTF16LE(old_text);
		const auto new_utf16 = UnicodeString::convertUTF8ToUTF16LE(new_text);
		if (old_utf16.empty())
		{
			return false;
		}

		auto blocks = text_tagged_blocks();
		if (blocks.empty())
		{
			return false;
		}

		bool changed_any = false;
		for (const auto& block : blocks)
		{
			auto parsed = TextLayerDetail::parse_text_span(*block);
			if (!parsed.has_value())
			{
				continue;
			}

			auto replaced = TextLayerDetail::replace_utf16(parsed->text_utf16, old_utf16, new_utf16, replace_all);
			if (!replaced.has_value())
			{
				continue;
			}

			if (TextLayerDetail::apply_text_mutation(*block, parsed.value(), replaced->text_utf16, replaced->replacements))
			{
				changed_any = true;
			}
		}

		return changed_any;
	}

	/// Set text while preserving descriptor metadata. This strict variant requires equal UTF-16 code-unit length.
	bool set_text_equal_length(const std::string& new_text)
	{
		auto blocks = text_tagged_blocks();
		if (blocks.empty())
		{
			return false;
		}

		const auto new_utf16_len = UnicodeString::convertUTF8ToUTF16LE(new_text).size();
		bool found_any = false;
		for (const auto& block : blocks)
		{
			auto parsed = TextLayerDetail::parse_text_span(*block);
			if (!parsed.has_value())
			{
				continue;
			}
			found_any = true;

			if (parsed->text_utf16_length != new_utf16_len)
			{
				return false;
			}
		}

		if (!found_any)
		{
			return false;
		}
		return set_text(new_text);
	}

	/// Replace text occurrences while preserving descriptor metadata. This strict variant requires equal UTF-16 code-unit length.
	bool replace_text_equal_length(const std::string& old_text, const std::string& new_text, const bool replace_all = true)
	{
		if (old_text.empty())
		{
			return false;
		}

		const auto old_utf16_len = UnicodeString::convertUTF8ToUTF16LE(old_text).size();
		const auto new_utf16_len = UnicodeString::convertUTF8ToUTF16LE(new_text).size();
		if (old_utf16_len != new_utf16_len)
		{
			return false;
		}

		return replace_text(old_text, new_text, replace_all);
	}

	// =================================================================
	//  Roundtrip helpers
	// =================================================================

	void generate_channel_image_data_from_read(
		ChannelImageData& channel_image_data,
		const std::vector<LayerRecords::ChannelInformation>& channel_infos)
	{
		for (const auto& channel_info : channel_infos)
		{
			auto channelPtr = channel_image_data.extract_image_ptr(channel_info.m_ChannelID);
			// Pointers might have already been released previously
			if (!channelPtr) continue;

			// Insert any valid pointers to channels we have. We move to avoid having
			// to uncompress / recompress
			Layer<T>::m_UnparsedImageData[channel_info.m_ChannelID] = std::move(channelPtr);
		}
	}

	std::tuple<LayerRecord, ChannelImageData> to_photoshop() override
	{
		PascalString name = Layer<T>::generate_name();
		ChannelExtents extents = generate_extents(ChannelCoordinates(Layer<T>::m_Width, Layer<T>::m_Height, Layer<T>::m_CenterX, Layer<T>::m_CenterY));

		LayerRecords::BitFlags bit_flags(Layer<T>::m_IsLocked, !Layer<T>::m_IsVisible, false);
		std::optional<LayerRecords::LayerMaskData> lr_mask_data = Layer<T>::internal_generate_mask_data();
		LayerRecords::LayerBlendingRanges blending_ranges = Layer<T>::generate_blending_ranges();

		// Generate our AdditionalLayerInfoSection. We dont need any special Tagged Blocks besides what is stored by the generic layer
		auto block_vec = this->generate_tagged_blocks();
		std::optional<AdditionalLayerInfo> tagged_blocks = std::nullopt;
		if (block_vec.size() > 0)
		{
			TaggedBlockStorage blockStorage = { block_vec };
			tagged_blocks.emplace(AdditionalLayerInfo(blockStorage));
		}

		// Initialize the channel information as well as the channel image data, the size held in the channelInfo might change depending on
		// the compression mode chosen on export and must therefore be updated later. This step is done last as generateChannelImageData() invalidates
		// all image data which we might need for operations above
		auto num_chan = this->num_channels(true);

		auto channel_data = this->generate_channel_image_data();
		auto& channel_info = std::get<0>(channel_data);
		ChannelImageData channel_img_data = std::move(std::get<1>(channel_data));

		LayerRecord lr_record = LayerRecord(
			name,
			extents.top,
			extents.left,
			extents.bottom,
			extents.right,
			static_cast<uint16_t>(num_chan),
			channel_info,
			Layer<T>::m_BlendMode,
			Layer<T>::m_Opacity,
			static_cast<uint8_t>(Layer<T>::m_IsClippingMask),
			bit_flags,
			lr_mask_data,
			blending_ranges,
			std::move(tagged_blocks)
		);
		return std::make_tuple(std::move(lr_record), std::move(channel_img_data));
	}


private:

	// -----------------------------------------------------------------
	//  Private helpers
	// -----------------------------------------------------------------

	std::vector<std::shared_ptr<TaggedBlock>> text_tagged_blocks() const
	{
		std::vector<std::shared_ptr<TaggedBlock>> out;
		for (const auto& block : Layer<T>::m_UnparsedBlocks)
		{
			if (block && block->getKey() == Enum::TaggedBlockKey::lrTypeTool)
			{
				out.push_back(block);
			}
		}
		return out;
	}

	size_t num_channels(bool include_mask) const
	{
		if (Layer<T>::has_mask() && include_mask)
		{
			return Layer<T>::m_UnparsedImageData.size() + 1;
		}
		return Layer<T>::m_UnparsedImageData.size();
	}

	std::tuple<std::vector<LayerRecords::ChannelInformation>, ChannelImageData> generate_channel_image_data()
	{
		std::vector<LayerRecords::ChannelInformation> channel_info;
		std::vector<std::unique_ptr<channel_wrapper>> channel_data;

		// First extract our mask data, the order of our channels does not matter as long as the
		// order of channelInfo and channelData is the same
		auto mask_data = Layer<T>::internal_extract_mask();
		if (mask_data.has_value())
		{
			channel_info.push_back(std::get<0>(mask_data.value()));
			channel_data.push_back(std::move(std::get<1>(mask_data.value())));
		}

		// Extract all the channels next and push them into our data representation
		for (auto& [id, channel] : Layer<T>::m_UnparsedImageData)
		{
			channel_info.push_back(LayerRecords::ChannelInformation{ id, channel->byte_size() });
			channel_data.push_back(std::move(channel));
		}

		// Construct the channel image data from our vector of ptrs, moving gets handled by the constructor
		ChannelImageData channel_image_data(std::move(channel_data));
		return std::make_tuple(channel_info, std::move(channel_image_data));
	}
};


extern template struct TextLayer<bpp8_t>;
extern template struct TextLayer<bpp16_t>;
extern template struct TextLayer<bpp32_t>;

PSAPI_NAMESPACE_END
