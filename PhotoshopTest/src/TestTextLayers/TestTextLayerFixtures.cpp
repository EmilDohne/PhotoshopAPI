#include "doctest.h"

#include "Core/TaggedBlocks/TaggedBlock.h"
#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"

#include <filesystem>
#include <string>
#include <vector>

namespace
{
	struct FixtureSpec
	{
		std::string file;
		size_t min_text_layers;
	};

	const std::vector<FixtureSpec> kFixtureSpecs
	{
		{ "TextLayers_Basic.psd", 2u },
		{ "TextLayers_Warp.psd", 2u },
		{ "TextLayers_Paragraph.psd", 2u },
		{ "TextLayers_CharacterStyles.psd", 2u },
		{ "TextLayers_StyleRuns.psd", 2u },
		{ "TextLayers_Vertical.psd", 2u },
		{ "TextLayers_Transform.psd", 2u },
		{ "TextLayers_VerticalBox.psd", 3u },
		{ "TextLayers_FontFallback.psd", 1u }
	};

	template <typename T>
	size_t count_text_layers(NAMESPACE_PSAPI::LayeredFile<T>& file)
	{
		size_t count = 0u;
		auto flat_layers = file.flat_layers();
		for (const auto& layer : flat_layers)
		{
			if (std::dynamic_pointer_cast<NAMESPACE_PSAPI::TextLayer<T>>(layer))
			{
				++count;
			}
		}
		return count;
	}

	template <typename T>
	std::shared_ptr<NAMESPACE_PSAPI::TextLayer<T>> find_text_layer_by_name(
		NAMESPACE_PSAPI::LayeredFile<T>& file,
		const std::string& name)
	{
		auto flat_layers = file.flat_layers();
		for (const auto& layer : flat_layers)
		{
			auto text_layer = std::dynamic_pointer_cast<NAMESPACE_PSAPI::TextLayer<T>>(layer);
			if (!text_layer)
			{
				continue;
			}

			if (text_layer->name() == name)
			{
				return text_layer;
			}
		}

		return nullptr;
	}
}


TEST_CASE("Text layer fixtures parse as TextLayer")
{
	using namespace NAMESPACE_PSAPI;

	const auto base_path = std::filesystem::current_path() / "documents" / "TextLayers";
	for (const auto& fixture : kFixtureSpecs)
	{
		const auto path = base_path / fixture.file;
		REQUIRE(std::filesystem::exists(path));

		auto file = LayeredFile<bpp8_t>::read(path);
		CHECK(count_text_layers(file) >= fixture.min_text_layers);
	}
}


TEST_CASE("Text layer fixtures preserve TySh or Txt2 tagged blocks on write")
{
	using namespace NAMESPACE_PSAPI;

	const auto base_path = std::filesystem::current_path() / "documents" / "TextLayers";
	size_t checked_total = 0u;

	for (const auto& fixture : kFixtureSpecs)
	{
		const auto path = base_path / fixture.file;
		REQUIRE(std::filesystem::exists(path));

		auto file = LayeredFile<bpp8_t>::read(path);
		auto flat_layers = file.flat_layers();
		size_t checked_in_file = 0u;

		for (const auto& layer : flat_layers)
		{
			auto text_layer = std::dynamic_pointer_cast<TextLayer<bpp8_t>>(layer);
			if (!text_layer)
			{
				continue;
			}

			auto [record, _] = text_layer->to_photoshop();
			REQUIRE(record.m_AdditionalLayerInfo.has_value());

			const auto tysh = record.m_AdditionalLayerInfo->getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrTypeTool);
			const auto txt2 = record.m_AdditionalLayerInfo->getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrTextEngineData);
			const bool has_text_tag = tysh.has_value() || txt2.has_value();
			CHECK(has_text_tag);
			++checked_in_file;
			++checked_total;
		}

		CHECK(checked_in_file >= fixture.min_text_layers);
	}

	CHECK(checked_total >= 16u);
}


TEST_CASE("Text style fixtures preserve underline and non-path semantics")
{
	using namespace NAMESPACE_PSAPI;

	const auto base_path = std::filesystem::current_path() / "documents" / "TextLayers";

	const auto character_styles_path = base_path / "TextLayers_CharacterStyles.psd";
	REQUIRE(std::filesystem::exists(character_styles_path));
	auto character_styles_file = LayeredFile<bpp8_t>::read(character_styles_path);
	auto character_styles_layer = find_text_layer_by_name(character_styles_file, "CharacterStylePrimary");
	REQUIRE(character_styles_layer != nullptr);

	CHECK_FALSE(character_styles_layer->is_vertical());
	CHECK_FALSE(character_styles_layer->has_warp());
	const auto character_run_underline = character_styles_layer->style_run_underline(0u);
	REQUIRE(character_run_underline.has_value());
	CHECK(character_run_underline.value());

	const auto style_runs_path = base_path / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(style_runs_path));
	auto style_runs_file = LayeredFile<bpp8_t>::read(style_runs_path);
	auto style_runs_layer = find_text_layer_by_name(style_runs_file, "MixedRuns");
	REQUIRE(style_runs_layer != nullptr);

	CHECK_FALSE(style_runs_layer->is_vertical());
	CHECK_FALSE(style_runs_layer->has_warp());
	REQUIRE(style_runs_layer->style_run_count() >= 5u);

	const auto run2_underline = style_runs_layer->style_run_underline(2u);
	REQUIRE(run2_underline.has_value());
	CHECK(run2_underline.value());

	const auto run2_bold = style_runs_layer->style_run_faux_bold(2u);
	REQUIRE(run2_bold.has_value());
	CHECK(run2_bold.value());

	const auto run4_italic = style_runs_layer->style_run_faux_italic(4u);
	REQUIRE(run4_italic.has_value());
	CHECK(run4_italic.value());
}
