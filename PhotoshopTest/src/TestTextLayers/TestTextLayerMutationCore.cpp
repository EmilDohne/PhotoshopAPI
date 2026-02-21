#include "doctest.h"
#include "TestTextLayerMutationUtils.h"

TEST_CASE("TextLayer can read text payload from fixture descriptor")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer);
	CHECK(text_layer->text().has_value());
	CHECK(text_layer->text().value() == "Hello 123");
}


TEST_CASE("TextLayer equal-length replacement survives write/read roundtrip")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer);

	CHECK(text_layer->replace_text_equal_length("Hello", "Hallo"));
	CHECK(text_layer->text().has_value());
	CHECK(text_layer->text().value() == "Hallo 123");

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_with_contents(reread, "Hallo 123");
	CHECK(reread_layer != nullptr);

	std::filesystem::remove(out_path);
}


TEST_CASE("TextLayer rejects non-equal-length replacements")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer);

	CHECK_FALSE(text_layer->replace_text_equal_length("Hello", "Greetings"));
	CHECK_FALSE(text_layer->set_text_equal_length("Longer than nine"));

	CHECK(text_layer->text().has_value());
	CHECK(text_layer->text().value() == "Hello 123");
}


TEST_CASE("TextLayer variable-length replacement survives write/read roundtrip")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(text_layer);

	CHECK(text_layer->replace_text("Hello", "Greetings"));
	CHECK(text_layer->text().has_value());
	CHECK(text_layer->text().value() == "Greetings 123");

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_with_contents(reread, "Greetings 123");
	CHECK(reread_layer != nullptr);

	std::filesystem::remove(out_path);
}


TEST_CASE("TextLayer remaps EngineData run lengths for style runs")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer);

	CHECK(text_layer->replace_text("Beta", "Betaaaa"));
	CHECK(text_layer->text().has_value());
	CHECK(text_layer->text().value() == "Alpha Betaaaa Gamma");

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_with_contents(reread, "Alpha Betaaaa Gamma");
	REQUIRE(reread_layer != nullptr);

	auto [reread_record, _reread_channel_data] = reread_layer->to_photoshop();
	REQUIRE(reread_record.m_AdditionalLayerInfo.has_value());
	const auto reread_tysh = reread_record.m_AdditionalLayerInfo->getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrTypeTool);
	REQUIRE(reread_tysh.has_value());

	const auto reread_arrays = extract_engine_run_length_arrays(*reread_tysh.value());
	CHECK(std::find(reread_arrays.begin(), reread_arrays.end(), std::vector<int32_t>{ 5, 1, 7, 1, 6 }) != reread_arrays.end());
	CHECK(std::find(reread_arrays.begin(), reread_arrays.end(), std::vector<int32_t>{ 20 }) != reread_arrays.end());

	std::filesystem::remove(out_path);
}



TEST_CASE("TextLayer legacy From/To remap: variable-length mutation on multi-style file roundtrips")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer);

	// Grow mid-run text so indices shift
	CHECK(text_layer->replace_text("Beta", "BetaBetaBeta"));
	CHECK(text_layer->text().value() == "Alpha BetaBetaBeta Gamma");

	// Write and re-read
	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_with_contents(reread, "Alpha BetaBetaBeta Gamma");
	REQUIRE(reread_layer != nullptr);

	// Verify EngineData run lengths are coherent.
	auto [rec, _] = reread_layer->to_photoshop();
	REQUIRE(rec.m_AdditionalLayerInfo.has_value());
	const auto tysh = rec.m_AdditionalLayerInfo->getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrTypeTool);
	REQUIRE(tysh.has_value());

	const auto arrays = extract_engine_run_length_arrays(*tysh.value());
	// Original: "Alpha Beta Gamma" -> runs [5, 1, 4, 1, 6] -> mutated "Alpha BetaBetaBeta Gamma" -> [5, 1, 12, 1, 6]
	// Plus trailing sentinel. Total = 5+1+12+1+6 = 25 (24 visible + 1 trailing?)
	CHECK(std::find(arrays.begin(), arrays.end(), std::vector<int32_t>{ 5, 1, 12, 1, 6 }) != arrays.end());

	// Verify style run count is preserved
	CHECK(reread_layer->style_run_count() == 5u);

	std::filesystem::remove(out_path);
}


TEST_CASE("TextLayer legacy From/To remap: shrinking mutation on multi-style file roundtrips")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer);

	// Shrink: "Alpha" -> "A"
	CHECK(text_layer->replace_text("Alpha", "A"));
	CHECK(text_layer->text().value() == "A Beta Gamma");

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_with_contents(reread, "A Beta Gamma");
	REQUIRE(reread_layer != nullptr);

	// Verify EngineData run lengths: original [5,1,4,1,6] -> after "Alpha"->"A": [1,1,4,1,6]
	auto [rec, _] = reread_layer->to_photoshop();
	REQUIRE(rec.m_AdditionalLayerInfo.has_value());
	const auto tysh = rec.m_AdditionalLayerInfo->getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrTypeTool);
	REQUIRE(tysh.has_value());

	const auto arrays = extract_engine_run_length_arrays(*tysh.value());
	CHECK(std::find(arrays.begin(), arrays.end(), std::vector<int32_t>{ 1, 1, 4, 1, 6 }) != arrays.end());

	CHECK(reread_layer->style_run_count() == 5u);

	std::filesystem::remove(out_path);
}


TEST_CASE("TextLayer legacy From/To remap: no-op on same-length mutation preserves TySh descriptor")
{
	using namespace NAMESPACE_PSAPI;

	const auto fixture_path = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture_path));

	auto file = LayeredFile<bpp8_t>::read(fixture_path);
	auto text_layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(text_layer);

	// Same-length substitution: "Beta" -> "XXXX"
	CHECK(text_layer->replace_text("Beta", "XXXX"));
	CHECK(text_layer->text().value() == "Alpha XXXX Gamma");

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_with_contents(reread, "Alpha XXXX Gamma");
	REQUIRE(reread_layer != nullptr);

	// Run lengths unchanged: [5, 1, 4, 1, 6]
	auto [rec, _] = reread_layer->to_photoshop();
	REQUIRE(rec.m_AdditionalLayerInfo.has_value());
	const auto tysh = rec.m_AdditionalLayerInfo->getTaggedBlock<TaggedBlock>(Enum::TaggedBlockKey::lrTypeTool);
	REQUIRE(tysh.has_value());

	const auto arrays = extract_engine_run_length_arrays(*tysh.value());
	CHECK(std::find(arrays.begin(), arrays.end(), std::vector<int32_t>{ 5, 1, 4, 1, 6 }) != arrays.end());

	CHECK(reread_layer->style_run_count() == 5u);

	std::filesystem::remove(out_path);
}

