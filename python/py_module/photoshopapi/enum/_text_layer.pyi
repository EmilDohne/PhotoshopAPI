# Pybind11 doesnt actually inherit from enum.Enum as seen here https://github.com/pybind/pybind11/issues/2332

class WritingDirection:
    Horizontal: int
    Vertical: int

class ShapeType:
    PointText: int
    BoxText: int

class WarpStyle:
    NoWarp: int
    Arc: int
    ArcLower: int
    ArcUpper: int
    Arch: int
    Bulge: int
    ShellLower: int
    ShellUpper: int
    Flag: int
    Wave: int
    Fish: int
    Rise: int
    FishEye: int
    Inflate: int
    Squeeze: int
    Twist: int
    Custom: int

class WarpRotation:
    Horizontal: int
    Vertical: int

class FontType:
    OpenType: int
    TrueType: int

class FontScript:
    Roman: int
    CJK: int

class FontCaps:
    Normal: int
    SmallCaps: int
    AllCaps: int

class FontBaseline:
    Normal: int
    Superscript: int
    Subscript: int

class CharacterDirection:
    Default: int
    LeftToRight: int
    RightToLeft: int

class BaselineDirection:
    Default: int
    Vertical: int
    CrossStream: int

class DiacriticPosition:
    OpenType: int
    Loose: int
    Medium: int
    Tight: int

class Justification:
    Left: int
    Right: int
    Center: int
    JustifyLastLeft: int
    JustifyLastRight: int
    JustifyLastCenter: int
    JustifyAll: int

class LeadingType:
    BottomToBottom: int
    TopToTop: int

class KinsokuOrder:
    PushInFirst: int
    PushOutFirst: int

class AntiAliasMethod:
    NoAntiAlias: int
    Crisp: int
    Strong: int
    Smooth: int
    Sharp: int
