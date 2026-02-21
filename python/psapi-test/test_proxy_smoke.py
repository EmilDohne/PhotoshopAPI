"""
Smoke tests for the proxy accessor objects added to TextLayer.

These tests verify that:
  - style_run(i) returns a StyleRunProxy with working getters/setters
  - style_normal() returns a StyleNormalProxy with working getters/setters
  - paragraph_run(i) returns a ParagraphRunProxy with working getters/setters
  - paragraph_normal() returns a ParagraphNormalProxy with working getters/setters
  - font(i) returns a FontProxy with working getters
  - font_set() returns a FontSetProxy with working getters
"""

import sys
import os
import io
import unittest

# Allow running from repo root with PYTHONPATH=build-vs/python/Release
import photoshopapi as psapi

_FIXTURE_PATH = os.path.abspath(
    os.path.join(
        os.path.dirname(__file__),
        "..",
        "..",
        "PhotoshopTest",
        "documents",
        "TextLayers",
        "TextLayers_Basic.psd",
    )
)

_STYLE_RUNS_FIXTURE_PATH = os.path.abspath(
    os.path.join(
        os.path.dirname(__file__),
        "..",
        "..",
        "PhotoshopTest",
        "documents",
        "TextLayers",
        "TextLayers_StyleRuns.psd",
    )
)


def _find_text_layer_by_name(file, name):
    for layer in file.layers:
        if layer.name == name and isinstance(layer, (psapi.TextLayer_8bit, psapi.TextLayer_16bit, psapi.TextLayer_32bit)):
            return layer
    return None


def _roundtrip(file):
    buf = io.BytesIO()
    file.write(buf)
    buf.seek(0)
    return psapi.LayeredFile.read(buf)


class TestStyleRunProxy(unittest.TestCase):

    def setUp(self):
        self.file = psapi.LayeredFile.read(_FIXTURE_PATH)
        self.layer = _find_text_layer_by_name(self.file, "SimpleASCII")
        self.assertIsNotNone(self.layer)

    def test_style_run_proxy_returns_object(self):
        proxy = self.layer.style_run(0)
        self.assertIsNotNone(proxy)

    def test_style_run_font_size_getter(self):
        proxy = self.layer.style_run(0)
        fs = proxy.font_size
        # font_size is Optional[float], should be a number
        self.assertIsNotNone(fs)
        self.assertIsInstance(fs, float)

    def test_style_run_set_font_size_roundtrip(self):
        proxy = self.layer.style_run(0)
        self.assertTrue(proxy.set_font_size(42.0))
        # Verify via flat API too
        self.assertAlmostEqual(self.layer.style_run_font_size(0), 42.0)

    def test_style_run_faux_bold_getter(self):
        proxy = self.layer.style_run(0)
        fb = proxy.faux_bold
        # faux_bold is Optional[bool]
        self.assertIsNotNone(fb)

    def test_style_run_set_faux_bold(self):
        proxy = self.layer.style_run(0)
        self.assertTrue(proxy.set_faux_bold(True))
        self.assertTrue(self.layer.style_run_faux_bold(0))

    def test_style_run_set_font_by_name(self):
        proxy = self.layer.style_run(0)
        # Use the same font that's already there (should succeed)
        current_font_idx = proxy.font
        if current_font_idx is not None:
            name = self.layer.font_postscript_name(current_font_idx)
            if name is not None:
                self.assertTrue(proxy.set_font_by_name(name))


class TestStyleNormalProxy(unittest.TestCase):

    def setUp(self):
        self.file = psapi.LayeredFile.read(_FIXTURE_PATH)
        self.layer = _find_text_layer_by_name(self.file, "SimpleASCII")
        self.assertIsNotNone(self.layer)

    def test_style_normal_proxy_returns_object(self):
        proxy = self.layer.style_normal()
        self.assertIsNotNone(proxy)

    def test_style_normal_font_size_getter(self):
        proxy = self.layer.style_normal()
        fs = proxy.font_size
        self.assertIsNotNone(fs)
        self.assertIsInstance(fs, float)

    def test_style_normal_set_font_size_roundtrip(self):
        proxy = self.layer.style_normal()
        self.assertTrue(proxy.set_font_size(18.0))
        self.assertAlmostEqual(self.layer.style_normal_font_size(), 18.0)

    def test_style_normal_faux_italic_getter(self):
        proxy = self.layer.style_normal()
        fi = proxy.faux_italic
        self.assertIsNotNone(fi)

    def test_style_normal_set_faux_italic(self):
        proxy = self.layer.style_normal()
        self.assertTrue(proxy.set_faux_italic(True))
        self.assertTrue(self.layer.style_normal_faux_italic())


class TestParagraphRunProxy(unittest.TestCase):

    def setUp(self):
        self.file = psapi.LayeredFile.read(_FIXTURE_PATH)
        self.layer = _find_text_layer_by_name(self.file, "SimpleASCII")
        self.assertIsNotNone(self.layer)

    def test_paragraph_run_proxy_returns_object(self):
        proxy = self.layer.paragraph_run(0)
        self.assertIsNotNone(proxy)

    def test_paragraph_run_justification_getter(self):
        proxy = self.layer.paragraph_run(0)
        j = proxy.justification
        self.assertIsNotNone(j)

    def test_paragraph_run_set_justification(self):
        proxy = self.layer.paragraph_run(0)
        self.assertTrue(proxy.set_justification(psapi.enum.Justification.Left))
        self.assertEqual(self.layer.paragraph_run_justification(0), psapi.enum.Justification.Left)

    def test_paragraph_run_auto_hyphenate_getter(self):
        proxy = self.layer.paragraph_run(0)
        ah = proxy.auto_hyphenate
        self.assertIsNotNone(ah)


class TestParagraphNormalProxy(unittest.TestCase):

    def setUp(self):
        self.file = psapi.LayeredFile.read(_FIXTURE_PATH)
        self.layer = _find_text_layer_by_name(self.file, "SimpleASCII")
        self.assertIsNotNone(self.layer)

    def test_paragraph_normal_proxy_returns_object(self):
        proxy = self.layer.paragraph_normal()
        self.assertIsNotNone(proxy)

    def test_paragraph_normal_justification_getter(self):
        proxy = self.layer.paragraph_normal()
        j = proxy.justification
        self.assertIsNotNone(j)

    def test_paragraph_normal_set_justification(self):
        proxy = self.layer.paragraph_normal()
        self.assertTrue(proxy.set_justification(psapi.enum.Justification.Left))
        self.assertEqual(self.layer.paragraph_normal_justification(), psapi.enum.Justification.Left)


class TestFontProxy(unittest.TestCase):

    def setUp(self):
        self.file = psapi.LayeredFile.read(_FIXTURE_PATH)
        self.layer = _find_text_layer_by_name(self.file, "SimpleASCII")
        self.assertIsNotNone(self.layer)

    def test_font_proxy_returns_object(self):
        proxy = self.layer.font(0)
        self.assertIsNotNone(proxy)

    def test_font_postscript_name_getter(self):
        proxy = self.layer.font(0)
        name = proxy.postscript_name
        self.assertIsNotNone(name)
        self.assertIsInstance(name, str)

    def test_font_name_alias(self):
        proxy = self.layer.font(0)
        self.assertEqual(proxy.name, proxy.postscript_name)

    def test_font_is_sentinel(self):
        proxy = self.layer.font(0)
        # is_sentinel is bool
        self.assertIsInstance(proxy.is_sentinel, bool)


class TestFontSetProxy(unittest.TestCase):

    def setUp(self):
        self.file = psapi.LayeredFile.read(_FIXTURE_PATH)
        self.layer = _find_text_layer_by_name(self.file, "SimpleASCII")
        self.assertIsNotNone(self.layer)

    def test_font_set_proxy_returns_object(self):
        proxy = self.layer.font_set()
        self.assertIsNotNone(proxy)

    def test_font_set_count(self):
        proxy = self.layer.font_set()
        self.assertGreater(proxy.count, 0)

    def test_font_set_used_names(self):
        proxy = self.layer.font_set()
        names = proxy.used_names
        self.assertIsInstance(names, list)
        self.assertGreater(len(names), 0)

    def test_font_set_find_index(self):
        proxy = self.layer.font_set()
        names = proxy.used_names
        if names:
            idx = proxy.find_index(names[0])
            self.assertGreaterEqual(idx, 0)

    def test_font_set_used_indices(self):
        proxy = self.layer.font_set()
        indices = proxy.used_indices
        self.assertIsInstance(indices, list)
        self.assertGreater(len(indices), 0)


if __name__ == "__main__":
    unittest.main()
