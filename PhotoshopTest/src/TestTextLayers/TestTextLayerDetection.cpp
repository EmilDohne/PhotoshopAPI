#include "doctest.h"

#include "Core/TaggedBlocks/TaggedBlock.h"
#include "LayeredFile/Impl/LayeredFileImpl.h"
#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <optional>
#include <vector>

namespace
{
	std::filesystem::path generate_temp_path()
	{
		static std::atomic<uint64_t> counter{ 0u };
		const auto stamp = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
		const auto idx = counter.fetch_add(1u, std::memory_order_relaxed);
		return std::filesystem::temp_directory_path() / ("psapi_taggedblock_" + std::to_string(stamp) + "_" + std::to_string(idx) + ".bin");
	}

	std::shared_ptr<NAMESPACE_PSAPI::TaggedBlock> make_empty_base_tagged_block(const NAMESPACE_PSAPI::Enum::TaggedBlockKey key)
	{
		using namespace NAMESPACE_PSAPI;

		const auto tmp_path = generate_temp_path();
		{
			std::ofstream out(tmp_path, std::ios::binary);
			const uint8_t zero_len[4] = { 0u, 0u, 0u, 0u };
			out.write(reinterpret_cast<const char*>(zero_len), sizeof(zero_len));
		}

		auto block = std::make_shared<TaggedBlock>();
		{
			File file(tmp_path);
			FileHeader header(Enum::Version::Psd, 3u, 1u, 1u, Enum::BitDepth::BD_8, Enum::ColorMode::RGB);
			block->read(file, header, 0u, Signature("8BIM"), key, 1u);
		}

		std::filesystem::remove(tmp_path);
		return block;
	}

	NAMESPACE_PSAPI::LayerRecord make_layer_record_with_blocks(std::vector<std::shared_ptr<NAMESPACE_PSAPI::TaggedBlock>> blocks)
	{
		using namespace NAMESPACE_PSAPI;

		TaggedBlockStorage block_storage(std::move(blocks));
		AdditionalLayerInfo additional_layer_info(block_storage);
		std::optional<AdditionalLayerInfo> additional_info = std::move(additional_layer_info);

		return LayerRecord(
			PascalString("TextLayer", 4u),
			0,
			0,
			0,
			0,
			0u,
			{},
			Enum::BlendMode::Normal,
			255u,
			0u,
			LayerRecords::BitFlags(false, false, false),
			std::nullopt,
			LayerRecords::LayerBlendingRanges{},
			std::move(additional_info)
		);
	}
}


TEST_CASE("Text layer detection from Txt2 tagged block")
{
	using namespace NAMESPACE_PSAPI;

	auto txt2_block = make_empty_base_tagged_block(Enum::TaggedBlockKey::lrTextEngineData);
	auto layer_record = make_layer_record_with_blocks({ txt2_block });

	ChannelImageData channel_image_data{};
	LayeredFile<bpp8_t> layered_file(Enum::ColorMode::RGB, 1u, 1u);
	FileHeader header(Enum::Version::Psd, 3u, 1u, 1u, Enum::BitDepth::BD_8, Enum::ColorMode::RGB);
	AdditionalLayerInfo global_additional_layer_info{};

	const auto layer = _Impl::identify_layer_type<bpp8_t>(
		layered_file,
		layer_record,
		channel_image_data,
		header,
		global_additional_layer_info
	);

	REQUIRE(layer);
	REQUIRE(std::dynamic_pointer_cast<TextLayer<bpp8_t>>(layer));
}


TEST_CASE("Text layer preserves Txt2 tagged block on write")
{
	using namespace NAMESPACE_PSAPI;

	auto txt2_block = make_empty_base_tagged_block(Enum::TaggedBlockKey::lrTextEngineData);
	auto layer_record = make_layer_record_with_blocks({ txt2_block });

	ChannelImageData channel_image_data{};
	LayeredFile<bpp8_t> layered_file(Enum::ColorMode::RGB, 1u, 1u);
	FileHeader header(Enum::Version::Psd, 3u, 1u, 1u, Enum::BitDepth::BD_8, Enum::ColorMode::RGB);
	AdditionalLayerInfo global_additional_layer_info{};

	const auto layer = _Impl::identify_layer_type<bpp8_t>(
		layered_file,
		layer_record,
		channel_image_data,
		header,
		global_additional_layer_info
	);
	auto text_layer = std::dynamic_pointer_cast<TextLayer<bpp8_t>>(layer);
	REQUIRE(text_layer);

	auto out_data = text_layer->to_photoshop();
	auto& out_layer_record = std::get<0>(out_data);
	REQUIRE(out_layer_record.m_AdditionalLayerInfo.has_value());

	const auto txt2_out = out_layer_record.m_AdditionalLayerInfo.value().getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrTextEngineData);
	CHECK(txt2_out.has_value());
}
