#include "doctest.h"

#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"

#include <cmath>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

using namespace NAMESPACE_PSAPI;

namespace
{
	template <typename T>
	std::shared_ptr<TextLayer<T>> find_text_layer(LayeredFile<T>& file, const std::string& name)
	{
		for (auto& layer : file.flat_layers())
		{
			auto tl = std::dynamic_pointer_cast<TextLayer<T>>(layer);
			if (tl && tl->name() == name)
				return tl;
		}
		return nullptr;
	}
}

// ── Transform fixture: TextLayers_Transform.psd ────────────────────────
// Contains:
//   "RotatedText"     – rotated 18° and resized 86% x 118%
//   "TransformControl" – plain text, identity transform

TEST_CASE("Transform: read returns 6-element vector for RotatedText")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Transform.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "RotatedText");
	REQUIRE(layer != nullptr);

	auto xform = layer->transform();
	REQUIRE(xform.size() == 6u);
	// Non-identity: xx and yy should not both be exactly 1.0
	// (rotated + scaled)
	CHECK(!(std::abs(xform[0] - 1.0) < 1e-6 && std::abs(xform[3] - 1.0) < 1e-6 &&
	        std::abs(xform[1]) < 1e-6 && std::abs(xform[2]) < 1e-6));
}

TEST_CASE("Transform: control layer has identity-like rotation submatrix")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Transform.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "TransformControl");
	REQUIRE(layer != nullptr);

	auto xform = layer->transform();
	REQUIRE(xform.size() == 6u);
	// For un-rotated, un-scaled text: xx≈1 yy≈1 xy≈0 yx≈0
	CHECK(xform[0] == doctest::Approx(1.0).epsilon(0.01));
	CHECK(xform[1] == doctest::Approx(0.0).epsilon(0.01));
	CHECK(xform[2] == doctest::Approx(0.0).epsilon(0.01));
	CHECK(xform[3] == doctest::Approx(1.0).epsilon(0.01));
}

TEST_CASE("Transform: individual component accessors agree with vector")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Transform.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "RotatedText");
	REQUIRE(layer != nullptr);

	auto xform = layer->transform();
	REQUIRE(xform.size() == 6u);

	CHECK(layer->transform_xx().value() == doctest::Approx(xform[0]));
	CHECK(layer->transform_xy().value() == doctest::Approx(xform[1]));
	CHECK(layer->transform_yx().value() == doctest::Approx(xform[2]));
	CHECK(layer->transform_yy().value() == doctest::Approx(xform[3]));
	CHECK(layer->transform_tx().value() == doctest::Approx(xform[4]));
	CHECK(layer->transform_ty().value() == doctest::Approx(xform[5]));
}

TEST_CASE("Transform: RotatedText has expected rotation direction")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Transform.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "RotatedText");
	REQUIRE(layer != nullptr);

	auto xx = layer->transform_xx().value();
	auto xy = layer->transform_xy().value();
	// For a positive rotation angle: xy > 0 means clockwise in Photoshop coordinate system
	// The fixture rotates +18 degrees
	CHECK(xy != doctest::Approx(0.0).epsilon(0.001));
	// xx should be less than 1.0 (cos(18°)*scale < 1.0 since scale = 0.86)
	CHECK(xx < 1.0);
	CHECK(xx > 0.0);
}

TEST_CASE("Transform: translation components are non-zero for positioned text")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Transform.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "RotatedText");
	REQUIRE(layer != nullptr);

	// tx, ty are the position offsets and should be non-zero for text placed at (180, 190)
	auto tx = layer->transform_tx();
	auto ty = layer->transform_ty();
	REQUIRE(tx.has_value());
	REQUIRE(ty.has_value());
	// Just verify they're plausible position values (order of magnitude of the document)
	CHECK(std::abs(tx.value()) > 1.0);
	CHECK(std::abs(ty.value()) > 1.0);
}

TEST_CASE("Transform: set_transform writes and reads back correctly")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "SimpleASCII");
	REQUIRE(layer != nullptr);

	// Read original
	auto orig = layer->transform();
	REQUIRE(orig.size() == 6u);

	// Set a custom transform (30° rotation, no scaling)
	const double angle = 30.0 * 3.14159265358979323846 / 180.0;
	const double c = std::cos(angle);
	const double s = std::sin(angle);
	std::vector<double> custom = {c, s, -s, c, 42.5, 99.0};
	CHECK_NOTHROW(layer->set_transform(custom));

	// Read back
	auto readback = layer->transform();
	REQUIRE(readback.size() == 6u);
	CHECK(readback[0] == doctest::Approx(c).epsilon(1e-10));
	CHECK(readback[1] == doctest::Approx(s).epsilon(1e-10));
	CHECK(readback[2] == doctest::Approx(-s).epsilon(1e-10));
	CHECK(readback[3] == doctest::Approx(c).epsilon(1e-10));
	CHECK(readback[4] == doctest::Approx(42.5).epsilon(1e-10));
	CHECK(readback[5] == doctest::Approx(99.0).epsilon(1e-10));
}

TEST_CASE("Transform: set_transform_xx/yy individual writers work")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "SimpleASCII");
	REQUIRE(layer != nullptr);

	CHECK_NOTHROW(layer->set_transform_xx(2.5));
	CHECK_NOTHROW(layer->set_transform_yy(3.0));
	CHECK_NOTHROW(layer->set_transform_xy(0.1));
	CHECK_NOTHROW(layer->set_transform_yx(-0.2));
	CHECK_NOTHROW(layer->set_transform_tx(100.0));
	CHECK_NOTHROW(layer->set_transform_ty(200.0));

	CHECK(layer->transform_xx().value() == doctest::Approx(2.5));
	CHECK(layer->transform_yy().value() == doctest::Approx(3.0));
	CHECK(layer->transform_xy().value() == doctest::Approx(0.1));
	CHECK(layer->transform_yx().value() == doctest::Approx(-0.2));
	CHECK(layer->transform_tx().value() == doctest::Approx(100.0));
	CHECK(layer->transform_ty().value() == doctest::Approx(200.0));
}

TEST_CASE("Transform: set_transform rejects wrong-size vector")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "SimpleASCII");
	REQUIRE(layer != nullptr);

	CHECK_THROWS(layer->set_transform({}));
	CHECK_THROWS(layer->set_transform({1.0, 0.0, 0.0}));
	CHECK_THROWS(layer->set_transform({1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0}));
}

TEST_CASE("Transform: set_transform_component rejects out-of-range index")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "SimpleASCII");
	REQUIRE(layer != nullptr);

	CHECK_THROWS(layer->set_transform_component(6, 1.0));
	CHECK_THROWS(layer->set_transform_component(100, 1.0));
	CHECK_FALSE(layer->transform_component(6).has_value());
}

TEST_CASE("Transform: roundtrip through file write preserves transform")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "SimpleASCII");
	REQUIRE(layer != nullptr);

	// Set a custom transform
	const double angle = 45.0 * 3.14159265358979323846 / 180.0;
	const double c = std::cos(angle);
	const double s = std::sin(angle);
	std::vector<double> custom = {c, s, -s, c, 55.5, 77.7};
	CHECK_NOTHROW(layer->set_transform(custom));

	// Write to temp and re-read
	auto tmp = std::filesystem::temp_directory_path() / "psapi_transform_roundtrip.psb";
	LayeredFile<bpp8_t>::write(std::move(file), tmp);

	auto file2 = LayeredFile<bpp8_t>::read(tmp);
	auto layer2 = find_text_layer(file2, "SimpleASCII");
	REQUIRE(layer2 != nullptr);

	auto readback = layer2->transform();
	REQUIRE(readback.size() == 6u);
	CHECK(readback[0] == doctest::Approx(c).epsilon(1e-10));
	CHECK(readback[1] == doctest::Approx(s).epsilon(1e-10));
	CHECK(readback[2] == doctest::Approx(-s).epsilon(1e-10));
	CHECK(readback[3] == doctest::Approx(c).epsilon(1e-10));
	CHECK(readback[4] == doctest::Approx(55.5).epsilon(1e-10));
	CHECK(readback[5] == doctest::Approx(77.7).epsilon(1e-10));

	std::filesystem::remove(tmp);
}

TEST_CASE("Transform: Basic fixture SimpleASCII has identity rotation submatrix")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "SimpleASCII");
	REQUIRE(layer != nullptr);

	auto xform = layer->transform();
	REQUIRE(xform.size() == 6u);
	// Un-transformed text: identity rotation/scale submatrix
	CHECK(xform[0] == doctest::Approx(1.0).epsilon(0.01));
	CHECK(xform[1] == doctest::Approx(0.0).epsilon(0.01));
	CHECK(xform[2] == doctest::Approx(0.0).epsilon(0.01));
	CHECK(xform[3] == doctest::Approx(1.0).epsilon(0.01));
}

// ── High-level convenience: rotation_angle / scale ─────────────────────

TEST_CASE("Transform: rotation_angle returns 0 for un-rotated text")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "SimpleASCII");
	REQUIRE(layer != nullptr);

	auto angle = layer->rotation_angle();
	REQUIRE(angle.has_value());
	CHECK(angle.value() == doctest::Approx(0.0).epsilon(0.1));
}

TEST_CASE("Transform: rotation_angle returns non-zero for rotated fixture")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Transform.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "RotatedText");
	REQUIRE(layer != nullptr);

	auto angle = layer->rotation_angle();
	REQUIRE(angle.has_value());
	// Fixture was rotated +18 degrees; combined with 86%/118% scale the angle should still be ~18
	CHECK(std::abs(angle.value()) > 1.0);  // definitely not zero
}

TEST_CASE("Transform: scale_x and scale_y return 1.0 for un-scaled text")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "SimpleASCII");
	REQUIRE(layer != nullptr);

	CHECK(layer->scale_x().value() == doctest::Approx(1.0).epsilon(0.01));
	CHECK(layer->scale_y().value() == doctest::Approx(1.0).epsilon(0.01));
}

TEST_CASE("Transform: scale_x and scale_y reflect fixture scaling")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Transform.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "RotatedText");
	REQUIRE(layer != nullptr);

	auto sx = layer->scale_x();
	auto sy = layer->scale_y();
	REQUIRE(sx.has_value());
	REQUIRE(sy.has_value());
	// Fixture scaled 86% x 118%
	CHECK(sx.value() == doctest::Approx(0.86).epsilon(0.02));
	CHECK(sy.value() == doctest::Approx(1.18).epsilon(0.02));
}

TEST_CASE("Transform: set_rotation_angle sets angle and preserves scale/translation")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "SimpleASCII");
	REQUIRE(layer != nullptr);

	auto orig = layer->transform();
	REQUIRE(orig.size() == 6u);
	const double orig_tx = orig[4];
	const double orig_ty = orig[5];

	CHECK_NOTHROW(layer->set_rotation_angle(45.0));

	auto angle = layer->rotation_angle();
	REQUIRE(angle.has_value());
	CHECK(angle.value() == doctest::Approx(45.0).epsilon(0.01));

	// Scale should still be ~1.0
	CHECK(layer->scale_x().value() == doctest::Approx(1.0).epsilon(0.01));
	CHECK(layer->scale_y().value() == doctest::Approx(1.0).epsilon(0.01));

	// Translation preserved
	CHECK(layer->transform_tx().value() == doctest::Approx(orig_tx).epsilon(1e-10));
	CHECK(layer->transform_ty().value() == doctest::Approx(orig_ty).epsilon(1e-10));
}

TEST_CASE("Transform: set_rotation_angle negative angle")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "SimpleASCII");
	REQUIRE(layer != nullptr);

	CHECK_NOTHROW(layer->set_rotation_angle(-30.0));
	CHECK(layer->rotation_angle().value() == doctest::Approx(-30.0).epsilon(0.01));
}

TEST_CASE("Transform: set_scale_x changes horizontal scale preserving angle")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "SimpleASCII");
	REQUIRE(layer != nullptr);

	CHECK_NOTHROW(layer->set_rotation_angle(20.0));
	CHECK_NOTHROW(layer->set_scale_x(1.5));

	CHECK(layer->scale_x().value() == doctest::Approx(1.5).epsilon(0.01));
	CHECK(layer->scale_y().value() == doctest::Approx(1.0).epsilon(0.01));
	CHECK(layer->rotation_angle().value() == doctest::Approx(20.0).epsilon(0.01));
}

TEST_CASE("Transform: set_scale_y changes vertical scale preserving angle")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "SimpleASCII");
	REQUIRE(layer != nullptr);

	CHECK_NOTHROW(layer->set_rotation_angle(20.0));
	CHECK_NOTHROW(layer->set_scale_y(0.75));

	CHECK(layer->scale_y().value() == doctest::Approx(0.75).epsilon(0.01));
	CHECK(layer->scale_x().value() == doctest::Approx(1.0).epsilon(0.01));
	CHECK(layer->rotation_angle().value() == doctest::Approx(20.0).epsilon(0.01));
}

TEST_CASE("Transform: set_scale sets both factors at once")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "SimpleASCII");
	REQUIRE(layer != nullptr);

	CHECK_NOTHROW(layer->set_rotation_angle(15.0));
	CHECK_NOTHROW(layer->set_scale(2.0, 0.5));

	CHECK(layer->scale_x().value() == doctest::Approx(2.0).epsilon(0.01));
	CHECK(layer->scale_y().value() == doctest::Approx(0.5).epsilon(0.01));
	CHECK(layer->rotation_angle().value() == doctest::Approx(15.0).epsilon(0.01));
}

TEST_CASE("Transform: rotation + scale roundtrip through file write")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "SimpleASCII");
	REQUIRE(layer != nullptr);

	CHECK_NOTHROW(layer->set_rotation_angle(60.0));
	CHECK_NOTHROW(layer->set_scale(1.25, 0.8));

	auto tmp = std::filesystem::temp_directory_path() / "psapi_rot_scale_rt.psb";
	LayeredFile<bpp8_t>::write(std::move(file), tmp);

	auto file2 = LayeredFile<bpp8_t>::read(tmp);
	auto layer2 = find_text_layer(file2, "SimpleASCII");
	REQUIRE(layer2 != nullptr);

	CHECK(layer2->rotation_angle().value() == doctest::Approx(60.0).epsilon(0.01));
	CHECK(layer2->scale_x().value() == doctest::Approx(1.25).epsilon(0.01));
	CHECK(layer2->scale_y().value() == doctest::Approx(0.8).epsilon(0.01));

	std::filesystem::remove(tmp);
}

// =======================================================================
//  Position convenience tests
// =======================================================================

TEST_CASE("Transform: position returns (tx, ty) pair")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Transform.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "RotatedText");
	REQUIRE(layer != nullptr);

	auto [tx, ty] = layer->position();
	// Should match individual accessors
	CHECK(tx == doctest::Approx(layer->transform_tx().value()));
	CHECK(ty == doctest::Approx(layer->transform_ty().value()));
}

TEST_CASE("Transform: set_position changes tx/ty and preserves rotation")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Transform.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "RotatedText");
	REQUIRE(layer != nullptr);

	auto angle_before = layer->rotation_angle();
	auto sx_before = layer->scale_x();

	CHECK_NOTHROW(layer->set_position(123.0, 456.0));

	auto [tx, ty] = layer->position();
	CHECK(tx == doctest::Approx(123.0));
	CHECK(ty == doctest::Approx(456.0));
	// Rotation and scale must be untouched
	CHECK(layer->rotation_angle().value() == doctest::Approx(angle_before.value()));
	CHECK(layer->scale_x().value() == doctest::Approx(sx_before.value()));
}

TEST_CASE("Transform: reset_transform produces identity with preserved position")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Transform.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "RotatedText");
	REQUIRE(layer != nullptr);

	auto [old_tx, old_ty] = layer->position();

	CHECK_NOTHROW(layer->reset_transform());

	auto xform = layer->transform();
	CHECK(xform[0] == doctest::Approx(1.0));
	CHECK(xform[1] == doctest::Approx(0.0));
	CHECK(xform[2] == doctest::Approx(0.0));
	CHECK(xform[3] == doctest::Approx(1.0));
	CHECK(xform[4] == doctest::Approx(old_tx));
	CHECK(xform[5] == doctest::Approx(old_ty));
}

// =======================================================================
//  High-level font convenience tests
// =======================================================================

TEST_CASE("Transform: primary_font_name returns a non-empty string")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "SimpleASCII");
	REQUIRE(layer != nullptr);

	auto name = layer->primary_font_name();
	REQUIRE(name.has_value());
	CHECK(!name.value().empty());
}

TEST_CASE("Transform: set_font applies to all style runs and normal sheet")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "SimpleASCII");
	REQUIRE(layer != nullptr);

	CHECK_NOTHROW(layer->set_font("Courier-Bold"));

	// primary_font_name should reflect the new font
	auto name = layer->primary_font_name();
	REQUIRE(name.has_value());
	CHECK(name.value() == "Courier-Bold");

	// Verify all style runs use the same index
	const auto run_count = layer->style_run_count();
	const auto idx0 = layer->style_run_font(0);
	REQUIRE(idx0.has_value());
	for (size_t i = 1; i < run_count; ++i)
	{
		auto idx_i = layer->style_run_font(i);
		if (idx_i.has_value())
			CHECK(idx_i.value() == idx0.value());
	}
	// Normal sheet should also match
	auto normal_idx = layer->style_normal_font();
	REQUIRE(normal_idx.has_value());
	CHECK(normal_idx.value() == idx0.value());
}

TEST_CASE("Transform: set_font roundtrips through file write")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "SimpleASCII");
	REQUIRE(layer != nullptr);

	CHECK_NOTHROW(layer->set_font("HelveticaNeue-Light"));

	auto tmp = std::filesystem::temp_directory_path() / "psapi_font_rt.psb";
	LayeredFile<bpp8_t>::write(std::move(file), tmp);

	auto file2 = LayeredFile<bpp8_t>::read(tmp);
	auto layer2 = find_text_layer(file2, "SimpleASCII");
	REQUIRE(layer2 != nullptr);

	auto name = layer2->primary_font_name();
	REQUIRE(name.has_value());
	CHECK(name.value() == "HelveticaNeue-Light");

	std::filesystem::remove(tmp);
}

TEST_CASE("Transform: position roundtrip through file write")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer(file, "SimpleASCII");
	REQUIRE(layer != nullptr);

	CHECK_NOTHROW(layer->set_position(77.5, 199.25));

	auto tmp = std::filesystem::temp_directory_path() / "psapi_pos_rt.psb";
	LayeredFile<bpp8_t>::write(std::move(file), tmp);

	auto file2 = LayeredFile<bpp8_t>::read(tmp);
	auto layer2 = find_text_layer(file2, "SimpleASCII");
	REQUIRE(layer2 != nullptr);

	auto [tx, ty] = layer2->position();
	CHECK(tx == doctest::Approx(77.5));
	CHECK(ty == doctest::Approx(199.25));

	std::filesystem::remove(tmp);
}
