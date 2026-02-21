#pragma once

#include "LayeredFile/LayerTypes/TextLayer/TextLayerEnum.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


// ---------------------------------------------------------------------------
//  TextLayer enum declarations (call once, NOT per bit-depth)
// ---------------------------------------------------------------------------
inline void declare_textlayer_enums(py::module& m)
{
    py::enum_<TextLayerEnum::WritingDirection>(m, "WritingDirection",
        "Writing direction for a text layer.")
        .value("Horizontal", TextLayerEnum::WritingDirection::Horizontal)
        .value("Vertical",   TextLayerEnum::WritingDirection::Vertical);

    py::enum_<TextLayerEnum::ShapeType>(m, "ShapeType",
        "Whether the text frame is point text or box (area) text.")
        .value("PointText", TextLayerEnum::ShapeType::PointText)
        .value("BoxText",   TextLayerEnum::ShapeType::BoxText);

    py::enum_<TextLayerEnum::WarpStyle>(m, "WarpStyle",
        "Warp style applied to a text layer.")
        .value("NoWarp",     TextLayerEnum::WarpStyle::NoWarp)
        .value("Arc",        TextLayerEnum::WarpStyle::Arc)
        .value("ArcLower",   TextLayerEnum::WarpStyle::ArcLower)
        .value("ArcUpper",   TextLayerEnum::WarpStyle::ArcUpper)
        .value("Arch",       TextLayerEnum::WarpStyle::Arch)
        .value("Bulge",      TextLayerEnum::WarpStyle::Bulge)
        .value("ShellLower", TextLayerEnum::WarpStyle::ShellLower)
        .value("ShellUpper", TextLayerEnum::WarpStyle::ShellUpper)
        .value("Flag",       TextLayerEnum::WarpStyle::Flag)
        .value("Wave",       TextLayerEnum::WarpStyle::Wave)
        .value("Fish",       TextLayerEnum::WarpStyle::Fish)
        .value("Rise",       TextLayerEnum::WarpStyle::Rise)
        .value("FishEye",    TextLayerEnum::WarpStyle::FishEye)
        .value("Inflate",    TextLayerEnum::WarpStyle::Inflate)
        .value("Squeeze",    TextLayerEnum::WarpStyle::Squeeze)
        .value("Twist",      TextLayerEnum::WarpStyle::Twist)
        .value("Custom",     TextLayerEnum::WarpStyle::Custom);

    py::enum_<TextLayerEnum::WarpRotation>(m, "WarpRotation",
        "Warp rotation axis.")
        .value("Horizontal", TextLayerEnum::WarpRotation::Horizontal)
        .value("Vertical",   TextLayerEnum::WarpRotation::Vertical);

    py::enum_<TextLayerEnum::FontType>(m, "FontType",
        "Font technology identifier stored in the FontSet.")
        .value("OpenType", TextLayerEnum::FontType::OpenType)
        .value("TrueType", TextLayerEnum::FontType::TrueType);

    py::enum_<TextLayerEnum::FontScript>(m, "FontScript",
        "Script classification for a font entry.")
        .value("Roman", TextLayerEnum::FontScript::Roman)
        .value("CJK",   TextLayerEnum::FontScript::CJK);

    py::enum_<TextLayerEnum::FontCaps>(m, "FontCaps",
        "Capitalization mode for a character style.")
        .value("Normal",   TextLayerEnum::FontCaps::Normal)
        .value("SmallCaps", TextLayerEnum::FontCaps::SmallCaps)
        .value("AllCaps",  TextLayerEnum::FontCaps::AllCaps);

    py::enum_<TextLayerEnum::FontBaseline>(m, "FontBaseline",
        "Baseline position for a character style.")
        .value("Normal",      TextLayerEnum::FontBaseline::Normal)
        .value("Superscript", TextLayerEnum::FontBaseline::Superscript)
        .value("Subscript",   TextLayerEnum::FontBaseline::Subscript);

    py::enum_<TextLayerEnum::CharacterDirection>(m, "CharacterDirection",
        "Per-character direction override.")
        .value("Default",      TextLayerEnum::CharacterDirection::Default)
        .value("LeftToRight",  TextLayerEnum::CharacterDirection::LeftToRight)
        .value("RightToLeft",  TextLayerEnum::CharacterDirection::RightToLeft);

    py::enum_<TextLayerEnum::BaselineDirection>(m, "BaselineDirection",
        "Baseline direction for CJK or advanced typography.")
        .value("Default",      TextLayerEnum::BaselineDirection::Default)
        .value("Vertical",     TextLayerEnum::BaselineDirection::Vertical)
        .value("CrossStream",  TextLayerEnum::BaselineDirection::CrossStream);

    py::enum_<TextLayerEnum::DiacriticPosition>(m, "DiacriticPosition",
        "Diacritic positioning mode.")
        .value("OpenType", TextLayerEnum::DiacriticPosition::OpenType)
        .value("Loose",    TextLayerEnum::DiacriticPosition::Loose)
        .value("Medium",   TextLayerEnum::DiacriticPosition::Medium)
        .value("Tight",    TextLayerEnum::DiacriticPosition::Tight);

    py::enum_<TextLayerEnum::Justification>(m, "Justification",
        "Paragraph justification / alignment.")
        .value("Left",             TextLayerEnum::Justification::Left)
        .value("Right",            TextLayerEnum::Justification::Right)
        .value("Center",           TextLayerEnum::Justification::Center)
        .value("JustifyLastLeft",  TextLayerEnum::Justification::JustifyLastLeft)
        .value("JustifyLastRight", TextLayerEnum::Justification::JustifyLastRight)
        .value("JustifyLastCenter",TextLayerEnum::Justification::JustifyLastCenter)
        .value("JustifyAll",       TextLayerEnum::Justification::JustifyAll);

    py::enum_<TextLayerEnum::LeadingType>(m, "LeadingType",
        "How leading (line spacing) is measured.")
        .value("BottomToBottom", TextLayerEnum::LeadingType::BottomToBottom)
        .value("TopToTop",       TextLayerEnum::LeadingType::TopToTop);

    py::enum_<TextLayerEnum::KinsokuOrder>(m, "KinsokuOrder",
        "Kinsoku processing order for CJK typography.")
        .value("PushInFirst",  TextLayerEnum::KinsokuOrder::PushInFirst)
        .value("PushOutFirst", TextLayerEnum::KinsokuOrder::PushOutFirst);

    py::enum_<TextLayerEnum::AntiAliasMethod>(m, "AntiAliasMethod",
        "Anti-aliasing method for text rendering. Corresponds to the Photoshop\n"
        "anti-aliasing options in the character panel / options bar.")
        .value("NoAntiAlias", TextLayerEnum::AntiAliasMethod::None)
        .value("Crisp",       TextLayerEnum::AntiAliasMethod::Crisp)
        .value("Strong",      TextLayerEnum::AntiAliasMethod::Strong)
        .value("Smooth",      TextLayerEnum::AntiAliasMethod::Smooth)
        .value("Sharp",       TextLayerEnum::AntiAliasMethod::Sharp);
}
