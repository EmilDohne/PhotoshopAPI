#include "doctest.h"

#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"

#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <string>

namespace
{
	std::filesystem::path temp_psd_path()
	{
		static std::atomic<uint64_t> counter{ 0u };
		const auto stamp = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
		const auto idx = counter.fetch_add(1u, std::memory_order_relaxed);
		return std::filesystem::temp_directory_path() / ("psapi_text_shape_" + std::to_string(stamp) + "_" + std::to_string(idx) + ".psd");
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
			if (text_layer && text_layer->name() == name)
			{
				return text_layer;
			}
		}
		return nullptr;
	}

	const auto kFixturePath = std::filesystem::path("documents") / "TextLayers" / "TextLayers_VerticalBox.psd";
}


// ---------------------------------------------------------------------------
//  shape_type / is_box_text / is_point_text
// ---------------------------------------------------------------------------

TEST_CASE("Shape: box text layer reports ShapeType::BoxText")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kFixturePath;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "HorizontalBoxControl");
	REQUIRE(layer != nullptr);

	const auto st = layer->shape_type();
	REQUIRE(st.has_value());
	CHECK(st.value() == TextLayerEnum::ShapeType::BoxText);
	CHECK(layer->is_box_text());
	CHECK_FALSE(layer->is_point_text());
}


TEST_CASE("Shape: point text layer reports ShapeType::PointText")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kFixturePath;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "VerticalPointControl");
	REQUIRE(layer != nullptr);

	const auto st = layer->shape_type();
	REQUIRE(st.has_value());
	CHECK(st.value() == TextLayerEnum::ShapeType::PointText);
	CHECK(layer->is_point_text());
	CHECK_FALSE(layer->is_box_text());
}


// ---------------------------------------------------------------------------
//  box_bounds / box_width / box_height
// ---------------------------------------------------------------------------

TEST_CASE("Shape: box_bounds returns valid bounds for box text")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kFixturePath;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "HorizontalBoxControl");
	REQUIRE(layer != nullptr);
	REQUIRE(layer->is_box_text());

	const auto bounds = layer->box_bounds();
	REQUIRE(bounds.has_value());

	// bounds = { top, left, bottom, right } — width and height must be positive
	const double width = (*bounds)[3] - (*bounds)[1];
	const double height = (*bounds)[2] - (*bounds)[0];
	CHECK(width > 0.0);
	CHECK(height > 0.0);
}


TEST_CASE("Shape: box_width and box_height agree with box_bounds")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kFixturePath;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "HorizontalBoxControl");
	REQUIRE(layer != nullptr);

	const auto bounds = layer->box_bounds();
	REQUIRE(bounds.has_value());

	const auto w = layer->box_width();
	const auto h = layer->box_height();
	REQUIRE(w.has_value());
	REQUIRE(h.has_value());

	CHECK(w.value() == doctest::Approx((*bounds)[3] - (*bounds)[1]));
	CHECK(h.value() == doctest::Approx((*bounds)[2] - (*bounds)[0]));
}


TEST_CASE("Shape: box_bounds returns nullopt for point text")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kFixturePath;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "VerticalPointControl");
	REQUIRE(layer != nullptr);
	REQUIRE(layer->is_point_text());

	CHECK_FALSE(layer->box_bounds().has_value());
	CHECK_FALSE(layer->box_width().has_value());
	CHECK_FALSE(layer->box_height().has_value());
}


// ---------------------------------------------------------------------------
//  set_box_bounds / set_box_size / set_box_width / set_box_height
// ---------------------------------------------------------------------------

TEST_CASE("Shape: set_box_bounds changes bounds and survives roundtrip")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kFixturePath;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "HorizontalBoxControl");
	REQUIRE(layer != nullptr);
	REQUIRE(layer->is_box_text());

	const double new_top = 10.0, new_left = 20.0, new_bottom = 310.0, new_right = 520.0;
	CHECK_NOTHROW(layer->set_box_bounds(new_top, new_left, new_bottom, new_right));

	// Verify immediate readback
	const auto bounds = layer->box_bounds();
	REQUIRE(bounds.has_value());
	CHECK((*bounds)[0] == doctest::Approx(new_top));
	CHECK((*bounds)[1] == doctest::Approx(new_left));
	CHECK((*bounds)[2] == doctest::Approx(new_bottom));
	CHECK((*bounds)[3] == doctest::Approx(new_right));

	// Roundtrip through file write / read
	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_by_name(reread, "HorizontalBoxControl");
	REQUIRE(reread_layer != nullptr);

	const auto rb = reread_layer->box_bounds();
	REQUIRE(rb.has_value());
	CHECK((*rb)[0] == doctest::Approx(new_top));
	CHECK((*rb)[1] == doctest::Approx(new_left));
	CHECK((*rb)[2] == doctest::Approx(new_bottom));
	CHECK((*rb)[3] == doctest::Approx(new_right));

	std::filesystem::remove(out_path);
}


TEST_CASE("Shape: set_box_size keeps top-left and sets new width/height")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kFixturePath;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "HorizontalBoxControl");
	REQUIRE(layer != nullptr);

	const auto orig = layer->box_bounds();
	REQUIRE(orig.has_value());
	const double orig_top = (*orig)[0];
	const double orig_left = (*orig)[1];

	CHECK_NOTHROW(layer->set_box_size(400.0, 200.0));

	const auto after = layer->box_bounds();
	REQUIRE(after.has_value());
	CHECK((*after)[0] == doctest::Approx(orig_top));
	CHECK((*after)[1] == doctest::Approx(orig_left));
	CHECK(layer->box_width().value() == doctest::Approx(400.0));
	CHECK(layer->box_height().value() == doctest::Approx(200.0));
}


TEST_CASE("Shape: set_box_width changes only width")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kFixturePath;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "HorizontalBoxControl");
	REQUIRE(layer != nullptr);

	const auto orig_h = layer->box_height();
	REQUIRE(orig_h.has_value());

	CHECK_NOTHROW(layer->set_box_width(999.0));
	CHECK(layer->box_width().value() == doctest::Approx(999.0));
	CHECK(layer->box_height().value() == doctest::Approx(orig_h.value()));
}


TEST_CASE("Shape: set_box_height changes only height")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kFixturePath;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "HorizontalBoxControl");
	REQUIRE(layer != nullptr);

	const auto orig_w = layer->box_width();
	REQUIRE(orig_w.has_value());

	CHECK_NOTHROW(layer->set_box_height(777.0));
	CHECK(layer->box_height().value() == doctest::Approx(777.0));
	CHECK(layer->box_width().value() == doctest::Approx(orig_w.value()));
}


TEST_CASE("Shape: set_box_bounds rejects non-finite values")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kFixturePath;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "HorizontalBoxControl");
	REQUIRE(layer != nullptr);

	const auto inf = std::numeric_limits<double>::infinity();
	const auto nan = std::numeric_limits<double>::quiet_NaN();
	CHECK_THROWS(layer->set_box_bounds(inf, 0, 100, 100));
	CHECK_THROWS(layer->set_box_bounds(0, nan, 100, 100));
	CHECK_THROWS(layer->set_box_size(inf, 100));
	CHECK_THROWS(layer->set_box_width(nan));
	CHECK_THROWS(layer->set_box_height(-1.0));
}


// ---------------------------------------------------------------------------
//  convert_to_box_text / convert_to_point_text
// ---------------------------------------------------------------------------

TEST_CASE("Shape: convert_to_box_text turns point text into box text")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kFixturePath;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "VerticalPointControl");
	REQUIRE(layer != nullptr);
	REQUIRE(layer->is_point_text());

	CHECK_NOTHROW(layer->convert_to_box_text(300.0, 150.0));
	CHECK(layer->is_box_text());
	CHECK_FALSE(layer->is_point_text());

	const auto w = layer->box_width();
	const auto h = layer->box_height();
	REQUIRE(w.has_value());
	REQUIRE(h.has_value());
	CHECK(w.value() == doctest::Approx(300.0));
	CHECK(h.value() == doctest::Approx(150.0));
}


TEST_CASE("Shape: convert_to_box_text roundtrips through file write/read")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kFixturePath;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "VerticalPointControl");
	REQUIRE(layer != nullptr);

	CHECK_NOTHROW(layer->convert_to_box_text(250.0, 120.0));

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_by_name(reread, "VerticalPointControl");
	REQUIRE(reread_layer != nullptr);
	CHECK(reread_layer->is_box_text());
	CHECK(reread_layer->box_width().value() == doctest::Approx(250.0));
	CHECK(reread_layer->box_height().value() == doctest::Approx(120.0));

	std::filesystem::remove(out_path);
}


TEST_CASE("Shape: convert_to_point_text turns box text into point text")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kFixturePath;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "HorizontalBoxControl");
	REQUIRE(layer != nullptr);
	REQUIRE(layer->is_box_text());

	CHECK_NOTHROW(layer->convert_to_point_text());
	CHECK(layer->is_point_text());
	CHECK_FALSE(layer->is_box_text());
	CHECK_FALSE(layer->box_bounds().has_value());
}


TEST_CASE("Shape: convert_to_point_text roundtrips through file write/read")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kFixturePath;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "HorizontalBoxControl");
	REQUIRE(layer != nullptr);

	CHECK_NOTHROW(layer->convert_to_point_text());

	const auto out_path = temp_psd_path();
	LayeredFile<bpp8_t>::write(std::move(file), out_path);
	REQUIRE(std::filesystem::exists(out_path));

	auto reread = LayeredFile<bpp8_t>::read(out_path);
	auto reread_layer = find_text_layer_by_name(reread, "HorizontalBoxControl");
	REQUIRE(reread_layer != nullptr);
	CHECK(reread_layer->is_point_text());
	CHECK_FALSE(reread_layer->box_bounds().has_value());

	std::filesystem::remove(out_path);
}


TEST_CASE("Shape: convert_to_box_text fails when already box text")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kFixturePath;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "HorizontalBoxControl");
	REQUIRE(layer != nullptr);
	REQUIRE(layer->is_box_text());

	CHECK_THROWS(layer->convert_to_box_text(100, 100));
}


TEST_CASE("Shape: convert_to_point_text fails when already point text")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kFixturePath;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "VerticalPointControl");
	REQUIRE(layer != nullptr);
	REQUIRE(layer->is_point_text());

	CHECK_THROWS(layer->convert_to_point_text());
}


// ---------------------------------------------------------------------------
//  Vertical box text (combined shape + orientation)
// ---------------------------------------------------------------------------

TEST_CASE("Shape: vertical box text has correct shape and orientation")
{
	using namespace NAMESPACE_PSAPI;
	const auto path = std::filesystem::current_path() / kFixturePath;
	REQUIRE(std::filesystem::exists(path));

	auto file = LayeredFile<bpp8_t>::read(path);
	auto layer = find_text_layer_by_name(file, "VerticalBoxText");
	REQUIRE(layer != nullptr);

	CHECK(layer->is_box_text());
	CHECK(layer->is_vertical());

	const auto orientation = layer->orientation();
	REQUIRE(orientation.has_value());
	CHECK(orientation.value() == TextLayerEnum::WritingDirection::Vertical);

	const auto w = layer->box_width();
	const auto h = layer->box_height();
	REQUIRE(w.has_value());
	REQUIRE(h.has_value());
	CHECK(w.value() > 0.0);
	CHECK(h.value() > 0.0);
}
