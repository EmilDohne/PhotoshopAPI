import os
import tempfile
import unittest

import photoshopapi as psapi

from ._textlayer_helpers import (
    FIXTURE_PATH,
    STYLE_RUNS_FIXTURE_PATH,
    FONTFALLBACK_FIXTURE_PATH,
    VERTICAL_FIXTURE_PATH,
    is_text_layer,
    find_text_layer,
    find_text_layer_by_name,
)

_FIXTURE_PATH = FIXTURE_PATH
_STYLE_RUNS_FIXTURE_PATH = STYLE_RUNS_FIXTURE_PATH
_FONTFALLBACK_FIXTURE_PATH = FONTFALLBACK_FIXTURE_PATH
_VERTICAL_FIXTURE_PATH = VERTICAL_FIXTURE_PATH
_is_text_layer = is_text_layer
_find_text_layer = find_text_layer
_find_text_layer_by_name = find_text_layer_by_name


class TestFontSetReadAPIs(unittest.TestCase):
    """Tests for FontSet read APIs."""

    def _find_text_layer(self, file, text):
        for layer in file.flat_layers:
            if isinstance(layer, psapi.TextLayer_8bit):
                val = layer.text
                if val is not None and val == text:
                    return layer
        return None

    def test_font_count(self):
        file = psapi.LayeredFile.read(_STYLE_RUNS_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Alpha Beta Gamma")
        self.assertIsNotNone(layer)
        self.assertGreaterEqual(layer.font_count, 3)

    def test_font_postscript_name(self):
        file = psapi.LayeredFile.read(_STYLE_RUNS_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Alpha Beta Gamma")
        self.assertIsNotNone(layer)

        count = layer.font_count
        self.assertGreaterEqual(count, 1)

        # Every font should have a non-empty PostScript name
        for i in range(count):
            name = layer.font_postscript_name(i)
            self.assertIsNotNone(name)
            self.assertTrue(len(name) > 0)

        # font_name is an alias
        self.assertEqual(layer.font_name(0), layer.font_postscript_name(0))

        # AdobeInvisFont should be present
        names = [layer.font_postscript_name(i) for i in range(count)]
        self.assertIn("AdobeInvisFont", names)

    def test_font_postscript_name_out_of_range(self):
        file = psapi.LayeredFile.read(_STYLE_RUNS_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Alpha Beta Gamma")
        self.assertIsNotNone(layer)
        self.assertIsNone(layer.font_postscript_name(9999))

    def test_font_script(self):
        file = psapi.LayeredFile.read(_STYLE_RUNS_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Alpha Beta Gamma")
        self.assertIsNotNone(layer)

        count = layer.font_count
        for i in range(count):
            script = layer.font_script(i)
            self.assertIsNotNone(script)
            self.assertIn(script, (psapi.enum.FontScript.Roman, psapi.enum.FontScript.CJK))

    def test_font_type(self):
        file = psapi.LayeredFile.read(_STYLE_RUNS_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Alpha Beta Gamma")
        self.assertIsNotNone(layer)

        count = layer.font_count
        for i in range(count):
            ft = layer.font_type(i)
            self.assertIsNotNone(ft)
            self.assertIn(ft, (psapi.enum.FontType.OpenType, psapi.enum.FontType.TrueType))

    def test_font_synthetic(self):
        file = psapi.LayeredFile.read(_STYLE_RUNS_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Alpha Beta Gamma")
        self.assertIsNotNone(layer)

        count = layer.font_count
        for i in range(count):
            synthetic = layer.font_synthetic(i)
            self.assertIsNotNone(synthetic)
            self.assertGreaterEqual(synthetic, 0)

    def test_style_run_font_maps_to_font_name(self):
        file = psapi.LayeredFile.read(_STYLE_RUNS_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Alpha Beta Gamma")
        self.assertIsNotNone(layer)

        run_font_idx = layer.style_run_font(0)
        self.assertIsNotNone(run_font_idx)

        psname = layer.font_postscript_name(run_font_idx)
        self.assertIsNotNone(psname)
        self.assertTrue(len(psname) > 0)


class TestOrientationAPIs(unittest.TestCase):
    """Tests for orientation / WritingDirection APIs."""

    @staticmethod
    def _find_text_layer(file, expected_text):
        for layer in file.flat_layers:
            if _is_text_layer(layer) and layer.text == expected_text:
                return layer
        return None

    @unittest.skipUnless(os.path.exists(_FIXTURE_PATH), "Basic fixture missing")
    def test_orientation_horizontal(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)
        self.assertEqual(layer.orientation(), psapi.enum.WritingDirection.Horizontal)
        self.assertFalse(layer.is_vertical)

    @unittest.skipUnless(os.path.exists(_VERTICAL_FIXTURE_PATH), "Vertical fixture missing")
    def test_orientation_vertical(self):
        file = psapi.LayeredFile.read(_VERTICAL_FIXTURE_PATH)
        layer = self._find_text_layer(file, "VERTICAL")
        self.assertIsNotNone(layer)
        self.assertEqual(layer.orientation(), psapi.enum.WritingDirection.Vertical)
        self.assertTrue(layer.is_vertical)

    @unittest.skipUnless(os.path.exists(_VERTICAL_FIXTURE_PATH), "Vertical fixture missing")
    def test_control_layer_is_horizontal(self):
        file = psapi.LayeredFile.read(_VERTICAL_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Control")
        self.assertIsNotNone(layer)
        self.assertEqual(layer.orientation(), psapi.enum.WritingDirection.Horizontal)
        self.assertFalse(layer.is_vertical)

    @unittest.skipUnless(os.path.exists(_FIXTURE_PATH), "Basic fixture missing")
    def test_set_orientation_to_vertical(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)
        self.assertEqual(layer.orientation(), psapi.enum.WritingDirection.Horizontal)
        layer.set_orientation(psapi.enum.WritingDirection.Vertical)
        self.assertEqual(layer.orientation(), psapi.enum.WritingDirection.Vertical)
        self.assertTrue(layer.is_vertical)

    @unittest.skipUnless(os.path.exists(_VERTICAL_FIXTURE_PATH), "Vertical fixture missing")
    def test_set_orientation_to_horizontal(self):
        file = psapi.LayeredFile.read(_VERTICAL_FIXTURE_PATH)
        layer = self._find_text_layer(file, "VERTICAL")
        self.assertIsNotNone(layer)
        self.assertEqual(layer.orientation(), psapi.enum.WritingDirection.Vertical)
        layer.set_orientation(psapi.enum.WritingDirection.Horizontal)
        self.assertEqual(layer.orientation(), psapi.enum.WritingDirection.Horizontal)
        self.assertFalse(layer.is_vertical)


class TestMissingFontQuery(unittest.TestCase):
    """Tests for missing-font query APIs."""

    @staticmethod
    def _find_text_layer(file, expected_text):
        for layer in file.flat_layers:
            if _is_text_layer(layer) and layer.text == expected_text:
                return layer
        return None

    @unittest.skipUnless(os.path.exists(_STYLE_RUNS_FIXTURE_PATH), "StyleRuns fixture missing")
    def test_used_font_indices_sorted_unique(self):
        file = psapi.LayeredFile.read(_STYLE_RUNS_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Alpha Beta Gamma")
        self.assertIsNotNone(layer)

        indices = layer.used_font_indices()
        self.assertGreaterEqual(len(indices), 1)
        # Sorted ascending
        self.assertEqual(indices, sorted(set(indices)))
        # All valid
        for idx in indices:
            self.assertLess(idx, layer.font_count)

    @unittest.skipUnless(os.path.exists(_STYLE_RUNS_FIXTURE_PATH), "StyleRuns fixture missing")
    def test_used_font_names_excludes_sentinel(self):
        file = psapi.LayeredFile.read(_STYLE_RUNS_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Alpha Beta Gamma")
        self.assertIsNotNone(layer)

        names = layer.used_font_names()
        self.assertGreaterEqual(len(names), 1)
        self.assertNotIn("AdobeInvisFont", names)
        for name in names:
            self.assertTrue(len(name) > 0)

    @unittest.skipUnless(os.path.exists(_FONTFALLBACK_FIXTURE_PATH), "FontFallback fixture missing")
    def test_is_sentinel_font(self):
        file = psapi.LayeredFile.read(_FONTFALLBACK_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Fallback Probe")
        self.assertIsNotNone(layer)

        self.assertEqual(layer.font_count, 2)
        self.assertFalse(layer.is_sentinel_font(0))  # ZZZMissingFontTok
        self.assertTrue(layer.is_sentinel_font(1))   # AdobeInvisFont

    @unittest.skipUnless(os.path.exists(_FONTFALLBACK_FIXTURE_PATH), "FontFallback fixture missing")
    def test_used_font_names_contains_patched_name(self):
        file = psapi.LayeredFile.read(_FONTFALLBACK_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Fallback Probe")
        self.assertIsNotNone(layer)

        names = layer.used_font_names()
        self.assertIn("ZZZMissingFontTok", names)

    @unittest.skipUnless(os.path.exists(_FIXTURE_PATH), "Basic fixture missing")
    def test_used_font_indices_basic(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)

        indices = layer.used_font_indices()
        self.assertGreaterEqual(len(indices), 1)
        for idx in indices:
            name = layer.font_postscript_name(idx)
            self.assertIsNotNone(name)
            self.assertTrue(len(name) > 0)


class TestFontSetWriteAPIs(unittest.TestCase):
    """Tests for add_font() and set_font_postscript_name()."""

    @staticmethod
    def _find_text_layer(file, expected_text):
        for layer in file.flat_layers:
            if _is_text_layer(layer) and layer.text == expected_text:
                return layer
        return None

    @unittest.skipUnless(os.path.exists(_FIXTURE_PATH), "Basic fixture missing")
    def test_add_font_increments_count(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)

        before = layer.font_count
        new_idx = layer.add_font("TestFont-Bold")
        self.assertEqual(layer.font_count, before + 1)
        self.assertEqual(new_idx, before)

    @unittest.skipUnless(os.path.exists(_FIXTURE_PATH), "Basic fixture missing")
    def test_add_font_name_round_trip(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)

        new_idx = layer.add_font("Helvetica-Neue")
        self.assertEqual(layer.font_postscript_name(new_idx), "Helvetica-Neue")

    @unittest.skipUnless(os.path.exists(_FIXTURE_PATH), "Basic fixture missing")
    def test_add_font_with_optional_params(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)

        new_idx = layer.add_font("MyFont-Regular", psapi.enum.FontType.TrueType, psapi.enum.FontScript.CJK)
        self.assertEqual(layer.font_type(new_idx), psapi.enum.FontType.TrueType)
        self.assertEqual(layer.font_script(new_idx), psapi.enum.FontScript.CJK)

    @unittest.skipUnless(os.path.exists(_FIXTURE_PATH), "Basic fixture missing")
    def test_set_font_postscript_name_renames(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)

        count_before = layer.font_count
        result = layer.set_font_postscript_name(0, "RenamedFont")
        self.assertIsNone(result)
        self.assertEqual(layer.font_postscript_name(0), "RenamedFont")
        self.assertEqual(layer.font_count, count_before)

    @unittest.skipUnless(os.path.exists(_FIXTURE_PATH), "Basic fixture missing")
    def test_set_font_postscript_name_out_of_range(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)

        with self.assertRaises(ValueError):
            layer.set_font_postscript_name(999, "NoSuchFont")

    @unittest.skipUnless(os.path.exists(_FIXTURE_PATH), "Basic fixture missing")
    def test_add_font_unicode_name(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)

        new_idx = layer.add_font("\u65E5\u672C\u8A9E\u30D5\u30A9\u30F3\u30C8")
        self.assertEqual(layer.font_postscript_name(new_idx), "\u65E5\u672C\u8A9E\u30D5\u30A9\u30F3\u30C8")

    @unittest.skipUnless(os.path.exists(_FIXTURE_PATH), "Basic fixture missing")
    def test_add_font_with_synthetic(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)

        new_idx = layer.add_font("SynthFont-Bold", psapi.enum.FontType.TrueType, psapi.enum.FontScript.Roman, 1)
        self.assertEqual(layer.font_synthetic(new_idx), 1)

    @unittest.skipUnless(os.path.exists(_FIXTURE_PATH), "Basic fixture missing")
    def test_find_font_index_existing(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)

        name0 = layer.font_postscript_name(0)
        self.assertIsNotNone(name0)
        self.assertEqual(layer.find_font_index(name0), 0)

    @unittest.skipUnless(os.path.exists(_FIXTURE_PATH), "Basic fixture missing")
    def test_find_font_index_missing(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)

        self.assertEqual(layer.find_font_index("NoSuchFont-12345"), -1)

    @unittest.skipUnless(os.path.exists(_FIXTURE_PATH), "Basic fixture missing")
    def test_set_style_run_font_by_name_new_font(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)

        count_before = layer.font_count
        result = layer.set_style_run_font_by_name(0, "BrandNewFont-Py")
        self.assertIsNone(result)
        self.assertEqual(layer.font_count, count_before + 1)
        # Run 0 should point at the newly-added index
        self.assertEqual(layer.style_run_font(0), count_before)

    @unittest.skipUnless(os.path.exists(_STYLE_RUNS_FIXTURE_PATH), "StyleRuns fixture missing")
    def test_set_style_run_font_by_name_existing_font(self):
        file = psapi.LayeredFile.read(_STYLE_RUNS_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Alpha Beta Gamma")
        self.assertIsNotNone(layer)

        existing_name = layer.font_postscript_name(0)
        count_before = layer.font_count
        result = layer.set_style_run_font_by_name(0, existing_name)
        self.assertIsNone(result)
        # Should NOT have added a new font
        self.assertEqual(layer.font_count, count_before)

    @unittest.skipUnless(os.path.exists(_FIXTURE_PATH), "Basic fixture missing")
    def test_set_style_normal_font_by_name_new_font(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)

        count_before = layer.font_count
        result = layer.set_style_normal_font_by_name("NormalNewFont-Py")
        self.assertIsNone(result)
        self.assertEqual(layer.font_count, count_before + 1)
        self.assertEqual(layer.style_normal_font(), count_before)

    @unittest.skipUnless(os.path.exists(_FIXTURE_PATH), "Basic fixture missing")
    def test_set_style_normal_font_by_name_existing_font(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)

        existing_name = layer.font_postscript_name(0)
        count_before = layer.font_count
        result = layer.set_style_normal_font_by_name(existing_name)
        self.assertIsNone(result)
        self.assertEqual(layer.font_count, count_before)

    # ── Step 7: Legacy From/To remap tests ─────────────────────────────────

    @unittest.skipUnless(os.path.exists(_STYLE_RUNS_FIXTURE_PATH), "Style-runs fixture missing")
    def test_legacy_remap_grow_mutation_roundtrips(self):
        """Variable-length grow mutation on multi-style file roundtrips with legacy remap enabled."""
        file = psapi.LayeredFile.read(_STYLE_RUNS_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Alpha Beta Gamma")
        self.assertIsNotNone(layer)

        layer.replace_text("Beta", "BetaBetaBeta")
        self.assertEqual(layer.text, "Alpha BetaBetaBeta Gamma")

        with tempfile.NamedTemporaryFile(suffix=".psd", delete=False) as f:
            tmp_path = f.name
        try:
            file.write(tmp_path)
            reread = psapi.LayeredFile.read(tmp_path)
            reread_layer = self._find_text_layer(reread, "Alpha BetaBetaBeta Gamma")
            self.assertIsNotNone(reread_layer)
            self.assertEqual(reread_layer.style_run_count, 5)
        finally:
            os.remove(tmp_path)

    @unittest.skipUnless(os.path.exists(_STYLE_RUNS_FIXTURE_PATH), "Style-runs fixture missing")
    def test_legacy_remap_shrink_mutation_roundtrips(self):
        """Variable-length shrink mutation on multi-style file roundtrips with legacy remap enabled."""
        file = psapi.LayeredFile.read(_STYLE_RUNS_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Alpha Beta Gamma")
        self.assertIsNotNone(layer)

        layer.replace_text("Alpha", "A")
        self.assertEqual(layer.text, "A Beta Gamma")

        with tempfile.NamedTemporaryFile(suffix=".psd", delete=False) as f:
            tmp_path = f.name
        try:
            file.write(tmp_path)
            reread = psapi.LayeredFile.read(tmp_path)
            reread_layer = self._find_text_layer(reread, "A Beta Gamma")
            self.assertIsNotNone(reread_layer)
            self.assertEqual(reread_layer.style_run_count, 5)
        finally:
            os.remove(tmp_path)

    @unittest.skipUnless(os.path.exists(_STYLE_RUNS_FIXTURE_PATH), "Style-runs fixture missing")
    def test_legacy_remap_same_length_mutation_roundtrips(self):
        """Same-length mutation preserves TySh descriptor integrity."""
        file = psapi.LayeredFile.read(_STYLE_RUNS_FIXTURE_PATH)
        layer = self._find_text_layer(file, "Alpha Beta Gamma")
        self.assertIsNotNone(layer)

        layer.replace_text("Beta", "XXXX")
        self.assertEqual(layer.text, "Alpha XXXX Gamma")

        with tempfile.NamedTemporaryFile(suffix=".psd", delete=False) as f:
            tmp_path = f.name
        try:
            file.write(tmp_path)
            reread = psapi.LayeredFile.read(tmp_path)
            reread_layer = self._find_text_layer(reread, "Alpha XXXX Gamma")
            self.assertIsNotNone(reread_layer)
            self.assertEqual(reread_layer.style_run_count, 5)
        finally:
            os.remove(tmp_path)


@unittest.skipUnless(os.path.exists(_FIXTURE_PATH), "Basic fixture missing")
class TestFontConvenience(unittest.TestCase):
    """primary_font_name / set_font convenience APIs."""

    def test_primary_font_name_non_empty(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "SimpleASCII")
        self.assertIsNotNone(layer)
        self.assertIsNotNone(layer.primary_font_name)
        self.assertGreater(len(layer.primary_font_name), 0)

    def test_set_font_changes_run_and_normal(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer_by_name(file, "SimpleASCII")
        self.assertIsNotNone(layer)
        old_name = layer.primary_font_name
        layer.set_font("TestPySetFont")
        self.assertEqual(layer.primary_font_name, "TestPySetFont")
        self.assertNotEqual(layer.primary_font_name, old_name)
