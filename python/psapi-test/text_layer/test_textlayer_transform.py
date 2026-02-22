import math
import os
import tempfile
import unittest

import photoshopapi as psapi

from ._textlayer_helpers import (
    FIXTURE_PATH,
    WARP_FIXTURE_PATH,
    TRANSFORM_FIXTURE_PATH,
    is_text_layer,
    find_text_layer_by_name,
)

_FIXTURE_PATH = FIXTURE_PATH
_WARP_FIXTURE_PATH = WARP_FIXTURE_PATH
_TRANSFORM_FIXTURE_PATH = TRANSFORM_FIXTURE_PATH
_is_text_layer = is_text_layer
_find_text_layer_by_name = find_text_layer_by_name


@unittest.skipUnless(os.path.exists(_WARP_FIXTURE_PATH), "Warp fixture missing")
class TestWarpReadAPIs(unittest.TestCase):
    """Step 10: Warp text read APIs."""

    def test_has_warp_true_for_warped_layer(self):
        file = psapi.LayeredFile.read(_WARP_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "WarpArc")
        self.assertIsNotNone(layer)
        self.assertTrue(layer.has_warp)

    def test_has_warp_false_for_non_warped_layer(self):
        file = psapi.LayeredFile.read(_WARP_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "Secondary")
        self.assertIsNotNone(layer)
        self.assertFalse(layer.has_warp)

    def test_warp_style_returns_enum(self):
        file = psapi.LayeredFile.read(_WARP_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "WarpArc")
        self.assertIsNotNone(layer)
        self.assertEqual(layer.warp_style, psapi.enum.WarpStyle.Arc)

    def test_warp_value_is_float(self):
        file = psapi.LayeredFile.read(_WARP_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "WarpArc")
        self.assertIsNotNone(layer)
        self.assertIsInstance(layer.warp_value, float)

    def test_warp_distortion_not_none(self):
        file = psapi.LayeredFile.read(_WARP_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "WarpArc")
        self.assertIsNotNone(layer)
        self.assertIsNotNone(layer.warp_horizontal_distortion)
        self.assertIsNotNone(layer.warp_vertical_distortion)

    def test_warp_rotation_type(self):
        file = psapi.LayeredFile.read(_WARP_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "WarpArc")
        self.assertIsNotNone(layer)
        rot = layer.warp_rotation
        self.assertIn(rot, (psapi.enum.WarpRotation.Horizontal, psapi.enum.WarpRotation.Vertical))

    def test_non_warped_layer_warp_style_is_none_like(self):
        file = psapi.LayeredFile.read(_WARP_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "Secondary")
        self.assertIsNotNone(layer)
        if layer.has_warp:
            self.fail("Expected no warp on Secondary layer")
        else:
            # Non-warped layers may expose no style or an explicit NoWarp enum.
            style = layer.warp_style
            self.assertIn(style, (None, psapi.enum.WarpStyle.NoWarp))


@unittest.skipUnless(os.path.exists(_TRANSFORM_FIXTURE_PATH), "Transform fixture missing")
class TestTransformAPIs(unittest.TestCase):
    """Transform read/write APIs for the TySh 2D affine matrix."""

    def test_rotated_layer_has_6_element_transform(self):
        file = psapi.LayeredFile.read(_TRANSFORM_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "RotatedText")
        self.assertIsNotNone(layer)
        xform = layer.transform()
        self.assertEqual(len(xform), 6)

    def test_transform_individual_components(self):
        file = psapi.LayeredFile.read(_TRANSFORM_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "RotatedText")
        self.assertIsNotNone(layer)

        xform = layer.transform()
        self.assertAlmostEqual(layer.transform_xx, xform[0], places=10)
        self.assertAlmostEqual(layer.transform_xy, xform[1], places=10)
        self.assertAlmostEqual(layer.transform_yx, xform[2], places=10)
        self.assertAlmostEqual(layer.transform_yy, xform[3], places=10)
        self.assertAlmostEqual(layer.transform_tx, xform[4], places=10)
        self.assertAlmostEqual(layer.transform_ty, xform[5], places=10)

    def test_set_transform_updates_all_fields(self):
        file = psapi.LayeredFile.read(_TRANSFORM_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "TransformControl")
        self.assertIsNotNone(layer)

        new_xform = [2.0, 0.1, -0.1, 2.0, 50.0, 75.0]
        layer.set_transform(new_xform)
        after = layer.transform()
        for i in range(6):
            self.assertAlmostEqual(after[i], new_xform[i], places=10)

    def test_set_transform_rejects_wrong_length(self):
        file = psapi.LayeredFile.read(_TRANSFORM_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "TransformControl")
        self.assertIsNotNone(layer)
        with self.assertRaises(ValueError):
            layer.set_transform([1.0, 0.0, 0.0])

    def test_set_transform_individual_setters(self):
        file = psapi.LayeredFile.read(_TRANSFORM_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "TransformControl")
        self.assertIsNotNone(layer)

        layer.set_transform([3.0, 0.5, -0.5, 3.0, 100.0, 200.0])

        self.assertAlmostEqual(layer.transform_xx, 3.0, places=10)
        self.assertAlmostEqual(layer.transform_xy, 0.5, places=10)
        self.assertAlmostEqual(layer.transform_yx, -0.5, places=10)
        self.assertAlmostEqual(layer.transform_yy, 3.0, places=10)
        self.assertAlmostEqual(layer.transform_tx, 100.0, places=10)
        self.assertAlmostEqual(layer.transform_ty, 200.0, places=10)

    def test_set_transform_roundtrip(self):
        file = psapi.LayeredFile.read(_TRANSFORM_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "TransformControl")
        self.assertIsNotNone(layer)

        new_xform = [1.5, 0.25, -0.25, 1.5, 33.3, 44.4]
        layer.set_transform(new_xform)

        with tempfile.NamedTemporaryFile(suffix=".psd", delete=False) as f:
            tmp_path = f.name
        try:
            file.write(tmp_path)
            reread = psapi.LayeredFile.read(tmp_path)
            rl = _find_text_layer_by_name(reread, "TransformControl")
            self.assertIsNotNone(rl)
            after = rl.transform()
            for i in range(6):
                self.assertAlmostEqual(after[i], new_xform[i], places=6)
        finally:
            os.remove(tmp_path)


class TestTransformConvenienceAPIs(unittest.TestCase):
    """High-level rotation_angle / scale convenience APIs."""

    # -- readers -----------------------------------------------------------

    def test_rotation_angle_zero_for_unrotated(self):
        file = psapi.LayeredFile.read(_TRANSFORM_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "TransformControl")
        self.assertIsNotNone(layer)
        self.assertAlmostEqual(layer.rotation_angle, 0.0, places=6)

    def test_rotation_angle_nonzero_for_rotated(self):
        file = psapi.LayeredFile.read(_TRANSFORM_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "RotatedText")
        self.assertIsNotNone(layer)
        angle = layer.rotation_angle
        self.assertNotAlmostEqual(angle, 0.0, places=2)

    def test_scale_x_y_identity_for_unscaled(self):
        file = psapi.LayeredFile.read(_TRANSFORM_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "TransformControl")
        self.assertIsNotNone(layer)
        self.assertAlmostEqual(layer.scale_x, 1.0, places=6)
        self.assertAlmostEqual(layer.scale_y, 1.0, places=6)

    # -- writers -----------------------------------------------------------

    def test_set_rotation_angle_roundtrip(self):
        file = psapi.LayeredFile.read(_TRANSFORM_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "TransformControl")
        self.assertIsNotNone(layer)
        layer.set_rotation_angle(45.0)
        self.assertAlmostEqual(layer.rotation_angle, 45.0, places=6)

    def test_set_rotation_preserves_scale(self):
        file = psapi.LayeredFile.read(_TRANSFORM_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "TransformControl")
        self.assertIsNotNone(layer)
        layer.set_scale(2.0, 3.0)
        layer.set_rotation_angle(30.0)
        self.assertAlmostEqual(layer.scale_x, 2.0, places=10)
        self.assertAlmostEqual(layer.scale_y, 3.0, places=10)
        self.assertAlmostEqual(layer.rotation_angle, 30.0, places=6)

    def test_set_scale_roundtrip(self):
        file = psapi.LayeredFile.read(_TRANSFORM_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "TransformControl")
        self.assertIsNotNone(layer)
        layer.set_scale(1.5, 2.0)
        self.assertAlmostEqual(layer.scale_x, 1.5, places=10)
        self.assertAlmostEqual(layer.scale_y, 2.0, places=10)

    def test_set_scale_preserves_rotation(self):
        file = psapi.LayeredFile.read(_TRANSFORM_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "TransformControl")
        self.assertIsNotNone(layer)
        layer.set_rotation_angle(60.0)
        layer.set_scale(1.5, 2.0)
        self.assertAlmostEqual(layer.rotation_angle, 60.0, places=6)
        self.assertAlmostEqual(layer.scale_x, 1.5, places=10)
        self.assertAlmostEqual(layer.scale_y, 2.0, places=10)

    def test_combined_rotation_and_scale_roundtrip(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "SimpleASCII")
        self.assertIsNotNone(layer)
        layer.set_rotation_angle(60.0)
        layer.set_scale(1.5, 2.0)

        with tempfile.NamedTemporaryFile(suffix=".psd", delete=False) as f:
            tmp_path = f.name
        try:
            file.write(tmp_path)
            reread = psapi.LayeredFile.read(tmp_path)
            rl = _find_text_layer_by_name(reread, "SimpleASCII")
            self.assertIsNotNone(rl)
            self.assertAlmostEqual(rl.rotation_angle, 60.0, places=6)
            self.assertAlmostEqual(rl.scale_x, 1.5, places=10)
            self.assertAlmostEqual(rl.scale_y, 2.0, places=10)
        finally:
            os.remove(tmp_path)


class TestPositionConvenience(unittest.TestCase):
    """Position, reset_transform convenience APIs."""

    def test_position_returns_tuple(self):
        file = psapi.LayeredFile.read(_TRANSFORM_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "RotatedText")
        self.assertIsNotNone(layer)
        pos = layer.position()
        self.assertEqual(len(pos), 2)
        self.assertAlmostEqual(pos[0], layer.transform_tx, places=10)
        self.assertAlmostEqual(pos[1], layer.transform_ty, places=10)

    def test_set_position_updates_tx_ty(self):
        file = psapi.LayeredFile.read(_TRANSFORM_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "TransformControl")
        self.assertIsNotNone(layer)
        layer.set_position(77.5, 199.25)
        self.assertAlmostEqual(layer.transform_tx, 77.5, places=10)
        self.assertAlmostEqual(layer.transform_ty, 199.25, places=10)

    def test_set_position_preserves_rotation_and_scale(self):
        file = psapi.LayeredFile.read(_TRANSFORM_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "TransformControl")
        self.assertIsNotNone(layer)
        layer.set_rotation_angle(25.0)
        layer.set_scale(1.8, 1.8)
        layer.set_position(77.5, 199.25)
        self.assertAlmostEqual(layer.rotation_angle, 25.0, places=6)
        self.assertAlmostEqual(layer.scale_x, 1.8, places=10)
        self.assertAlmostEqual(layer.scale_y, 1.8, places=10)
        pos = layer.position()
        self.assertAlmostEqual(pos[0], 77.5)
        self.assertAlmostEqual(pos[1], 199.25)

    def test_reset_transform_restores_identity(self):
        file = psapi.LayeredFile.read(_TRANSFORM_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "RotatedText")
        self.assertIsNotNone(layer)
        layer.reset_transform()
        self.assertAlmostEqual(layer.rotation_angle, 0.0, places=10)
        self.assertAlmostEqual(layer.scale_x, 1.0, places=10)
        self.assertAlmostEqual(layer.scale_y, 1.0, places=10)


@unittest.skipUnless(os.path.exists(_FIXTURE_PATH), "Basic fixture missing")
class TestUniformScale(unittest.TestCase):
    """set_scale(factor) single-arg uniform scale."""

    def test_uniform_scale_sets_both_axes(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "SimpleASCII")
        self.assertIsNotNone(layer)
        layer.set_scale(1.5)
        self.assertAlmostEqual(layer.scale_x, 1.5, places=10)
        self.assertAlmostEqual(layer.scale_y, 1.5, places=10)

    def test_uniform_scale_preserves_rotation(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "SimpleASCII")
        self.assertIsNotNone(layer)
        layer.set_rotation_angle(35.0)
        layer.set_scale(2.0)
        self.assertAlmostEqual(layer.rotation_angle, 35.0, places=6)
        self.assertAlmostEqual(layer.scale_x, 2.0, places=10)
        self.assertAlmostEqual(layer.scale_y, 2.0, places=10)
