# Text Layer Fixtures

These fixtures are generated from Photoshop and used for text layer parsing and roundtripping tests.

To regenerate locally (requires Photoshop installed):

1. Open PowerShell in the repository root.
2. Run:

```powershell
$app = New-Object -ComObject Photoshop.Application
$app.DoJavaScriptFile((Resolve-Path .\scripts\photoshop_generate_textlayer_fixtures.jsx).Path)
```

The script writes:

- `TextLayers_Basic.psd`
- `TextLayers_Warp.psd`
- `TextLayers_Paragraph.psd`
- `TextLayers_CharacterStyles.psd`
- `TextLayers_StyleRuns.psd`
- `TextLayers_Vertical.psd`
- `TextLayers_Transform.psd`
- `TextLayers_FontFallback.psd`
- `manifest.txt`

Coverage snapshot:

- Normal text
- Multiline text
- Text warp
- Paragraph text box layout settings
- Character-level style properties (layer-wide)
- Mixed style runs in a single text layer (`textStyleRange`)
- Native vertical text orientation (`Direction.VERTICAL`)
- Transformed text layer geometry (rotate/scale)
- Deterministic unresolved missing-font token (post-save patch)
