import os
import unittest

import photoshopapi as psapi

from ._textlayer_helpers import (
    FIXTURE_PATH,
    find_text_layer,
    roundtrip,
)

_FIXTURE_PATH = FIXTURE_PATH
_find_text_layer = find_text_layer
_roundtrip = roundtrip


@unittest.skipUnless(os.path.exists(_FIXTURE_PATH), "Text layer fixture is missing")
class TestUnicodeEdgeCases(unittest.TestCase):
    """Step 8: CJK, RTL, emoji/surrogate-pair, combining-mark edge cases."""

    # ── CJK ─────────────────────────────────────────────────────────────

    def test_cjk_set_text_roundtrip(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)
        cjk = "\u4F60\u597D\u4E16\u754C"           # 你好世界
        self.assertTrue(layer.set_text(cjk))
        self.assertEqual(layer.text, cjk)
        reread = _roundtrip(file)
        rl = _find_text_layer(reread, cjk)
        self.assertIsNotNone(rl)

    def test_cjk_replace_text_partial(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)
        mixed = "AB\u4E2D\u6587CD"                  # AB中文CD
        self.assertTrue(layer.set_text(mixed))
        replacement = "\u65E5\u672C\u8A9E\u6587\u5B57"  # 日本語文字
        self.assertTrue(layer.replace_text("\u4E2D\u6587", replacement))
        expected = "AB\u65E5\u672C\u8A9E\u6587\u5B57CD"
        self.assertEqual(layer.text, expected)
        reread = _roundtrip(file)
        self.assertIsNotNone(_find_text_layer(reread, expected))

    def test_cjk_fullwidth_roundtrip(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)
        fw = "\uFF21\uFF22\uFF23"                   # ＡＢＣ
        self.assertTrue(layer.set_text(fw))
        self.assertEqual(layer.text, fw)
        reread = _roundtrip(file)
        self.assertIsNotNone(_find_text_layer(reread, fw))

    # ── RTL (Arabic / Hebrew) ───────────────────────────────────────────

    def test_arabic_set_text_roundtrip(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)
        arabic = "\u0645\u0631\u062D\u0628\u0627"   # مرحبا
        self.assertTrue(layer.set_text(arabic))
        self.assertEqual(layer.text, arabic)
        reread = _roundtrip(file)
        self.assertIsNotNone(_find_text_layer(reread, arabic))

    def test_hebrew_replace_roundtrip(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)
        hebrew = "\u05E9\u05DC\u05D5\u05DD"         # שלום
        self.assertTrue(layer.set_text(hebrew))
        self.assertTrue(layer.replace_text(hebrew, "\u05D1\u05D3\u05D9\u05E7\u05D4"))  # בדיקה
        self.assertEqual(layer.text, "\u05D1\u05D3\u05D9\u05E7\u05D4")
        reread = _roundtrip(file)
        self.assertIsNotNone(_find_text_layer(reread, "\u05D1\u05D3\u05D9\u05E7\u05D4"))

    def test_mixed_ltr_rtl_roundtrip(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)
        mixed = "Hello \u0645\u0631\u062D\u0628\u0627 World"
        self.assertTrue(layer.set_text(mixed))
        self.assertEqual(layer.text, mixed)
        reread = _roundtrip(file)
        self.assertIsNotNone(_find_text_layer(reread, mixed))

    # ── Emoji / surrogate pairs ──────────────────────────────────────────

    def test_emoji_surrogate_pairs_roundtrip(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)
        emoji = "\U0001F600\U0001F389\U0001F30D"    # 😀🎉🌍
        self.assertTrue(layer.set_text(emoji))
        self.assertEqual(layer.text, emoji)
        reread = _roundtrip(file)
        self.assertIsNotNone(_find_text_layer(reread, emoji))

    def test_mixed_ascii_emoji_replace(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)
        text = "A\U0001F600B\U0001F389C"
        self.assertTrue(layer.set_text(text))
        self.assertTrue(layer.replace_text("\U0001F600B\U0001F389", "\U0001F30D"))
        expected = "A\U0001F30DC"
        self.assertEqual(layer.text, expected)
        reread = _roundtrip(file)
        self.assertIsNotNone(_find_text_layer(reread, expected))

    def test_adjacent_surrogate_pairs_replace(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)
        text = "\U0001F600\U0001F389\U0001F30D"
        self.assertTrue(layer.set_text(text))
        self.assertTrue(layer.replace_text("\U0001F389", "X"))
        expected = "\U0001F600X\U0001F30D"
        self.assertEqual(layer.text, expected)
        reread = _roundtrip(file)
        self.assertIsNotNone(_find_text_layer(reread, expected))

    # ── Combining marks ──────────────────────────────────────────────────

    def test_precomposed_vs_decomposed_roundtrip(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)
        precomposed = "caf\u00E9"                    # café (precomposed)
        self.assertTrue(layer.set_text(precomposed))
        self.assertEqual(layer.text, precomposed)
        reread = _roundtrip(file)
        self.assertIsNotNone(_find_text_layer(reread, precomposed))

    def test_combining_diacritics_roundtrip(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)
        decomposed = "cafe\u0301"                    # café (decomposed)
        self.assertTrue(layer.set_text(decomposed))
        self.assertEqual(layer.text, decomposed)
        reread = _roundtrip(file)
        self.assertIsNotNone(_find_text_layer(reread, decomposed))

    def test_multiple_combining_marks(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)
        stacked = "a\u0301\u0303"
        self.assertTrue(layer.set_text(stacked))
        self.assertEqual(layer.text, stacked)
        reread = _roundtrip(file)
        self.assertIsNotNone(_find_text_layer(reread, stacked))

    # ── Korean / Thai ────────────────────────────────────────────────────

    def test_korean_hangul_roundtrip(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)
        hangul = "\uD55C\uAD6D\uC5B4"               # 한국어
        self.assertTrue(layer.set_text(hangul))
        self.assertEqual(layer.text, hangul)
        reread = _roundtrip(file)
        self.assertIsNotNone(_find_text_layer(reread, hangul))

    def test_thai_script_roundtrip(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)
        thai = "\u0E2A\u0E27\u0E31\u0E2A\u0E14\u0E35"  # สวัสดี
        self.assertTrue(layer.set_text(thai))
        self.assertEqual(layer.text, thai)
        reread = _roundtrip(file)
        self.assertIsNotNone(_find_text_layer(reread, thai))

    # ── Kitchen-sink mixed ───────────────────────────────────────────────

    def test_kitchen_sink_mixed_script(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)
        kitchen = "Hi\u4E16\u754C\u0645\u0631\u062D\U0001F600e\u0301"
        self.assertTrue(layer.set_text(kitchen))
        self.assertEqual(layer.text, kitchen)
        reread = _roundtrip(file)
        self.assertIsNotNone(_find_text_layer(reread, kitchen))

    def test_zero_width_space_roundtrip(self):
        file = psapi.LayeredFile.read(_FIXTURE_PATH)
        layer = _find_text_layer(file, "Hello 123")
        self.assertIsNotNone(layer)
        zws = "A\u200BB\u200BC"                      # Zero-width spaces
        self.assertTrue(layer.set_text(zws))
        self.assertEqual(layer.text, zws)
        reread = _roundtrip(file)
        self.assertIsNotNone(_find_text_layer(reread, zws))
