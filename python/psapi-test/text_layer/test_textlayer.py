import os
import tempfile
import unittest

import photoshopapi as psapi

from ._textlayer_helpers import (
    FIXTURE_PATH,
    STYLE_RUNS_FIXTURE_PATH,
    PARAGRAPH_FIXTURE_PATH,
    CHARACTER_STYLES_FIXTURE_PATH,
    is_text_layer,
    find_text_layer,
    find_text_layer_containing,
    find_text_layer_by_name,
)

_FIXTURE_PATH = FIXTURE_PATH
_STYLE_RUNS_FIXTURE_PATH = STYLE_RUNS_FIXTURE_PATH
_PARAGRAPH_FIXTURE_PATH = PARAGRAPH_FIXTURE_PATH
_CHARACTER_STYLES_FIXTURE_PATH = CHARACTER_STYLES_FIXTURE_PATH
_find_text_layer = find_text_layer
_find_text_layer_by_name = find_text_layer_by_name
_find_text_layer_containing = find_text_layer_containing
_is_text_layer = is_text_layer


@unittest.skipUnless(os.path.exists(_FIXTURE_PATH), "Text layer fixture is missing")
class TestTextLayer(unittest.TestCase):
    fixture_path = _FIXTURE_PATH
    style_runs_fixture_path = _STYLE_RUNS_FIXTURE_PATH
    paragraph_fixture_path = _PARAGRAPH_FIXTURE_PATH
    character_styles_fixture_path = _CHARACTER_STYLES_FIXTURE_PATH

    def test_detect_and_read_text(self):
        file = psapi.LayeredFile.read(self.fixture_path)
        layer = _find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)
        self.assertEqual(layer.text, "Hello 123")

    def test_replace_text_roundtrip(self):
        file = psapi.LayeredFile.read(self.fixture_path)
        layer = _find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)

        self.assertTrue(layer.replace_text("Hello", "Greetings"))
        self.assertEqual(layer.text, "Greetings 123")

        with tempfile.NamedTemporaryFile(suffix=".psd", delete=False) as tmp:
            out_path = tmp.name

        # The write API creates/overwrites the path itself.
        os.remove(out_path)
        try:
            file.write(out_path)
            self.assertTrue(os.path.exists(out_path))

            reread = psapi.LayeredFile.read(out_path)
            reread_layer = _find_text_layer(reread, "Greetings 123")
            self.assertIsNotNone(reread_layer)
            self.assertEqual(reread_layer.text, "Greetings 123")
        finally:
            if os.path.exists(out_path):
                os.remove(out_path)

    def test_equal_length_replacement_guard(self):
        file = psapi.LayeredFile.read(self.fixture_path)
        layer = _find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)

        self.assertFalse(layer.replace_text_equal_length("Hello", "Greetings"))
        self.assertTrue(layer.replace_text_equal_length("Hello", "Hallo"))
        self.assertEqual(layer.text, "Hallo 123")

    @unittest.skipUnless(os.path.exists(_STYLE_RUNS_FIXTURE_PATH), "Style-runs text layer fixture is missing")
    def test_style_run_mutation_roundtrip(self):
        file = psapi.LayeredFile.read(self.style_runs_fixture_path)
        layer = _find_text_layer(file, "Alpha Beta Gamma")
        self.assertIsNotNone(layer)

        self.assertEqual(layer.style_run_count, 5)
        self.assertAlmostEqual(layer.style_run_font_size(2), 44.0, places=3)

        fill_before = layer.style_run_fill_color(2)
        self.assertIsNotNone(fill_before)
        self.assertEqual(len(fill_before), 4)

        self.assertTrue(layer.set_style_run_font_size(2, 48.0))
        self.assertTrue(layer.set_style_run_fill_color(2, [1.0, 0.1, 0.25, 0.75]))
        self.assertFalse(layer.set_style_run_fill_color(2, [1.0, 0.1, 0.25]))
        self.assertFalse(layer.set_style_run_font_size(200, 48.0))

        with tempfile.NamedTemporaryFile(suffix=".psd", delete=False) as tmp:
            out_path = tmp.name

        os.remove(out_path)
        try:
            file.write(out_path)
            reread = psapi.LayeredFile.read(out_path)
            reread_layer = _find_text_layer(reread, "Alpha Beta Gamma")
            self.assertIsNotNone(reread_layer)

            self.assertAlmostEqual(reread_layer.style_run_font_size(2), 48.0, places=3)
            fill_after = reread_layer.style_run_fill_color(2)
            self.assertIsNotNone(fill_after)
            self.assertEqual(len(fill_after), 4)
            self.assertAlmostEqual(fill_after[0], 1.0, places=3)
            self.assertAlmostEqual(fill_after[1], 0.1, places=3)
            self.assertAlmostEqual(fill_after[2], 0.25, places=3)
            self.assertAlmostEqual(fill_after[3], 0.75, places=3)
        finally:
            if os.path.exists(out_path):
                os.remove(out_path)

    @unittest.skipUnless(os.path.exists(_CHARACTER_STYLES_FIXTURE_PATH), "Character-styles text layer fixture is missing")
    def test_character_styles_fixture_has_expected_underline_and_no_warp(self):
        file = psapi.LayeredFile.read(self.character_styles_fixture_path)
        layer = _find_text_layer_by_name(file, "CharacterStylePrimary")
        self.assertIsNotNone(layer)

        self.assertFalse(layer.is_vertical)
        self.assertFalse(layer.has_warp)
        self.assertGreaterEqual(layer.style_run_count, 1)
        self.assertTrue(layer.style_run_underline(0))

    @unittest.skipUnless(os.path.exists(_STYLE_RUNS_FIXTURE_PATH), "Style-runs text layer fixture is missing")
    def test_style_runs_fixture_has_expected_emphasis_and_no_warp(self):
        file = psapi.LayeredFile.read(self.style_runs_fixture_path)
        layer = _find_text_layer_by_name(file, "MixedRuns")
        self.assertIsNotNone(layer)

        self.assertFalse(layer.is_vertical)
        self.assertFalse(layer.has_warp)
        self.assertGreaterEqual(layer.style_run_count, 5)
        self.assertTrue(layer.style_run_underline(2))
        self.assertTrue(layer.style_run_faux_bold(2))
        self.assertTrue(layer.style_run_faux_italic(4))

    @unittest.skipUnless(os.path.exists(_CHARACTER_STYLES_FIXTURE_PATH), "Character-styles text layer fixture is missing")
    def test_style_run_character_properties_roundtrip(self):
        file = psapi.LayeredFile.read(self.character_styles_fixture_path)
        layer = _find_text_layer_by_name(file, "CharacterStylePrimary")
        self.assertIsNotNone(layer)

        self.assertGreaterEqual(layer.style_run_count, 1)
        run_font_before = layer.style_run_font(0)
        run_font_size_before = layer.style_run_font_size(0)
        run_faux_bold_before = layer.style_run_faux_bold(0)
        run_faux_italic_before = layer.style_run_faux_italic(0)
        run_horizontal_scale_before = layer.style_run_horizontal_scale(0)
        run_vertical_scale_before = layer.style_run_vertical_scale(0)
        run_tracking_before = layer.style_run_tracking(0)
        run_auto_kerning_before = layer.style_run_auto_kerning(0)
        run_baseline_shift_before = layer.style_run_baseline_shift(0)
        run_leading_before = layer.style_run_leading(0)
        run_auto_leading_before = layer.style_run_auto_leading(0)
        run_kerning_before = layer.style_run_kerning(0)
        run_font_caps_before = layer.style_run_font_caps(0)
        run_no_break_before = layer.style_run_no_break(0)
        run_font_baseline_before = layer.style_run_font_baseline(0)
        run_language_before = layer.style_run_language(0)
        run_character_direction_before = layer.style_run_character_direction(0)
        run_baseline_direction_before = layer.style_run_baseline_direction(0)
        run_tsume_before = layer.style_run_tsume(0)
        run_kashida_before = layer.style_run_kashida(0)
        run_diacritic_pos_before = layer.style_run_diacritic_pos(0)
        run_ligatures_before = layer.style_run_ligatures(0)
        run_dligatures_before = layer.style_run_dligatures(0)
        run_underline_before = layer.style_run_underline(0)
        run_strikethrough_before = layer.style_run_strikethrough(0)
        run_stroke_flag_before = layer.style_run_stroke_flag(0)
        run_fill_flag_before = layer.style_run_fill_flag(0)
        run_fill_first_before = layer.style_run_fill_first(0)
        run_outline_width_before = layer.style_run_outline_width(0)
        run_fill_color_before = layer.style_run_fill_color(0)
        run_stroke_color_before = layer.style_run_stroke_color(0)
        normal_font_size_before = layer.style_normal_font_size()

        self.assertIsNotNone(run_font_before)
        self.assertIsNotNone(run_font_size_before)
        self.assertIsNotNone(run_faux_bold_before)
        self.assertIsNotNone(run_faux_italic_before)
        self.assertIsNotNone(run_horizontal_scale_before)
        self.assertIsNotNone(run_vertical_scale_before)
        self.assertIsNotNone(run_tracking_before)
        self.assertIsNotNone(run_auto_kerning_before)
        self.assertIsNotNone(run_baseline_shift_before)
        self.assertIsNotNone(run_leading_before)
        self.assertIsNotNone(run_auto_leading_before)
        self.assertIsNotNone(run_kerning_before)
        self.assertIsNotNone(run_font_caps_before)
        self.assertIsNotNone(run_no_break_before)
        self.assertIsNotNone(run_font_baseline_before)
        self.assertIsNotNone(run_language_before)
        self.assertIsNotNone(run_baseline_direction_before)
        self.assertIsNotNone(run_tsume_before)
        self.assertIsNotNone(run_kashida_before)
        self.assertIsNotNone(run_ligatures_before)
        self.assertIsNotNone(run_dligatures_before)
        self.assertIsNotNone(run_underline_before)
        self.assertIsNotNone(run_strikethrough_before)
        self.assertIsNotNone(run_fill_color_before)
        self.assertEqual(len(run_fill_color_before), 4)
        if run_stroke_color_before is not None:
            self.assertEqual(len(run_stroke_color_before), 4)
        self.assertIsNotNone(normal_font_size_before)

        run_font_after = 1 if run_font_before == 0 else 0
        run_font_size_after = run_font_size_before + 2.0
        run_faux_bold_after = not run_faux_bold_before
        run_faux_italic_after = not run_faux_italic_before
        run_horizontal_scale_after = run_horizontal_scale_before + 0.05
        run_vertical_scale_after = run_vertical_scale_before + 0.05
        run_tracking_after = run_tracking_before + 8
        run_auto_kerning_after = not run_auto_kerning_before
        run_baseline_shift_after = run_baseline_shift_before + 1.0
        run_leading_after = run_leading_before + 1.25
        run_auto_leading_after = not run_auto_leading_before
        run_kerning_after = run_kerning_before + 12
        run_font_caps_after = psapi.enum.FontCaps.AllCaps if run_font_caps_before == psapi.enum.FontCaps.Normal else psapi.enum.FontCaps.Normal
        run_no_break_after = not run_no_break_before
        run_font_baseline_after = psapi.enum.FontBaseline.Superscript if run_font_baseline_before == psapi.enum.FontBaseline.Normal else psapi.enum.FontBaseline.Normal
        run_language_after = run_language_before + 1
        run_character_direction_after = (psapi.enum.CharacterDirection.LeftToRight if run_character_direction_before == psapi.enum.CharacterDirection.Default else psapi.enum.CharacterDirection.Default) if run_character_direction_before is not None else None
        run_baseline_direction_after = psapi.enum.BaselineDirection.Vertical if run_baseline_direction_before == psapi.enum.BaselineDirection.Default else psapi.enum.BaselineDirection.Default
        run_tsume_after = run_tsume_before + 5.0
        run_kashida_after = run_kashida_before + 1
        run_diacritic_pos_after = (psapi.enum.DiacriticPosition.Loose if run_diacritic_pos_before == psapi.enum.DiacriticPosition.OpenType else psapi.enum.DiacriticPosition.OpenType) if run_diacritic_pos_before is not None else None
        run_ligatures_after = not run_ligatures_before
        run_dligatures_after = not run_dligatures_before
        run_underline_after = not run_underline_before
        run_strikethrough_after = not run_strikethrough_before
        run_stroke_flag_after = (not run_stroke_flag_before) if run_stroke_flag_before is not None else None
        run_fill_flag_after = (not run_fill_flag_before) if run_fill_flag_before is not None else None
        run_fill_first_after = (not run_fill_first_before) if run_fill_first_before is not None else None
        run_outline_width_after = (run_outline_width_before + 1.0) if run_outline_width_before is not None else None
        run_fill_color_after = [
            run_fill_color_before[0],
            min(1.0, max(0.0, run_fill_color_before[1] + 0.05)),
            min(1.0, max(0.0, run_fill_color_before[2] + 0.05)),
            min(1.0, max(0.0, run_fill_color_before[3] + 0.05)),
        ]
        run_stroke_color_after = None
        if run_stroke_color_before is not None and len(run_stroke_color_before) == 4:
            run_stroke_color_after = [
                run_stroke_color_before[0],
                min(1.0, max(0.0, run_stroke_color_before[1] + 0.05)),
                min(1.0, max(0.0, run_stroke_color_before[2] + 0.05)),
                min(1.0, max(0.0, run_stroke_color_before[3] + 0.05)),
            ]

        self.assertTrue(layer.set_style_run_font(0, run_font_after))
        self.assertTrue(layer.set_style_run_font_size(0, run_font_size_after))
        self.assertTrue(layer.set_style_run_faux_bold(0, run_faux_bold_after))
        self.assertTrue(layer.set_style_run_faux_italic(0, run_faux_italic_after))
        self.assertTrue(layer.set_style_run_horizontal_scale(0, run_horizontal_scale_after))
        self.assertTrue(layer.set_style_run_vertical_scale(0, run_vertical_scale_after))
        self.assertTrue(layer.set_style_run_tracking(0, run_tracking_after))
        self.assertTrue(layer.set_style_run_auto_kerning(0, run_auto_kerning_after))
        self.assertTrue(layer.set_style_run_baseline_shift(0, run_baseline_shift_after))
        self.assertTrue(layer.set_style_run_leading(0, run_leading_after))
        self.assertTrue(layer.set_style_run_auto_leading(0, run_auto_leading_after))
        self.assertTrue(layer.set_style_run_kerning(0, run_kerning_after))
        self.assertTrue(layer.set_style_run_font_caps(0, run_font_caps_after))
        self.assertTrue(layer.set_style_run_no_break(0, run_no_break_after))
        self.assertTrue(layer.set_style_run_font_baseline(0, run_font_baseline_after))
        self.assertTrue(layer.set_style_run_language(0, run_language_after))
        expected_character_direction_after = run_character_direction_after
        if expected_character_direction_after is None:
            expected_character_direction_after = psapi.enum.CharacterDirection.LeftToRight
        self.assertTrue(layer.set_style_run_character_direction(0, expected_character_direction_after))
        self.assertTrue(layer.set_style_run_baseline_direction(0, run_baseline_direction_after))
        self.assertTrue(layer.set_style_run_tsume(0, run_tsume_after))
        self.assertTrue(layer.set_style_run_kashida(0, run_kashida_after))
        expected_diacritic_pos_after = run_diacritic_pos_after
        if expected_diacritic_pos_after is None:
            expected_diacritic_pos_after = psapi.enum.DiacriticPosition.Loose
        self.assertTrue(layer.set_style_run_diacritic_pos(0, expected_diacritic_pos_after))
        self.assertTrue(layer.set_style_run_ligatures(0, run_ligatures_after))
        self.assertTrue(layer.set_style_run_dligatures(0, run_dligatures_after))
        self.assertTrue(layer.set_style_run_underline(0, run_underline_after))
        self.assertTrue(layer.set_style_run_strikethrough(0, run_strikethrough_after))
        if run_stroke_flag_after is not None:
            self.assertTrue(layer.set_style_run_stroke_flag(0, run_stroke_flag_after))
        else:
            self.assertTrue(layer.set_style_run_stroke_flag(0, True))
        if run_fill_flag_after is not None:
            self.assertTrue(layer.set_style_run_fill_flag(0, run_fill_flag_after))
        else:
            self.assertTrue(layer.set_style_run_fill_flag(0, True))
        if run_fill_first_after is not None:
            self.assertTrue(layer.set_style_run_fill_first(0, run_fill_first_after))
        else:
            self.assertTrue(layer.set_style_run_fill_first(0, False))
        expected_outline_width_after = run_outline_width_after
        if expected_outline_width_after is None:
            expected_outline_width_after = 2.0
        self.assertTrue(layer.set_style_run_outline_width(0, expected_outline_width_after))
        self.assertTrue(layer.set_style_run_fill_color(0, run_fill_color_after))
        expected_stroke_color_after = run_stroke_color_after
        if expected_stroke_color_after is None:
            expected_stroke_color_after = [1.0, 0.1, 0.2, 0.3]
        self.assertTrue(layer.set_style_run_stroke_color(0, expected_stroke_color_after))
        self.assertFalse(layer.set_style_run_font(200, run_font_after))
        self.assertFalse(layer.set_style_run_font_size(0, float("inf")))
        self.assertFalse(layer.set_style_run_leading(200, run_leading_after))
        self.assertFalse(layer.set_style_run_auto_leading(200, run_auto_leading_after))
        self.assertFalse(layer.set_style_run_kerning(200, run_kerning_after))
        self.assertFalse(layer.set_style_run_font_baseline(200, run_font_baseline_after))
        self.assertFalse(layer.set_style_run_language(200, run_language_after))
        self.assertFalse(layer.set_style_run_baseline_direction(200, run_baseline_direction_after))
        self.assertFalse(layer.set_style_run_tsume(200, run_tsume_after))
        self.assertFalse(layer.set_style_run_kashida(200, run_kashida_after))
        self.assertFalse(layer.set_style_run_stroke_flag(200, True))
        self.assertFalse(layer.set_style_run_fill_flag(200, True))
        self.assertFalse(layer.set_style_run_fill_first(200, True))
        self.assertFalse(layer.set_style_run_outline_width(200, 1.0))
        self.assertFalse(layer.set_style_run_fill_color(0, []))
        self.assertFalse(layer.set_style_run_fill_color(0, [1.0, 0.0, 0.0]))
        self.assertFalse(layer.set_style_run_stroke_color(0, []))
        self.assertFalse(layer.set_style_run_stroke_color(0, [1.0, 0.0, 0.0]))

        with tempfile.NamedTemporaryFile(suffix=".psd", delete=False) as tmp:
            out_path = tmp.name

        os.remove(out_path)
        try:
            file.write(out_path)
            reread = psapi.LayeredFile.read(out_path)
            reread_layer = _find_text_layer_by_name(reread, "CharacterStylePrimary")
            self.assertIsNotNone(reread_layer)

            self.assertEqual(reread_layer.style_run_font(0), run_font_after)
            self.assertAlmostEqual(reread_layer.style_run_font_size(0), run_font_size_after, places=3)
            self.assertEqual(reread_layer.style_run_faux_bold(0), run_faux_bold_after)
            self.assertEqual(reread_layer.style_run_faux_italic(0), run_faux_italic_after)
            self.assertAlmostEqual(reread_layer.style_run_horizontal_scale(0), run_horizontal_scale_after, places=3)
            self.assertAlmostEqual(reread_layer.style_run_vertical_scale(0), run_vertical_scale_after, places=3)
            self.assertEqual(reread_layer.style_run_tracking(0), run_tracking_after)
            self.assertEqual(reread_layer.style_run_auto_kerning(0), run_auto_kerning_after)
            self.assertAlmostEqual(reread_layer.style_run_baseline_shift(0), run_baseline_shift_after, places=3)
            self.assertAlmostEqual(reread_layer.style_run_leading(0), run_leading_after, places=3)
            self.assertEqual(reread_layer.style_run_auto_leading(0), run_auto_leading_after)
            self.assertEqual(reread_layer.style_run_kerning(0), run_kerning_after)
            self.assertEqual(reread_layer.style_run_font_caps(0), run_font_caps_after)
            self.assertEqual(reread_layer.style_run_no_break(0), run_no_break_after)
            self.assertEqual(reread_layer.style_run_font_baseline(0), run_font_baseline_after)
            self.assertEqual(reread_layer.style_run_language(0), run_language_after)
            self.assertEqual(reread_layer.style_run_character_direction(0), expected_character_direction_after)
            self.assertEqual(reread_layer.style_run_baseline_direction(0), run_baseline_direction_after)
            self.assertAlmostEqual(reread_layer.style_run_tsume(0), run_tsume_after, places=3)
            self.assertEqual(reread_layer.style_run_kashida(0), run_kashida_after)
            self.assertEqual(reread_layer.style_run_diacritic_pos(0), expected_diacritic_pos_after)
            self.assertEqual(reread_layer.style_run_ligatures(0), run_ligatures_after)
            self.assertEqual(reread_layer.style_run_dligatures(0), run_dligatures_after)
            self.assertEqual(reread_layer.style_run_underline(0), run_underline_after)
            self.assertEqual(reread_layer.style_run_strikethrough(0), run_strikethrough_after)
            if run_stroke_flag_after is not None:
                self.assertEqual(reread_layer.style_run_stroke_flag(0), run_stroke_flag_after)
            else:
                self.assertTrue(reread_layer.style_run_stroke_flag(0))
            if run_fill_flag_after is not None:
                self.assertEqual(reread_layer.style_run_fill_flag(0), run_fill_flag_after)
            else:
                self.assertTrue(reread_layer.style_run_fill_flag(0))
            if run_fill_first_after is not None:
                self.assertEqual(reread_layer.style_run_fill_first(0), run_fill_first_after)
            else:
                self.assertFalse(reread_layer.style_run_fill_first(0))
            self.assertAlmostEqual(reread_layer.style_run_outline_width(0), expected_outline_width_after, places=3)
            reread_fill = reread_layer.style_run_fill_color(0)
            self.assertIsNotNone(reread_fill)
            self.assertEqual(len(reread_fill), 4)
            self.assertAlmostEqual(reread_fill[0], run_fill_color_after[0], places=3)
            self.assertAlmostEqual(reread_fill[1], run_fill_color_after[1], places=3)
            self.assertAlmostEqual(reread_fill[2], run_fill_color_after[2], places=3)
            self.assertAlmostEqual(reread_fill[3], run_fill_color_after[3], places=3)
            reread_stroke = reread_layer.style_run_stroke_color(0)
            self.assertIsNotNone(reread_stroke)
            self.assertEqual(len(reread_stroke), 4)
            self.assertAlmostEqual(reread_stroke[0], expected_stroke_color_after[0], places=3)
            self.assertAlmostEqual(reread_stroke[1], expected_stroke_color_after[1], places=3)
            self.assertAlmostEqual(reread_stroke[2], expected_stroke_color_after[2], places=3)
            self.assertAlmostEqual(reread_stroke[3], expected_stroke_color_after[3], places=3)

            # Style-run mutation should not rewrite normal style-sheet defaults.
            self.assertAlmostEqual(reread_layer.style_normal_font_size(), normal_font_size_before, places=3)
        finally:
            if os.path.exists(out_path):
                os.remove(out_path)

    @unittest.skipUnless(os.path.exists(_CHARACTER_STYLES_FIXTURE_PATH), "Character-styles text layer fixture is missing")
    def test_style_normal_sheet_roundtrip(self):
        file = psapi.LayeredFile.read(self.character_styles_fixture_path)
        layer = _find_text_layer_by_name(file, "CharacterStylePrimary")
        self.assertIsNotNone(layer)

        self.assertGreaterEqual(layer.style_sheet_count, 1)
        normal_sheet_index_before = layer.style_normal_sheet_index()
        normal_font_before = layer.style_normal_font()
        normal_font_size_before = layer.style_normal_font_size()
        normal_leading_before = layer.style_normal_leading()
        normal_auto_leading_before = layer.style_normal_auto_leading()
        normal_kerning_before = layer.style_normal_kerning()
        normal_faux_bold_before = layer.style_normal_faux_bold()
        normal_faux_italic_before = layer.style_normal_faux_italic()
        normal_horizontal_scale_before = layer.style_normal_horizontal_scale()
        normal_vertical_scale_before = layer.style_normal_vertical_scale()
        normal_tracking_before = layer.style_normal_tracking()
        normal_auto_kerning_before = layer.style_normal_auto_kerning()
        normal_baseline_shift_before = layer.style_normal_baseline_shift()
        normal_font_caps_before = layer.style_normal_font_caps()
        normal_font_baseline_before = layer.style_normal_font_baseline()
        normal_no_break_before = layer.style_normal_no_break()
        normal_language_before = layer.style_normal_language()
        normal_character_direction_before = layer.style_normal_character_direction()
        normal_baseline_direction_before = layer.style_normal_baseline_direction()
        normal_tsume_before = layer.style_normal_tsume()
        normal_kashida_before = layer.style_normal_kashida()
        normal_diacritic_pos_before = layer.style_normal_diacritic_pos()
        normal_ligatures_before = layer.style_normal_ligatures()
        normal_dligatures_before = layer.style_normal_dligatures()
        normal_underline_before = layer.style_normal_underline()
        normal_strikethrough_before = layer.style_normal_strikethrough()
        normal_stroke_flag_before = layer.style_normal_stroke_flag()
        normal_fill_flag_before = layer.style_normal_fill_flag()
        normal_fill_first_before = layer.style_normal_fill_first()
        normal_outline_width_before = layer.style_normal_outline_width()
        normal_fill_color_before = layer.style_normal_fill_color()
        normal_stroke_color_before = layer.style_normal_stroke_color()
        run_font_size_before = layer.style_run_font_size(0)

        self.assertIsNotNone(normal_sheet_index_before)
        self.assertIsNotNone(normal_font_before)
        self.assertIsNotNone(normal_font_size_before)
        self.assertIsNotNone(normal_leading_before)
        self.assertIsNotNone(normal_auto_leading_before)
        self.assertIsNotNone(normal_kerning_before)
        self.assertIsNotNone(normal_faux_bold_before)
        self.assertIsNotNone(normal_faux_italic_before)
        self.assertIsNotNone(normal_horizontal_scale_before)
        self.assertIsNotNone(normal_vertical_scale_before)
        self.assertIsNotNone(normal_tracking_before)
        self.assertIsNotNone(normal_auto_kerning_before)
        self.assertIsNotNone(normal_baseline_shift_before)
        self.assertIsNotNone(normal_font_caps_before)
        self.assertIsNotNone(normal_font_baseline_before)
        self.assertIsNotNone(normal_no_break_before)
        self.assertIsNotNone(normal_language_before)
        self.assertIsNotNone(normal_character_direction_before)
        self.assertIsNotNone(normal_baseline_direction_before)
        self.assertIsNotNone(normal_tsume_before)
        self.assertIsNotNone(normal_kashida_before)
        self.assertIsNotNone(normal_diacritic_pos_before)
        self.assertIsNotNone(normal_ligatures_before)
        self.assertIsNotNone(normal_dligatures_before)
        self.assertIsNotNone(normal_underline_before)
        self.assertIsNotNone(normal_strikethrough_before)
        self.assertIsNotNone(normal_stroke_flag_before)
        self.assertIsNotNone(normal_fill_flag_before)
        self.assertIsNotNone(normal_fill_first_before)
        self.assertIsNotNone(normal_outline_width_before)
        self.assertIsNotNone(normal_fill_color_before)
        self.assertEqual(len(normal_fill_color_before), 4)
        self.assertIsNotNone(normal_stroke_color_before)
        self.assertEqual(len(normal_stroke_color_before), 4)
        self.assertIsNotNone(run_font_size_before)

        normal_font_after = 1 if normal_font_before == 0 else 0
        normal_font_size_after = normal_font_size_before + 3.0
        normal_leading_after = normal_leading_before + 2.0
        normal_auto_leading_after = not normal_auto_leading_before
        normal_kerning_after = normal_kerning_before + 12
        normal_faux_bold_after = not normal_faux_bold_before
        normal_faux_italic_after = not normal_faux_italic_before
        normal_horizontal_scale_after = normal_horizontal_scale_before + 0.05
        normal_vertical_scale_after = normal_vertical_scale_before + 0.05
        normal_tracking_after = normal_tracking_before + 12
        normal_auto_kerning_after = not normal_auto_kerning_before
        normal_baseline_shift_after = normal_baseline_shift_before + 2.5
        normal_font_caps_after = psapi.enum.FontCaps.AllCaps if normal_font_caps_before == psapi.enum.FontCaps.Normal else psapi.enum.FontCaps.Normal
        normal_font_baseline_after = psapi.enum.FontBaseline.Superscript if normal_font_baseline_before == psapi.enum.FontBaseline.Normal else psapi.enum.FontBaseline.Normal
        normal_no_break_after = not normal_no_break_before
        normal_language_after = normal_language_before + 1
        normal_character_direction_after = psapi.enum.CharacterDirection.LeftToRight if normal_character_direction_before == psapi.enum.CharacterDirection.Default else psapi.enum.CharacterDirection.Default
        normal_baseline_direction_after = psapi.enum.BaselineDirection.Vertical if normal_baseline_direction_before == psapi.enum.BaselineDirection.Default else psapi.enum.BaselineDirection.Default
        normal_tsume_after = normal_tsume_before + 5.0
        normal_kashida_after = normal_kashida_before + 1
        normal_diacritic_pos_after = psapi.enum.DiacriticPosition.Loose if normal_diacritic_pos_before == psapi.enum.DiacriticPosition.OpenType else psapi.enum.DiacriticPosition.OpenType
        normal_ligatures_after = not normal_ligatures_before
        normal_dligatures_after = not normal_dligatures_before
        normal_underline_after = not normal_underline_before
        normal_strikethrough_after = not normal_strikethrough_before
        normal_stroke_flag_after = not normal_stroke_flag_before
        normal_fill_flag_after = not normal_fill_flag_before
        normal_fill_first_after = not normal_fill_first_before
        normal_outline_width_after = normal_outline_width_before + 1.0
        normal_fill_color_after = [
            normal_fill_color_before[0],
            min(1.0, max(0.0, normal_fill_color_before[1] + 0.1)),
            min(1.0, max(0.0, normal_fill_color_before[2] + 0.1)),
            min(1.0, max(0.0, normal_fill_color_before[3] + 0.1)),
        ]
        normal_stroke_color_after = [
            normal_stroke_color_before[0],
            min(1.0, max(0.0, normal_stroke_color_before[1] + 0.1)),
            min(1.0, max(0.0, normal_stroke_color_before[2] + 0.1)),
            min(1.0, max(0.0, normal_stroke_color_before[3] + 0.1)),
        ]

        self.assertTrue(layer.set_style_normal_sheet_index(normal_sheet_index_before))
        self.assertTrue(layer.set_style_normal_font(normal_font_after))
        self.assertTrue(layer.set_style_normal_font_size(normal_font_size_after))
        self.assertTrue(layer.set_style_normal_leading(normal_leading_after))
        self.assertTrue(layer.set_style_normal_auto_leading(normal_auto_leading_after))
        self.assertTrue(layer.set_style_normal_kerning(normal_kerning_after))
        self.assertTrue(layer.set_style_normal_faux_bold(normal_faux_bold_after))
        self.assertTrue(layer.set_style_normal_faux_italic(normal_faux_italic_after))
        self.assertTrue(layer.set_style_normal_horizontal_scale(normal_horizontal_scale_after))
        self.assertTrue(layer.set_style_normal_vertical_scale(normal_vertical_scale_after))
        self.assertTrue(layer.set_style_normal_tracking(normal_tracking_after))
        self.assertTrue(layer.set_style_normal_auto_kerning(normal_auto_kerning_after))
        self.assertTrue(layer.set_style_normal_baseline_shift(normal_baseline_shift_after))
        self.assertTrue(layer.set_style_normal_font_caps(normal_font_caps_after))
        self.assertTrue(layer.set_style_normal_font_baseline(normal_font_baseline_after))
        self.assertTrue(layer.set_style_normal_no_break(normal_no_break_after))
        self.assertTrue(layer.set_style_normal_language(normal_language_after))
        self.assertTrue(layer.set_style_normal_character_direction(normal_character_direction_after))
        self.assertTrue(layer.set_style_normal_baseline_direction(normal_baseline_direction_after))
        self.assertTrue(layer.set_style_normal_tsume(normal_tsume_after))
        self.assertTrue(layer.set_style_normal_kashida(normal_kashida_after))
        self.assertTrue(layer.set_style_normal_diacritic_pos(normal_diacritic_pos_after))
        self.assertTrue(layer.set_style_normal_ligatures(normal_ligatures_after))
        self.assertTrue(layer.set_style_normal_dligatures(normal_dligatures_after))
        self.assertTrue(layer.set_style_normal_underline(normal_underline_after))
        self.assertTrue(layer.set_style_normal_strikethrough(normal_strikethrough_after))
        self.assertTrue(layer.set_style_normal_stroke_flag(normal_stroke_flag_after))
        self.assertTrue(layer.set_style_normal_fill_flag(normal_fill_flag_after))
        self.assertTrue(layer.set_style_normal_fill_first(normal_fill_first_after))
        self.assertTrue(layer.set_style_normal_outline_width(normal_outline_width_after))
        self.assertTrue(layer.set_style_normal_fill_color(normal_fill_color_after))
        self.assertTrue(layer.set_style_normal_stroke_color(normal_stroke_color_after))
        self.assertFalse(layer.set_style_normal_sheet_index(-1))
        self.assertFalse(layer.set_style_normal_sheet_index(200))
        self.assertFalse(layer.set_style_normal_font_size(float("inf")))
        self.assertFalse(layer.set_style_normal_fill_color([]))
        self.assertFalse(layer.set_style_normal_fill_color([1.0, 0.0, 0.0]))
        self.assertFalse(layer.set_style_normal_fill_color([1.0, 0.0, float("inf"), 0.0]))
        self.assertFalse(layer.set_style_normal_stroke_color([]))
        self.assertFalse(layer.set_style_normal_stroke_color([1.0, 0.0, 0.0]))
        self.assertFalse(layer.set_style_normal_stroke_color([1.0, 0.0, float("inf"), 0.0]))

        with tempfile.NamedTemporaryFile(suffix=".psd", delete=False) as tmp:
            out_path = tmp.name

        os.remove(out_path)
        try:
            file.write(out_path)
            reread = psapi.LayeredFile.read(out_path)
            reread_layer = _find_text_layer_by_name(reread, "CharacterStylePrimary")
            self.assertIsNotNone(reread_layer)

            self.assertEqual(reread_layer.style_normal_sheet_index(), normal_sheet_index_before)
            self.assertEqual(reread_layer.style_normal_font(), normal_font_after)
            self.assertAlmostEqual(reread_layer.style_normal_font_size(), normal_font_size_after, places=3)
            self.assertAlmostEqual(reread_layer.style_normal_leading(), normal_leading_after, places=3)
            self.assertEqual(reread_layer.style_normal_auto_leading(), normal_auto_leading_after)
            self.assertEqual(reread_layer.style_normal_kerning(), normal_kerning_after)
            self.assertEqual(reread_layer.style_normal_faux_bold(), normal_faux_bold_after)
            self.assertEqual(reread_layer.style_normal_faux_italic(), normal_faux_italic_after)
            self.assertAlmostEqual(reread_layer.style_normal_horizontal_scale(), normal_horizontal_scale_after, places=3)
            self.assertAlmostEqual(reread_layer.style_normal_vertical_scale(), normal_vertical_scale_after, places=3)
            self.assertEqual(reread_layer.style_normal_tracking(), normal_tracking_after)
            self.assertEqual(reread_layer.style_normal_auto_kerning(), normal_auto_kerning_after)
            self.assertAlmostEqual(reread_layer.style_normal_baseline_shift(), normal_baseline_shift_after, places=3)
            self.assertEqual(reread_layer.style_normal_font_caps(), normal_font_caps_after)
            self.assertEqual(reread_layer.style_normal_font_baseline(), normal_font_baseline_after)
            self.assertEqual(reread_layer.style_normal_no_break(), normal_no_break_after)
            self.assertEqual(reread_layer.style_normal_language(), normal_language_after)
            self.assertEqual(reread_layer.style_normal_character_direction(), normal_character_direction_after)
            self.assertEqual(reread_layer.style_normal_baseline_direction(), normal_baseline_direction_after)
            self.assertAlmostEqual(reread_layer.style_normal_tsume(), normal_tsume_after, places=3)
            self.assertEqual(reread_layer.style_normal_kashida(), normal_kashida_after)
            self.assertEqual(reread_layer.style_normal_diacritic_pos(), normal_diacritic_pos_after)
            self.assertEqual(reread_layer.style_normal_ligatures(), normal_ligatures_after)
            self.assertEqual(reread_layer.style_normal_dligatures(), normal_dligatures_after)
            self.assertEqual(reread_layer.style_normal_underline(), normal_underline_after)
            self.assertEqual(reread_layer.style_normal_strikethrough(), normal_strikethrough_after)
            self.assertEqual(reread_layer.style_normal_stroke_flag(), normal_stroke_flag_after)
            self.assertEqual(reread_layer.style_normal_fill_flag(), normal_fill_flag_after)
            self.assertEqual(reread_layer.style_normal_fill_first(), normal_fill_first_after)
            self.assertAlmostEqual(reread_layer.style_normal_outline_width(), normal_outline_width_after, places=3)
            reread_fill = reread_layer.style_normal_fill_color()
            self.assertIsNotNone(reread_fill)
            self.assertEqual(len(reread_fill), 4)
            self.assertAlmostEqual(reread_fill[0], normal_fill_color_after[0], places=3)
            self.assertAlmostEqual(reread_fill[1], normal_fill_color_after[1], places=3)
            self.assertAlmostEqual(reread_fill[2], normal_fill_color_after[2], places=3)
            self.assertAlmostEqual(reread_fill[3], normal_fill_color_after[3], places=3)
            reread_stroke = reread_layer.style_normal_stroke_color()
            self.assertIsNotNone(reread_stroke)
            self.assertEqual(len(reread_stroke), 4)
            self.assertAlmostEqual(reread_stroke[0], normal_stroke_color_after[0], places=3)
            self.assertAlmostEqual(reread_stroke[1], normal_stroke_color_after[1], places=3)
            self.assertAlmostEqual(reread_stroke[2], normal_stroke_color_after[2], places=3)
            self.assertAlmostEqual(reread_stroke[3], normal_stroke_color_after[3], places=3)

            # Normal style-sheet mutation should not rewrite explicit run values.
            self.assertAlmostEqual(reread_layer.style_run_font_size(0), run_font_size_before, places=3)
        finally:
            if os.path.exists(out_path):
                os.remove(out_path)

    @unittest.skipUnless(os.path.exists(_PARAGRAPH_FIXTURE_PATH), "Paragraph text layer fixture is missing")
    def test_paragraph_run_properties_roundtrip(self):
        file = psapi.LayeredFile.read(self.paragraph_fixture_path)
        layer = _find_text_layer_containing(file, "paragraph text for fixture coverage")
        self.assertIsNotNone(layer)

        self.assertGreaterEqual(layer.paragraph_run_count, 1)
        before = layer.paragraph_run_justification(0)
        first_line_before = layer.paragraph_run_first_line_indent(0)
        start_before = layer.paragraph_run_start_indent(0)
        end_before = layer.paragraph_run_end_indent(0)
        space_before_before = layer.paragraph_run_space_before(0)
        space_after_before = layer.paragraph_run_space_after(0)
        auto_hyphenate_before = layer.paragraph_run_auto_hyphenate(0)
        hyphenated_word_size_before = layer.paragraph_run_hyphenated_word_size(0)
        pre_hyphen_before = layer.paragraph_run_pre_hyphen(0)
        post_hyphen_before = layer.paragraph_run_post_hyphen(0)
        consecutive_hyphens_before = layer.paragraph_run_consecutive_hyphens(0)
        zone_before = layer.paragraph_run_zone(0)
        word_spacing_before = layer.paragraph_run_word_spacing(0)
        letter_spacing_before = layer.paragraph_run_letter_spacing(0)
        glyph_spacing_before = layer.paragraph_run_glyph_spacing(0)
        auto_leading_before = layer.paragraph_run_auto_leading(0)
        leading_type_before = layer.paragraph_run_leading_type(0)
        hanging_before = layer.paragraph_run_hanging(0)
        burasagari_before = layer.paragraph_run_burasagari(0)
        kinsoku_order_before = layer.paragraph_run_kinsoku_order(0)
        every_line_composer_before = layer.paragraph_run_every_line_composer(0)
        self.assertIsNotNone(before)
        self.assertIsNotNone(first_line_before)
        self.assertIsNotNone(start_before)
        self.assertIsNotNone(end_before)
        self.assertIsNotNone(space_before_before)
        self.assertIsNotNone(space_after_before)
        self.assertIsNotNone(auto_hyphenate_before)
        self.assertIsNotNone(hyphenated_word_size_before)
        self.assertIsNotNone(pre_hyphen_before)
        self.assertIsNotNone(post_hyphen_before)
        self.assertIsNotNone(consecutive_hyphens_before)
        self.assertIsNotNone(zone_before)
        self.assertIsNotNone(word_spacing_before)
        self.assertIsNotNone(letter_spacing_before)
        self.assertIsNotNone(glyph_spacing_before)
        self.assertIsNotNone(auto_leading_before)
        self.assertIsNotNone(leading_type_before)
        self.assertIsNotNone(hanging_before)
        self.assertIsNotNone(burasagari_before)
        self.assertIsNotNone(kinsoku_order_before)
        self.assertIsNotNone(every_line_composer_before)
        self.assertEqual(len(word_spacing_before), 3)
        self.assertEqual(len(letter_spacing_before), 3)
        self.assertEqual(len(glyph_spacing_before), 3)

        after = psapi.enum.Justification.Right if before == psapi.enum.Justification.Left else psapi.enum.Justification.Left
        first_line_after = first_line_before + 12.5
        start_after = start_before + 4.25
        end_after = end_before + 2.75
        space_before_after = space_before_before + 3.0
        space_after_after = space_after_before + 6.0
        auto_hyphenate_after = not auto_hyphenate_before
        hyphenated_word_size_after = hyphenated_word_size_before + 1
        pre_hyphen_after = pre_hyphen_before + 1
        post_hyphen_after = post_hyphen_before + 1
        consecutive_hyphens_after = consecutive_hyphens_before + 1
        zone_after = zone_before + 5.5
        word_spacing_after = [word_spacing_before[0] + 0.05, word_spacing_before[1] + 0.05, word_spacing_before[2] + 0.05]
        letter_spacing_after = [letter_spacing_before[0] + 1.0, letter_spacing_before[1] + 1.0, letter_spacing_before[2] + 1.0]
        glyph_spacing_after = [glyph_spacing_before[0] + 0.1, glyph_spacing_before[1] + 0.1, glyph_spacing_before[2] + 0.1]
        auto_leading_after = auto_leading_before + 0.2
        leading_type_after = psapi.enum.LeadingType.TopToTop if leading_type_before == psapi.enum.LeadingType.BottomToBottom else psapi.enum.LeadingType.BottomToBottom
        hanging_after = not hanging_before
        burasagari_after = not burasagari_before
        kinsoku_order_after = psapi.enum.KinsokuOrder.PushOutFirst if kinsoku_order_before == psapi.enum.KinsokuOrder.PushInFirst else psapi.enum.KinsokuOrder.PushInFirst
        every_line_composer_after = not every_line_composer_before

        self.assertTrue(layer.set_paragraph_run_justification(0, after))
        self.assertTrue(layer.set_paragraph_run_first_line_indent(0, first_line_after))
        self.assertTrue(layer.set_paragraph_run_start_indent(0, start_after))
        self.assertTrue(layer.set_paragraph_run_end_indent(0, end_after))
        self.assertTrue(layer.set_paragraph_run_space_before(0, space_before_after))
        self.assertTrue(layer.set_paragraph_run_space_after(0, space_after_after))
        self.assertTrue(layer.set_paragraph_run_auto_hyphenate(0, auto_hyphenate_after))
        self.assertTrue(layer.set_paragraph_run_hyphenated_word_size(0, hyphenated_word_size_after))
        self.assertTrue(layer.set_paragraph_run_pre_hyphen(0, pre_hyphen_after))
        self.assertTrue(layer.set_paragraph_run_post_hyphen(0, post_hyphen_after))
        self.assertTrue(layer.set_paragraph_run_consecutive_hyphens(0, consecutive_hyphens_after))
        self.assertTrue(layer.set_paragraph_run_zone(0, zone_after))
        self.assertTrue(layer.set_paragraph_run_word_spacing(0, word_spacing_after))
        self.assertTrue(layer.set_paragraph_run_letter_spacing(0, letter_spacing_after))
        self.assertTrue(layer.set_paragraph_run_glyph_spacing(0, glyph_spacing_after))
        self.assertTrue(layer.set_paragraph_run_auto_leading(0, auto_leading_after))
        self.assertTrue(layer.set_paragraph_run_leading_type(0, leading_type_after))
        self.assertTrue(layer.set_paragraph_run_hanging(0, hanging_after))
        self.assertTrue(layer.set_paragraph_run_burasagari(0, burasagari_after))
        self.assertTrue(layer.set_paragraph_run_kinsoku_order(0, kinsoku_order_after))
        self.assertTrue(layer.set_paragraph_run_every_line_composer(0, every_line_composer_after))
        self.assertFalse(layer.set_paragraph_run_justification(200, after))
        self.assertFalse(layer.set_paragraph_run_first_line_indent(200, first_line_after))
        self.assertFalse(layer.set_paragraph_run_start_indent(200, start_after))
        self.assertFalse(layer.set_paragraph_run_end_indent(200, end_after))
        self.assertFalse(layer.set_paragraph_run_space_before(200, space_before_after))
        self.assertFalse(layer.set_paragraph_run_space_after(200, space_after_after))
        self.assertFalse(layer.set_paragraph_run_auto_hyphenate(200, auto_hyphenate_after))
        self.assertFalse(layer.set_paragraph_run_hyphenated_word_size(200, hyphenated_word_size_after))
        self.assertFalse(layer.set_paragraph_run_pre_hyphen(200, pre_hyphen_after))
        self.assertFalse(layer.set_paragraph_run_post_hyphen(200, post_hyphen_after))
        self.assertFalse(layer.set_paragraph_run_consecutive_hyphens(200, consecutive_hyphens_after))
        self.assertFalse(layer.set_paragraph_run_zone(200, zone_after))
        self.assertFalse(layer.set_paragraph_run_word_spacing(200, word_spacing_after))
        self.assertFalse(layer.set_paragraph_run_letter_spacing(200, letter_spacing_after))
        self.assertFalse(layer.set_paragraph_run_glyph_spacing(200, glyph_spacing_after))
        self.assertFalse(layer.set_paragraph_run_auto_leading(200, auto_leading_after))
        self.assertFalse(layer.set_paragraph_run_leading_type(200, leading_type_after))
        self.assertFalse(layer.set_paragraph_run_hanging(200, hanging_after))
        self.assertFalse(layer.set_paragraph_run_burasagari(200, burasagari_after))
        self.assertFalse(layer.set_paragraph_run_kinsoku_order(200, kinsoku_order_after))
        self.assertFalse(layer.set_paragraph_run_every_line_composer(200, every_line_composer_after))
        self.assertFalse(layer.set_paragraph_run_word_spacing(0, []))
        self.assertFalse(layer.set_paragraph_run_word_spacing(0, [1.0, float("inf"), 2.0]))

        with tempfile.NamedTemporaryFile(suffix=".psd", delete=False) as tmp:
            out_path = tmp.name

        os.remove(out_path)
        try:
            file.write(out_path)
            reread = psapi.LayeredFile.read(out_path)
            reread_layer = _find_text_layer_containing(reread, "paragraph text for fixture coverage")
            self.assertIsNotNone(reread_layer)

            self.assertEqual(reread_layer.paragraph_run_justification(0), after)
            self.assertAlmostEqual(reread_layer.paragraph_run_first_line_indent(0), first_line_after, places=3)
            self.assertAlmostEqual(reread_layer.paragraph_run_start_indent(0), start_after, places=3)
            self.assertAlmostEqual(reread_layer.paragraph_run_end_indent(0), end_after, places=3)
            self.assertAlmostEqual(reread_layer.paragraph_run_space_before(0), space_before_after, places=3)
            self.assertAlmostEqual(reread_layer.paragraph_run_space_after(0), space_after_after, places=3)
            self.assertEqual(reread_layer.paragraph_run_auto_hyphenate(0), auto_hyphenate_after)
            self.assertEqual(reread_layer.paragraph_run_hyphenated_word_size(0), hyphenated_word_size_after)
            self.assertEqual(reread_layer.paragraph_run_pre_hyphen(0), pre_hyphen_after)
            self.assertEqual(reread_layer.paragraph_run_post_hyphen(0), post_hyphen_after)
            self.assertEqual(reread_layer.paragraph_run_consecutive_hyphens(0), consecutive_hyphens_after)
            self.assertAlmostEqual(reread_layer.paragraph_run_zone(0), zone_after, places=3)
            self.assertEqual(len(reread_layer.paragraph_run_word_spacing(0)), 3)
            self.assertEqual(len(reread_layer.paragraph_run_letter_spacing(0)), 3)
            self.assertEqual(len(reread_layer.paragraph_run_glyph_spacing(0)), 3)
            self.assertAlmostEqual(reread_layer.paragraph_run_word_spacing(0)[0], word_spacing_after[0], places=3)
            self.assertAlmostEqual(reread_layer.paragraph_run_word_spacing(0)[1], word_spacing_after[1], places=3)
            self.assertAlmostEqual(reread_layer.paragraph_run_word_spacing(0)[2], word_spacing_after[2], places=3)
            self.assertAlmostEqual(reread_layer.paragraph_run_letter_spacing(0)[0], letter_spacing_after[0], places=3)
            self.assertAlmostEqual(reread_layer.paragraph_run_letter_spacing(0)[1], letter_spacing_after[1], places=3)
            self.assertAlmostEqual(reread_layer.paragraph_run_letter_spacing(0)[2], letter_spacing_after[2], places=3)
            self.assertAlmostEqual(reread_layer.paragraph_run_glyph_spacing(0)[0], glyph_spacing_after[0], places=3)
            self.assertAlmostEqual(reread_layer.paragraph_run_glyph_spacing(0)[1], glyph_spacing_after[1], places=3)
            self.assertAlmostEqual(reread_layer.paragraph_run_glyph_spacing(0)[2], glyph_spacing_after[2], places=3)
            self.assertAlmostEqual(reread_layer.paragraph_run_auto_leading(0), auto_leading_after, places=3)
            self.assertEqual(reread_layer.paragraph_run_leading_type(0), leading_type_after)
            self.assertEqual(reread_layer.paragraph_run_hanging(0), hanging_after)
            self.assertEqual(reread_layer.paragraph_run_burasagari(0), burasagari_after)
            self.assertEqual(reread_layer.paragraph_run_kinsoku_order(0), kinsoku_order_after)
            self.assertEqual(reread_layer.paragraph_run_every_line_composer(0), every_line_composer_after)
            self.assertIsNone(reread_layer.paragraph_run_justification(200))
            self.assertIsNone(reread_layer.paragraph_run_first_line_indent(200))
            self.assertIsNone(reread_layer.paragraph_run_start_indent(200))
            self.assertIsNone(reread_layer.paragraph_run_end_indent(200))
            self.assertIsNone(reread_layer.paragraph_run_space_before(200))
            self.assertIsNone(reread_layer.paragraph_run_space_after(200))
            self.assertIsNone(reread_layer.paragraph_run_auto_hyphenate(200))
            self.assertIsNone(reread_layer.paragraph_run_hyphenated_word_size(200))
            self.assertIsNone(reread_layer.paragraph_run_pre_hyphen(200))
            self.assertIsNone(reread_layer.paragraph_run_post_hyphen(200))
            self.assertIsNone(reread_layer.paragraph_run_consecutive_hyphens(200))
            self.assertIsNone(reread_layer.paragraph_run_zone(200))
            self.assertIsNone(reread_layer.paragraph_run_word_spacing(200))
            self.assertIsNone(reread_layer.paragraph_run_letter_spacing(200))
            self.assertIsNone(reread_layer.paragraph_run_glyph_spacing(200))
            self.assertIsNone(reread_layer.paragraph_run_auto_leading(200))
            self.assertIsNone(reread_layer.paragraph_run_leading_type(200))
            self.assertIsNone(reread_layer.paragraph_run_hanging(200))
            self.assertIsNone(reread_layer.paragraph_run_burasagari(200))
            self.assertIsNone(reread_layer.paragraph_run_kinsoku_order(200))
            self.assertIsNone(reread_layer.paragraph_run_every_line_composer(200))
        finally:
            if os.path.exists(out_path):
                os.remove(out_path)

    @unittest.skipUnless(os.path.exists(_PARAGRAPH_FIXTURE_PATH), "Paragraph text layer fixture is missing")
    def test_paragraph_normal_sheet_roundtrip(self):
        file = psapi.LayeredFile.read(self.paragraph_fixture_path)
        layer = _find_text_layer_containing(file, "paragraph text for fixture coverage")
        self.assertIsNotNone(layer)

        self.assertGreaterEqual(layer.paragraph_sheet_count, 1)
        normal_sheet_index_before = layer.paragraph_normal_sheet_index()
        normal_justification_before = layer.paragraph_normal_justification()
        normal_first_line_before = layer.paragraph_normal_first_line_indent()
        normal_start_before = layer.paragraph_normal_start_indent()
        normal_end_before = layer.paragraph_normal_end_indent()
        normal_space_before_before = layer.paragraph_normal_space_before()
        normal_space_after_before = layer.paragraph_normal_space_after()
        normal_auto_hyphenate_before = layer.paragraph_normal_auto_hyphenate()
        normal_hyphenated_word_size_before = layer.paragraph_normal_hyphenated_word_size()
        normal_pre_hyphen_before = layer.paragraph_normal_pre_hyphen()
        normal_post_hyphen_before = layer.paragraph_normal_post_hyphen()
        normal_consecutive_hyphens_before = layer.paragraph_normal_consecutive_hyphens()
        normal_zone_before = layer.paragraph_normal_zone()
        normal_word_spacing_before = layer.paragraph_normal_word_spacing()
        normal_letter_spacing_before = layer.paragraph_normal_letter_spacing()
        normal_glyph_spacing_before = layer.paragraph_normal_glyph_spacing()
        normal_auto_leading_before = layer.paragraph_normal_auto_leading()
        normal_leading_type_before = layer.paragraph_normal_leading_type()
        normal_hanging_before = layer.paragraph_normal_hanging()
        normal_burasagari_before = layer.paragraph_normal_burasagari()
        normal_kinsoku_order_before = layer.paragraph_normal_kinsoku_order()
        normal_every_line_composer_before = layer.paragraph_normal_every_line_composer()
        run_justification_before = layer.paragraph_run_justification(0)

        self.assertIsNotNone(normal_sheet_index_before)
        self.assertIsNotNone(normal_justification_before)
        self.assertIsNotNone(normal_first_line_before)
        self.assertIsNotNone(normal_start_before)
        self.assertIsNotNone(normal_end_before)
        self.assertIsNotNone(normal_space_before_before)
        self.assertIsNotNone(normal_space_after_before)
        self.assertIsNotNone(normal_auto_hyphenate_before)
        self.assertIsNotNone(normal_hyphenated_word_size_before)
        self.assertIsNotNone(normal_pre_hyphen_before)
        self.assertIsNotNone(normal_post_hyphen_before)
        self.assertIsNotNone(normal_consecutive_hyphens_before)
        self.assertIsNotNone(normal_zone_before)
        self.assertIsNotNone(normal_word_spacing_before)
        self.assertIsNotNone(normal_letter_spacing_before)
        self.assertIsNotNone(normal_glyph_spacing_before)
        self.assertIsNotNone(normal_auto_leading_before)
        self.assertIsNotNone(normal_leading_type_before)
        self.assertIsNotNone(normal_hanging_before)
        self.assertIsNotNone(normal_burasagari_before)
        self.assertIsNotNone(normal_kinsoku_order_before)
        self.assertIsNotNone(normal_every_line_composer_before)
        self.assertEqual(len(normal_word_spacing_before), 3)
        self.assertEqual(len(normal_letter_spacing_before), 3)
        self.assertEqual(len(normal_glyph_spacing_before), 3)
        self.assertIsNotNone(run_justification_before)

        normal_justification_after = psapi.enum.Justification.Center if normal_justification_before == psapi.enum.Justification.Left else psapi.enum.Justification.Left
        normal_first_line_after = normal_first_line_before + 8.5
        normal_start_after = normal_start_before + 2.25
        normal_end_after = normal_end_before + 1.75
        normal_space_before_after = normal_space_before_before + 1.0
        normal_space_after_after = normal_space_after_before + 2.0
        normal_auto_hyphenate_after = not normal_auto_hyphenate_before
        normal_hyphenated_word_size_after = normal_hyphenated_word_size_before + 1
        normal_pre_hyphen_after = normal_pre_hyphen_before + 1
        normal_post_hyphen_after = normal_post_hyphen_before + 1
        normal_consecutive_hyphens_after = normal_consecutive_hyphens_before + 1
        normal_zone_after = normal_zone_before + 3.5
        normal_word_spacing_after = [
            normal_word_spacing_before[0] + 0.02,
            normal_word_spacing_before[1] + 0.02,
            normal_word_spacing_before[2] + 0.02,
        ]
        normal_letter_spacing_after = [
            normal_letter_spacing_before[0] + 0.5,
            normal_letter_spacing_before[1] + 0.5,
            normal_letter_spacing_before[2] + 0.5,
        ]
        normal_glyph_spacing_after = [
            normal_glyph_spacing_before[0] + 0.02,
            normal_glyph_spacing_before[1] + 0.02,
            normal_glyph_spacing_before[2] + 0.02,
        ]
        normal_auto_leading_after = normal_auto_leading_before + 0.1
        normal_leading_type_after = psapi.enum.LeadingType.TopToTop if normal_leading_type_before == psapi.enum.LeadingType.BottomToBottom else psapi.enum.LeadingType.BottomToBottom
        normal_hanging_after = not normal_hanging_before
        normal_burasagari_after = not normal_burasagari_before
        normal_kinsoku_order_after = psapi.enum.KinsokuOrder.PushOutFirst if normal_kinsoku_order_before == psapi.enum.KinsokuOrder.PushInFirst else psapi.enum.KinsokuOrder.PushInFirst
        normal_every_line_composer_after = not normal_every_line_composer_before

        self.assertTrue(layer.set_paragraph_normal_sheet_index(normal_sheet_index_before))
        self.assertTrue(layer.set_paragraph_normal_justification(normal_justification_after))
        self.assertTrue(layer.set_paragraph_normal_first_line_indent(normal_first_line_after))
        self.assertTrue(layer.set_paragraph_normal_start_indent(normal_start_after))
        self.assertTrue(layer.set_paragraph_normal_end_indent(normal_end_after))
        self.assertTrue(layer.set_paragraph_normal_space_before(normal_space_before_after))
        self.assertTrue(layer.set_paragraph_normal_space_after(normal_space_after_after))
        self.assertTrue(layer.set_paragraph_normal_auto_hyphenate(normal_auto_hyphenate_after))
        self.assertTrue(layer.set_paragraph_normal_hyphenated_word_size(normal_hyphenated_word_size_after))
        self.assertTrue(layer.set_paragraph_normal_pre_hyphen(normal_pre_hyphen_after))
        self.assertTrue(layer.set_paragraph_normal_post_hyphen(normal_post_hyphen_after))
        self.assertTrue(layer.set_paragraph_normal_consecutive_hyphens(normal_consecutive_hyphens_after))
        self.assertTrue(layer.set_paragraph_normal_zone(normal_zone_after))
        self.assertTrue(layer.set_paragraph_normal_word_spacing(normal_word_spacing_after))
        self.assertTrue(layer.set_paragraph_normal_letter_spacing(normal_letter_spacing_after))
        self.assertTrue(layer.set_paragraph_normal_glyph_spacing(normal_glyph_spacing_after))
        self.assertTrue(layer.set_paragraph_normal_auto_leading(normal_auto_leading_after))
        self.assertTrue(layer.set_paragraph_normal_leading_type(normal_leading_type_after))
        self.assertTrue(layer.set_paragraph_normal_hanging(normal_hanging_after))
        self.assertTrue(layer.set_paragraph_normal_burasagari(normal_burasagari_after))
        self.assertTrue(layer.set_paragraph_normal_kinsoku_order(normal_kinsoku_order_after))
        self.assertTrue(layer.set_paragraph_normal_every_line_composer(normal_every_line_composer_after))
        self.assertFalse(layer.set_paragraph_normal_sheet_index(200))
        self.assertFalse(layer.set_paragraph_normal_sheet_index(-1))
        self.assertFalse(layer.set_paragraph_normal_space_before(float("inf")))
        self.assertFalse(layer.set_paragraph_normal_word_spacing([]))
        self.assertFalse(layer.set_paragraph_normal_word_spacing([1.0, float("inf"), 2.0]))
        self.assertFalse(layer.set_paragraph_normal_letter_spacing([]))
        self.assertFalse(layer.set_paragraph_normal_glyph_spacing([]))

        with tempfile.NamedTemporaryFile(suffix=".psd", delete=False) as tmp:
            out_path = tmp.name

        os.remove(out_path)
        try:
            file.write(out_path)
            reread = psapi.LayeredFile.read(out_path)
            reread_layer = _find_text_layer_containing(reread, "paragraph text for fixture coverage")
            self.assertIsNotNone(reread_layer)

            self.assertEqual(reread_layer.paragraph_normal_sheet_index(), normal_sheet_index_before)
            self.assertEqual(reread_layer.paragraph_normal_justification(), normal_justification_after)
            self.assertAlmostEqual(reread_layer.paragraph_normal_first_line_indent(), normal_first_line_after, places=3)
            self.assertAlmostEqual(reread_layer.paragraph_normal_start_indent(), normal_start_after, places=3)
            self.assertAlmostEqual(reread_layer.paragraph_normal_end_indent(), normal_end_after, places=3)
            self.assertAlmostEqual(reread_layer.paragraph_normal_space_before(), normal_space_before_after, places=3)
            self.assertAlmostEqual(reread_layer.paragraph_normal_space_after(), normal_space_after_after, places=3)
            self.assertEqual(reread_layer.paragraph_normal_auto_hyphenate(), normal_auto_hyphenate_after)
            self.assertEqual(reread_layer.paragraph_normal_hyphenated_word_size(), normal_hyphenated_word_size_after)
            self.assertEqual(reread_layer.paragraph_normal_pre_hyphen(), normal_pre_hyphen_after)
            self.assertEqual(reread_layer.paragraph_normal_post_hyphen(), normal_post_hyphen_after)
            self.assertEqual(reread_layer.paragraph_normal_consecutive_hyphens(), normal_consecutive_hyphens_after)
            self.assertAlmostEqual(reread_layer.paragraph_normal_zone(), normal_zone_after, places=3)
            reread_word_spacing = reread_layer.paragraph_normal_word_spacing()
            self.assertIsNotNone(reread_word_spacing)
            self.assertEqual(len(reread_word_spacing), 3)
            self.assertAlmostEqual(reread_word_spacing[0], normal_word_spacing_after[0], places=3)
            self.assertAlmostEqual(reread_word_spacing[1], normal_word_spacing_after[1], places=3)
            self.assertAlmostEqual(reread_word_spacing[2], normal_word_spacing_after[2], places=3)
            reread_letter_spacing = reread_layer.paragraph_normal_letter_spacing()
            self.assertIsNotNone(reread_letter_spacing)
            self.assertEqual(len(reread_letter_spacing), 3)
            self.assertAlmostEqual(reread_letter_spacing[0], normal_letter_spacing_after[0], places=3)
            self.assertAlmostEqual(reread_letter_spacing[1], normal_letter_spacing_after[1], places=3)
            self.assertAlmostEqual(reread_letter_spacing[2], normal_letter_spacing_after[2], places=3)
            reread_glyph_spacing = reread_layer.paragraph_normal_glyph_spacing()
            self.assertIsNotNone(reread_glyph_spacing)
            self.assertEqual(len(reread_glyph_spacing), 3)
            self.assertAlmostEqual(reread_glyph_spacing[0], normal_glyph_spacing_after[0], places=3)
            self.assertAlmostEqual(reread_glyph_spacing[1], normal_glyph_spacing_after[1], places=3)
            self.assertAlmostEqual(reread_glyph_spacing[2], normal_glyph_spacing_after[2], places=3)
            self.assertAlmostEqual(reread_layer.paragraph_normal_auto_leading(), normal_auto_leading_after, places=3)
            self.assertEqual(reread_layer.paragraph_normal_leading_type(), normal_leading_type_after)
            self.assertEqual(reread_layer.paragraph_normal_hanging(), normal_hanging_after)
            self.assertEqual(reread_layer.paragraph_normal_burasagari(), normal_burasagari_after)
            self.assertEqual(reread_layer.paragraph_normal_kinsoku_order(), normal_kinsoku_order_after)
            self.assertEqual(reread_layer.paragraph_normal_every_line_composer(), normal_every_line_composer_after)

            # Normal paragraph sheet mutation should not rewrite explicit run values.
            self.assertEqual(reread_layer.paragraph_run_justification(0), run_justification_before)
        finally:
            if os.path.exists(out_path):
                os.remove(out_path)

