"""Shared fixture paths and helper functions for TextLayer tests."""

import os
import tempfile

import photoshopapi as psapi


# ── Fixture paths ────────────────────────────────────────────────────────

def _fixture(*parts):
    return os.path.abspath(
        os.path.join(os.path.dirname(__file__), "..", "..", "..", "PhotoshopTest", "documents", "TextLayers", *parts)
    )


FIXTURE_PATH = _fixture("TextLayers_Basic.psd")
STYLE_RUNS_FIXTURE_PATH = _fixture("TextLayers_StyleRuns.psd")
PARAGRAPH_FIXTURE_PATH = _fixture("TextLayers_Paragraph.psd")
CHARACTER_STYLES_FIXTURE_PATH = _fixture("TextLayers_CharacterStyles.psd")
VERTICAL_FIXTURE_PATH = _fixture("TextLayers_Vertical.psd")
FONTFALLBACK_FIXTURE_PATH = _fixture("TextLayers_FontFallback.psd")
WARP_FIXTURE_PATH = _fixture("TextLayers_Warp.psd")
TRANSFORM_FIXTURE_PATH = _fixture("TextLayers_Transform.psd")


# ── Helper utilities ─────────────────────────────────────────────────────

def is_text_layer(layer) -> bool:
    return isinstance(layer, (psapi.TextLayer_8bit, psapi.TextLayer_16bit, psapi.TextLayer_32bit))


def find_text_layer(file, expected_text=None):
    for layer in file.flat_layers:
        if not is_text_layer(layer):
            continue
        if expected_text is None or layer.text == expected_text:
            return layer
    return None


def find_text_layer_containing(file, needle):
    for layer in file.flat_layers:
        if not is_text_layer(layer):
            continue
        if layer.text is not None and needle in layer.text:
            return layer
    return None


def find_text_layer_by_name(file, expected_name):
    for layer in file.flat_layers:
        if not is_text_layer(layer):
            continue
        if layer.name == expected_name:
            return layer
    return None


def roundtrip(file):
    """Write file to a temp PSD, re-read it, and delete the temp file."""
    with tempfile.NamedTemporaryFile(suffix=".psd", delete=False) as f:
        tmp_path = f.name
    try:
        file.write(tmp_path)
        return psapi.LayeredFile.read(tmp_path)
    finally:
        os.remove(tmp_path)
