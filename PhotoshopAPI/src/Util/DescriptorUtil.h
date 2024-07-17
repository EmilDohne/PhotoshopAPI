#include "Macros.h"

#include <vector>
#include <string>

PSAPI_NAMESPACE_BEGIN

namespace Descriptors
{
	namespace Impl
	{
		/// Despite the photoshop docs saying that 4-byte keys are always represented
		/// preceded by a length field that is simply 0 it actually only considers
		/// the following keys to have this specific designation so if the key is one of these
		/// the length field is 0. Otherwise it is 4.
		/// 
		/// These definitions are adapted from psd-tools
		inline const std::vector<std::string> knownFourByteKeys
		{
			// Class Definitions
			"Actn", // Action 
			"ASet", // ActionSet 
			"Adjs", // Adjustment 
			"AdjL", // AdjustmentLayer 
			"AbTl", // AirbrushTool 
			"AChl", // AlphaChannelOptions 
			"AntA", // AntiAliasedPICTAcquire 
			"capp", // Application 
			"cArw", // Arrowhead 
			"Asrt", // Assert 
			"AssP", // AssumedProfile 
			"BMPF", // BMPFormat 
			"BckL", // BackgroundLayer 
			"ebbl", // BevelEmboss 
			"BtmM", // BitmapMode 
			"Blnd", // BlendRange 
			"BlTl", // BlurTool 
			"BkCl", // BookColor 
			"BrgC", // BrightnessContrast 
			"Brsh", // Brush 
			"BrTl", // BurnInTool 
			"CchP", // CachePrefs 
			"CMYC", // CMYKColor 
			"CMYM", // CMYKColorMode 
			"CMYS", // CMYKSetup 
			"Clcl", // Calculation 
			"Chnl", // Channel 
			"ChMx", // ChannelMatrix 
			"ChnM", // ChannelMixer 
			"SDPX", // CineonFormat 
			"Clpo", // ClippingInfo 
			"ClpP", // ClippingPath 
			"ClTl", // CloneStampTool 
			"Clr ", // Color 
			"ClrB", // ColorBalance 
			"ClrC", // ColorCorrection 
			"Clrk", // ColorPickerPrefs 
			"ClSm", // ColorSampler 
			"Clrt", // ColorStop 
			"Cmnd", // Command 
			"Crvs", // Curves 
			"CrPt", // CurvePoint 
			"Cstl", // CustomPalette 
			"CrvA", // CurvesAdjustment 
			"CstP", // CustomPhosphors 
			"CstW", // CustomWhitePoint 
			"Dicm", // DicomFormat 
			"DspP", // DisplayPrefs 
			"Dcmn", // Document 
			"DdTl", // DodgeTool 
			"DrSh", // DropShadow 
			"DtnI", // DuotoneInk 
			"DtnM", // DuotoneMode 
			"EPSG", // EPSGenericFormat 
			"EPSC", // EPSPICTPreview 
			"EPST", // EPSTIFFPreview 
			"Elmn", // Element 
			"Elps", // Ellipse 
			"ErTl", // EraserTool 
			"Expr", // Export 
			"FlIn", // FileInfo 
			"FlSv", // FileSavePrefs 
			"FlsP", // FlashPixFormat 
			"FntD", // FontDesignAxes 
			"Fmt ", // Format 
			"FrFX", // FrameFX 
			"FxSc", // Contour 
			"GnrP", // GeneralPrefs 
			"GF89", // GIF89aExport 
			"GFFr", // GIFFormat 
			"gblA", // GlobalAngle 
			"Grdn", // Gradient 
			"Grdf", // GradientFill 
			"GdMp", // GradientMap 
			"GrTl", // GradientTool 
			"GrSt", // GraySetup 
			"Grsc", // Grayscale 
			"Grys", // GrayscaleMode 
			"Gd  ", // Guide 
			"GdPr", // GuidesPrefs 
			"HlfS", // HalftoneScreen 
			"Hlfp", // HalftoneSpec 
			"HSBC", // HSBColor 
			"HSBM", // HSBColorMode 
			"HBTl", // HistoryBrushTool 
			"CHsP", // HistoryPrefs 
			"HstS", // HistoryState 
			"HStA", // HueSatAdjustment 
			"Hst2", // HueSatAdjustmentV2 
			"HStr", // HueSaturation 
			"IFFF", // IFFFormat 
			"IlsP", // IllustratorPathsExport 
			"ImgP", // ImagePoint 
			"Impr", // Import 
			"IndC", // IndexedColorMode 
			"InkT", // InkTransfer 
			"IrGl", // InnerGlow 
			"IrSh", // InnerShadow 
			"IClr", // InterfaceColor 
			"Invr", // Invert 
			"JPEG", // JPEGFormat 
			"LbCl", // LabColor 
			"LbCM", // LabColorMode 
			"Lyr ", // Layer 
			"Lefx", // LayerEffects 
			"lfxv", // LayerFXVisible 
			"Lvls", // Levels 
			"LvlA", // LevelsAdjustment 
			"LghS", // LightSource 
			"Ln  ", // Line 
			"McPn", // MacPaintFormat 
			"MgEr", // MagicEraserTool 
			"Mgcp", // MagicPoint 
			"Msk ", // Mask 
			"Mn  ", // MenuItem 
			"Md  ", // Mode 
			"MltC", // MultichannelMode 
			"TxLy", // ObsoleteTextLayer 
			"null", // Null 
			"Ofst", // Offset 
			"Opac", // Opacity 
			"OrGl", // OuterGlow 
			"PDFG", // PDFGenericFormat 
			"PICF", // PICTFileFormat 
			"PICR", // PICTResourceFormat 
			"PNGF", // PNGFormat 
			"PgSt", // PageSetup 
			"PbTl", // PaintbrushTool 
			"Path", // Path 
			"PaCm", // PathComponent 
			"Pthp", // PathPoint 
			"PttR", // Pattern 
			"PaTl", // PatternStampTool 
			"PcTl", // PencilTool 
			"Pht2", // Photoshop20Format 
			"Pht3", // Photoshop35Format 
			"PhD2", // PhotoshopDCS2Format 
			"PhD1", // PhotoshopDCSFormat 
			"PhtE", // PhotoshopEPSFormat 
			"PhtP", // PhotoshopPDFFormat 
			"Pxel", // Pixel 
			"PxlP", // PixelPaintFormat 
			"PlgP", // PluginPrefs 
			"Pnt ", // Point 
			"Pnt1", // Point16 
			"Plgn", // Polygon 
			"Pstr", // Posterize 
			"GnrP", // Preferences 
			"PrfS", // ProfileSetup 
			"Prpr", // Property 
			"Rang", // Range 
			"Rct1", // Rect16 
			"RGBC", // RGBColor 
			"RGBM", // RGBColorMode 
			"RGBt", // RGBSetup 
			"Rw  ", // RawFormat 
			"Rctn", // Rectangle 
			"SrTl", // SaturationTool 
			"Sctx", // ScitexCTFormat 
			"csel", // Selection 
			"SlcC", // SelectiveColor 
			"ShpC", // ShapingCurve 
			"ShTl", // SharpenTool 
			"Sngc", // SingleColumn 
			"Sngr", // SingleRow 
			"SETl", // BackgroundEraserTool 
			"SoFi", // SolidFill 
			"ABTl", // ArtHistoryBrushTool 
			"SmTl", // SmudgeTool 
			"SnpS", // Snapshot 
			"SCch", // SpotColorChannel 
			"StyC", // Style 
			"Sbpl", // SubPath 
			"TIFF", // TIFFFormat 
			"TrgF", // TargaFormat 
			"TxLr", // TextLayer 
			"TxtS", // TextStyle 
			"Txtt", // TextStyleRange 
			"Thrs", // Threshold 
			"Tool", // Tool 
			"Trfp", // TransferSpec 
			"DtnP", // TransferPoint 
			"TrnP", // TransparencyPrefs 
			"TrnS", // TransparencyStop 
			"UntP", // UnitsPrefs 
			"UnsC", // UnspecifiedColor 
			"Vrsn", // Version 
			"Wdbv", // WebdavPrefs 
			"XYYC", // XYYColor 
			"ChFX", // ChromeFX 
			"BakL", // BackLight 
			"FilF", // FillFlash 
			"ColC", // ColorCast 
			"EXRf", // EXRf 
			// Enums
			"Add ", // Add 
			"amHi", // AmountHigh 
			"amLo", // AmountLow 
			"amMd", // AmountMedium 
			"Anno", // AntiAliasNone 
			"AnLo", // AntiAliasLow 
			"AnMd", // AntiAliasMedium 
			"AnHi", // AntiAliasHigh 
			"AnCr", // AntiAliasCrisp 
			"AnSt", // AntiAliasStrong 
			"AnSm", // AntiAliasSmooth 
			"AppR", // AppleRGB 
			"ASCI", // ASCII 
			"AskW", // AskWhenOpening 
			"Bcbc", // Bicubic 
			"Bnry", // Binary 
			"MntS", // MonitorSetup 
			"16Bt", // _16BitsPerPixel 
			"OnBt", // _1BitPerPixel 
			"2Bts", // _2BitsPerPixel 
			"32Bt", // _32BitsPerPixel 
			"4Bts", // _4BitsPerPixel 
			"5000", // _5000 
			"5500", // _5500 
			"6500", // _6500 
			"72Cl", // _72Color 
			"72Gr", // _72Gray 
			"7500", // _7500 
			"EghB", // _8BitsPerPixel 
			"9300", // _9300 
			"A   ", // A 
			"AClr", // AbsColorimetric 
			"AdBt", // ADSBottoms 
			"AdCH", // ADSCentersH 
			"AdCV", // ADSCentersV 
			"AdHr", // ADSHorizontal 
			"AdLf", // ADSLefts 
			"AdRg", // ADSRights 
			"AdTp", // ADSTops 
			"AdVr", // ADSVertical 
			"AbAp", // AboutApp 
			"Absl", // Absolute 
			"ActP", // ActualPixels 
			"Adpt", // Adaptive 
			"AdjO", // AdjustmentOptions 
			"Arbs", // AirbrushEraser 
			"Al  ", // All 
			"Amga", // Amiga 
			"Angl", // Angle 
			"Any ", // Any 
			"AplI", // ApplyImage 
			"ArnC", // AroundCenter 
			"Arng", // Arrange 
			"Ask ", // Ask 
			"B   ", // B 
			"Back", // Back 
			"Bckg", // Background 
			"BckC", // BackgroundColor 
			"Bckw", // Backward 
			"Bhnd", // Behind 
			"Bst ", // Best 
			"Dthb", // Better 
			"Blnr", // Bilinear 
			"BD1 ", // BitDepth1 
			"BD16", // BitDepth16 
			"BD24", // BitDepth24 
			"BD32", // BitDepth32 
			"BD4 ", // BitDepth4 
			"BD8 ", // BitDepth8 
			"1565", // BitDepthA1R5G5B5 
			"x565", // BitDepthR5G6B5 
			"x444", // BitDepthX4R4G4B4 
			"4444", // BitDepthA4R4G4B4 
			"x888", // BitDepthX8R8G8B8 
			"Btmp", // Bitmap 
			"Blck", // Black 
			"BanW", // BlackAndWhite 
			"BlcB", // BlackBody 
			"Blks", // Blacks 
			"Blk ", // BlockEraser 
			"Blst", // Blast 
			"Blks", // Blocks 
			"Bl  ", // Blue 
			"Bls ", // Blues 
			"Bttm", // Bottom 
			"BrDR", // BrushDarkRough 
			"BrsA", // BrushesAppend 
			"BrsD", // BrushesDefine 
			"Brsf", // BrushesDelete 
			"Brsd", // BrushesLoad 
			"BrsN", // BrushesNew 
			"BrsO", // BrushesOptions 
			"BrsR", // BrushesReset 
			"Brsv", // BrushesSave 
			"BrsL", // BrushLightRough 
			"BrSm", // BrushSimple 
			"BrsS", // BrushSize 
			"BrSp", // BrushSparkle 
			"BrbW", // BrushWideBlurry 
			"BrsW", // BrushWideSharp 
			"Bltn", // Builtin 
			"BrnH", // BurnInH 
			"BrnM", // BurnInM 
			"BrnS", // BurnInS 
			"BtnM", // ButtonMode 
			"CRGB", // CIERGB 
			"Wide", // WidePhosphors 
			"WRGB", // WideGamutRGB 
			"CMYK", // CMYK 
			"CMSF", // CMYK64 
			"ECMY", // CMYKColor 
			"Clcl", // Calculations 
			"Cscd", // Cascade 
			"Cntr", // Center 
			"SrcC", // CenterGlow 
			"CtrF", // CenteredFrame 
			"ChnO", // ChannelOptions 
			"ChnP", // ChannelsPaletteOptions 
			"ChcN", // CheckerboardNone 
			"ChcS", // CheckerboardSmall 
			"ChcM", // CheckerboardMedium 
			"ChcL", // CheckerboardLarge 
			"Clar", // Clear 
			"ClrG", // ClearGuides 
			"Clpb", // Clipboard 
			"ClpP", // ClippingPath 
			"ClsA", // CloseAll 
			"CrsD", // CoarseDots 
			"Clr ", // Color 
			"CBrn", // ColorBurn 
			"CDdg", // ColorDodge 
			"ClMt", // ColorMatch 
			"ClNs", // ColorNoise 
			"Clrm", // Colorimetric 
			"Cmps", // Composite 
			"CnvC", // ConvertToCMYK 
			"CnvG", // ConvertToGray 
			"CnvL", // ConvertToLab 
			"CnvR", // ConvertToRGB 
			"CrtD", // CreateDuplicate 
			"CrtI", // CreateInterpolation 
			"Crs ", // Cross 
			"CrrL", // CurrentLayer 
			"Cst ", // Custom 
			"Cstm", // CustomPattern 
			"CstS", // CustomStops 
			"Cyn ", // Cyan 
			"Cyns", // Cyans 
			"Drk ", // Dark 
			"Drkn", // Darken 
			"DrkO", // DarkenOnly 
			"DshL", // DashedLines 
			"Dstt", // Desaturate 
			"Dmnd", // Diamond 
			"Dfrn", // Difference 
			"Dfsn", // Diffusion 
			"DfnD", // DiffusionDither 
			"DspC", // DisplayCursorsPreferences 
			"Dslv", // Dissolve 
			"Dstr", // Distort 
			"DdgH", // DodgeH 
			"DdgM", // DodgeM 
			"DdgS", // DodgeS 
			"Dts ", // Dots 
			"Drft", // Draft 
			"Dtn ", // Duotone 
			"EBT ", // EBUITU 
			"SrcE", // EdgeGlow 
			"ElmE", // EliminateEvenFields 
			"ElmO", // EliminateOddFields 
			"Elps", // Ellipse 
			"Embs", // Emboss 
			"Exct", // Exact 
			"Xclu", // Exclusion 
			"FxJP", // FPXCompressLossyJPEG 
			"FxNo", // FPXCompressNone 
			"Dthf", // Faster 
			"Fle ", // File 
			"FlIn", // FileInfo 
			"FlBc", // FillBack 
			"FlFr", // FillFore 
			"FlIn", // FillInverse 
			"FlSm", // FillSame 
			"FnDt", // FineDots 
			"Frst", // First 
			"FrId", // FirstIdle 
			"FtOn", // FitOnScreen 
			"FrgC", // ForegroundColor 
			"Frwr", // Forward 
			"FrTr", // FreeTransform 
			"Frnt", // Front 
			"FllD", // FullDocument 
			"FlSz", // FullSize 
			"Gsn ", // GaussianDistribution 
			"GFCT", // GIFColorFileColorTable 
			"GFCF", // GIFColorFileColors 
			"GFMS", // GIFColorFileMicrosoftPalette 
			"GFPA", // GIFPaletteAdaptive 
			"GFPE", // GIFPaletteExact 
			"GFPO", // GIFPaletteOther 
			"GFPS", // GIFPaletteSystem 
			"GFCI", // GIFRequiredColorSpaceIndexed 
			"GFRG", // GIFRequiredColorSpaceRGB 
			"GFIN", // GIFRowOrderInterlaced 
			"GFNI", // GIFRowOrderNormal 
			"GnrP", // GeneralPreferences 
			"Gd  ", // Good 
			"GrFl", // GradientFill 
			"GrnC", // GrainClumped 
			"GrCn", // GrainContrasty 
			"GrnE", // GrainEnlarged 
			"GrnH", // GrainHorizontal 
			"GrnR", // GrainRegular 
			"GrSf", // GrainSoft 
			"GrSp", // GrainSpeckle 
			"GrSr", // GrainSprinkles 
			"GrSt", // GrainStippled 
			"GrnV", // GrainVertical 
			"GrnD", // GrainyDots 
			"Grp ", // Graphics 
			"Gry ", // Gray 
			"GryX", // Gray16 
			"Gr18", // Gray18 
			"Gr22", // Gray22 
			"Gr50", // Gray50 
			"Gryc", // GrayScale 
			"Grys", // Grayscale 
			"Grn ", // Green 
			"Grns", // Greens 
			"GudG", // GuidesGridPreferences 
			"HDTV", // HDTV 
			"HSBl", // HSBColor 
			"HSLC", // HSLColor 
			"HlfF", // HalftoneFile 
			"HlfS", // HalftoneScreen 
			"HrdL", // HardLight 
			"Hvy ", // Heavy 
			"HdAl", // HideAll 
			"HdSl", // HideSelection 
			"High", // High 
			"Hgh ", // HighQuality 
			"Hghl", // Highlights 
			"Hstg", // Histogram 
			"Hsty", // History 
			"HstO", // HistoryPaletteOptions 
			"HstP", // HistoryPreferences 
			"Hrzn", // Horizontal 
			"HrzO", // HorizontalOnly 
			"H   ", // Hue 
			"IBMP", // IBMPC 
			"ICC ", // ICC 
			"Icn ", // Icon 
			"IdVM", // IdleVM 
			"Ignr", // Ignore 
			"Img ", // Image 
			"ImgP", // ImageCachePreferences 
			"Indl", // IndexedColor 
			"InfP", // InfoPaletteOptions 
			"InfT", // InfoPaletteToggleSamplers 
			"InrB", // InnerBevel 
			"InsF", // InsetFrame 
			"Insd", // Inside 
			"JPEG", // JPEG 
			"JstA", // JustifyAll 
			"JstF", // JustifyFull 
			"KPro", // KeepProfile 
			"KybP", // KeyboardPreferences 
			"Lab ", // Lab 
			"LbCF", // Lab48 
			"LbCl", // LabColor 
			"Lrg ", // Large 
			"Lst ", // Last 
			"LstF", // LastFilter 
			"LyrO", // LayerOptions 
			"LyrP", // LayersPaletteOptions 
			"Left", // Left 
			"Lft ", // Left_PLUGIN 
			"LvlB", // LevelBased 
			"Lgt ", // Light 
			"LgtB", // LightBlue 
			"LDBt", // LightDirBottom 
			"LDBL", // LightDirBottomLeft 
			"LDBR", // LightDirBottomRight 
			"LDLf", // LightDirLeft 
			"LDRg", // LightDirRight 
			"LDTp", // LightDirTop 
			"LDTL", // LightDirTopLeft 
			"LDTR", // LightDirTopRight 
			"LgtG", // LightGray 
			"LghD", // LightDirectional 
			"LghO", // LightenOnly 
			"LghO", // LightOmni 
			"LPBt", // LightPosBottom 
			"LPBL", // LightPosBottomLeft 
			"LPBr", // LightPosBottomRight 
			"LPLf", // LightPosLeft 
			"LPRg", // LightPosRight 
			"LPTp", // LightPosTop 
			"LPTL", // LightPosTopLeft 
			"LPTR", // LightPosTopRight 
			"LgtR", // LightRed 
			"LghS", // LightSpot 
			"Lghn", // Lighten 
			"Lght", // Lightness 
			"Ln  ", // Line 
			"Lns ", // Lines 
			"Lnr ", // Linear 
			"Lnkd", // Linked 
			"LngL", // LongLines 
			"LngS", // LongStrokes 
			"Low ", // Low 
			"Lwr ", // Lower 
			"Lw  ", // LowQuality 
			"Lmns", // Luminosity 
			"Maya", // Maya 
			"McTh", // MacThumbnail 
			"Mcnt", // Macintosh 
			"McnS", // MacintoshSystem 
			"Mgnt", // Magenta 
			"Mgnt", // Magentas 
			"Msk ", // Mask 
			"MskA", // MaskedAreas 
			"MAdp", // MasterAdaptive 
			"MPer", // MasterPerceptual 
			"MSel", // MasterSelective 
			"Mxmm", // Maximum 
			"Mxm ", // MaximumQuality 
			"Mdim", // Medium 
			"MdmB", // MediumBlue 
			"Mdm ", // MediumQuality 
			"MdmD", // MediumDots 
			"MdmL", // MediumLines 
			"MdmS", // MediumStrokes 
			"MmrP", // MemoryPreferences 
			"MrgC", // MergeChannels 
			"Mrgd", // Merged 
			"Mrg2", // MergedLayers 
			"MrgL", // MergedLayersOld 
			"Mddl", // Middle 
			"Mdtn", // Midtones 
			"MdGr", // ModeGray 
			"MdRG", // ModeRGB 
			"Moni", // Monitor 
			"Mntn", // Monotone 
			"72CM", // Multi72Color 
			"72GM", // Multi72Gray 
			"Mlth", // Multichannel 
			"NCmM", // MultiNoCompositePS 
			"Mltp", // Multiply 
			"NvgP", // NavigatorPaletteOptions 
			"Nrst", // NearestNeighbor 
			"NsGr", // NetscapeGray 
			"Ntrl", // Neutrals 
			"NwVw", // NewView 
			"Nxt ", // Next 
			"Nkn ", // Nikon 
			"Nkn1", // Nikon105 
			"N   ", // No 
			"NCmp", // NoCompositePS 
			"None", // _None 
			"Nrml", // Normal 
			"NrmP", // NormalPath 
			"NTSC", // NTSC 
			"null", // Null 
			"OS2 ", // OS2 
			"Off ", // Off 
			"On  ", // On 
			"OpAs", // OpenAs 
			"Orng", // Orange 
			"OtFr", // OutFromCenter 
			"OtOf", // OutOfGamut 
			"OtrB", // OuterBevel 
			"Otsd", // Outside 
			"OutF", // OutsetFrame 
			"Ovrl", // Overlay 
			"Pntb", // PaintbrushEraser 
			"Pncl", // PencilEraser 
			"P22B", // P22EBU 
			"PGAd", // PNGFilterAdaptive 
			"PGAv", // PNGFilterAverage 
			"PGNo", // PNGFilterNone 
			"PGPt", // PNGFilterPaeth 
			"PGSb", // PNGFilterSub 
			"PGUp", // PNGFilterUp 
			"PGIA", // PNGInterlaceAdam7 
			"PGIN", // PNGInterlaceNone 
			"PgPC", // PagePosCentered 
			"PgTL", // PagePosTopLeft 
			"PgSt", // PageSetup 
			"PlSc", // PalSecam 
			"PnVs", // PanaVision 
			"PthP", // PathsPaletteOptions 
			"Ptrn", // Pattern 
			"PtnD", // PatternDither 
			"Perc", // Perceptual 
			"Prsp", // Perspective 
			"Phtk", // PhotoshopPicker 
			"PckC", // PickCMYK 
			"PckG", // PickGray 
			"PckH", // PickHSB 
			"PckL", // PickLab 
			"PckO", // PickOptions 
			"PckR", // PickRGB 
			"PlEb", // PillowEmboss 
			"PxS1", // PixelPaintSize1 
			"PxS2", // PixelPaintSize2 
			"PxS3", // PixelPaintSize3 
			"PxS4", // PixelPaintSize4 
			"Plce", // Place 
			"PbkO", // PlaybackOptions 
			"PlgP", // PluginPicker 
			"PlgS", // PluginsScratchDiskPreferences 
			"PlrR", // PolarToRect 
			"PndR", // PondRipples 
			"Prc ", // Precise 
			"PrBL", // PreciseMatte 
			"PrvO", // PreviewOff 
			"PrvC", // PreviewCMYK 
			"Prvy", // PreviewCyan 
			"PrvM", // PreviewMagenta 
			"PrvY", // PreviewYellow 
			"PrvB", // PreviewBlack 
			"PrvN", // PreviewCMY 
			"Prvs", // Previous 
			"Prim", // Primaries 
			"PrnS", // PrintSize 
			"PrnI", // PrintingInksSetup 
			"Prp ", // Purple 
			"Pyrm", // Pyramids 
			"Qcsa", // QCSAverage 
			"Qcs0", // QCSCorner0 
			"Qcs1", // QCSCorner1 
			"Qcs2", // QCSCorner2 
			"Qcs3", // QCSCorner3 
			"Qcsi", // QCSIndependent 
			"Qcs4", // QCSSide0 
			"Qcs5", // QCSSide1 
			"Qcs6", // QCSSide2 
			"Qcs7", // QCSSide3 
			"Qdtn", // Quadtone 
			"QurA", // QueryAlways 
			"Qurl", // QueryAsk 
			"QurN", // QueryNever 
			"Rpt ", // Repeat 
			"RGB ", // RGB 
			"RGBF", // RGB48 
			"RGBC", // RGBColor 
			"Rdl ", // Radial 
			"Rndm", // Random 
			"RctP", // RectToPolar 
			"Rd  ", // Red 
			"RdCm", // RedrawComplete 
			"Rds ", // Reds 
			"Rflc", // Reflected 
			"Rltv", // Relative 
			"RptE", // RepeatEdgePixels 
			"RvlA", // RevealAll 
			"RvlS", // RevealSelection 
			"Rvrt", // Revert 
			"Rght", // Right 
			"Rtte", // Rotate 
			"RtsP", // RotoscopingPreferences 
			"Rnd ", // Round 
			"RrCm", // RulerCm 
			"RrIn", // RulerInches 
			"RrPr", // RulerPercent 
			"RrPi", // RulerPicas 
			"RrPx", // RulerPixels 
			"RrPt", // RulerPoints 
			"SMPT", // AdobeRGB1998 
			"SMPC", // SMPTEC 
			"SRGB", // SRGB 
			"Smp3", // Sample3x3 
			"Smp5", // Sample5x5 
			"SmpP", // SamplePoint 
			"Str ", // Saturate 
			"Strt", // Saturation 
			"Sved", // Saved 
			"Svfw", // SaveForWeb 
			"SvnF", // SavingFilesPreferences 
			"Scl ", // Scale 
			"Scrn", // Screen 
			"ScrC", // ScreenCircle 
			"ScrD", // ScreenDot 
			"ScrL", // ScreenLine 
			"SlcA", // SelectedAreas 
			"Slct", // Selection 
			"Sele", // Selective 
			"SprS", // SeparationSetup 
			"SprT", // SeparationTables 
			"Shdw", // Shadows 
			"sp01", // ContourLinear 
			"sp02", // ContourGaussian 
			"sp03", // ContourSingle 
			"sp04", // ContourDouble 
			"sp05", // ContourTriple 
			"sp06", // ContourCustom 
			"ShrL", // ShortLines 
			"ShSt", // ShortStrokes 
			"72CS", // Single72Color 
			"72GS", // Single72Gray 
			"NCmS", // SingleNoCompositePS 
			"Skew", // Skew 
			"Slmt", // SlopeLimitMatte 
			"Sml ", // Small 
			"SBME", // SmartBlurModeEdgeOnly 
			"SBMN", // SmartBlurModeNormal 
			"SBMO", // SmartBlurModeOverlayEdge 
			"SBQH", // SmartBlurQualityHigh 
			"SBQL", // SmartBlurQualityLow 
			"SBQM", // SmartBlurQualityMedium 
			"Snps", // Snapshot 
			"SClr", // SolidColor 
			"SftL", // SoftLight 
			"SfBL", // SoftMatte 
			"Spct", // Spectrum 
			"Spn ", // Spin 
			"Spot", // SpotColor 
			"Sqr ", // Square 
			"Stgr", // Stagger 
			"In  ", // StampIn 
			"Out ", // StampOut 
			"Std ", // Standard 
			"StdA", // StdA 
			"StdB", // StdB 
			"StdC", // StdC 
			"StdE", // StdE 
			"StrF", // StretchToFit 
			"SDHz", // StrokeDirHorizontal 
			"SDLD", // StrokeDirLeftDiag 
			"SDRD", // StrokeDirRightDiag 
			"SDVt", // StrokeDirVertical 
			"SlsA", // StylesAppend 
			"Slsf", // StylesDelete 
			"Slsd", // StylesLoad 
			"SlsN", // StylesNew 
			"SlsR", // StylesReset 
			"Slsv", // StylesSave 
			"Sbtr", // Subtract 
			"SwtA", // SwatchesAppend 
			"Swtp", // SwatchesReplace 
			"SwtR", // SwatchesReset 
			"SwtS", // SwatchesSave 
			"SysP", // SystemPicker 
			"Tbl ", // Tables 
			"Trgt", // Target 
			"Trgp", // TargetPath 
			"TxBl", // TexTypeBlocks 
			"TxBr", // TexTypeBrick 
			"TxBu", // TexTypeBurlap 
			"TxCa", // TexTypeCanvas 
			"TxFr", // TexTypeFrosted 
			"TxSt", // TexTypeSandstone 
			"TxTL", // TexTypeTinyLens 
			"Thrh", // Threshold 
			"Thmb", // Thumbnail 
			"TIFF", // TIFF 
			"Tile", // Tile 
			"Tl  ", // Tile_PLUGIN 
			"TglA", // ToggleActionsPalette 
			"TgBP", // ToggleBlackPreview 
			"TglB", // ToggleBrushesPalette 
			"TglC", // ToggleCMYKPreview 
			"TgCM", // ToggleCMYPreview 
			"Tglh", // ToggleChannelsPalette 
			"Tglc", // ToggleColorPalette 
			"TgCP", // ToggleCyanPreview 
			"TglE", // ToggleEdges 
			"TglG", // ToggleGamutWarning 
			"TgGr", // ToggleGrid 
			"Tgld", // ToggleGuides 
			"TglH", // ToggleHistoryPalette 
			"TglI", // ToggleInfoPalette 
			"TglM", // ToggleLayerMask 
			"Tgly", // ToggleLayersPalette 
			"TglL", // ToggleLockGuides 
			"TgMP", // ToggleMagentaPreview 
			"TglN", // ToggleNavigatorPalette 
			"TglO", // ToggleOptionsPalette 
			"TglP", // TogglePaths 
			"Tglt", // TogglePathsPalette 
			"TrMp", // ToggleRGBMacPreview 
			"TrWp", // ToggleRGBWindowsPreview 
			"TrUp", // ToggleRGBUncompensatedPreview 
			"TglR", // ToggleRulers 
			"TgSn", // ToggleSnapToGrid 
			"TglS", // ToggleSnapToGuides 
			"Tgls", // ToggleStatusBar 
			"TgSl", // ToggleStylesPalette 
			"Tglw", // ToggleSwatchesPalette 
			"TglT", // ToggleToolsPalette 
			"TgYP", // ToggleYellowPreview 
			"TgDc", // ToggleDocumentPalette 
			"Top ", // Top 
			"Trsp", // Transparency 
			"TrnG", // TransparencyGamutPreferences 
			"Trns", // Transparent 
			"Trnt", // Trinitron 
			"Trtn", // Tritone 
			"UBtm", // UIBitmap 
			"UCMY", // UICMYK 
			"UDtn", // UIDuotone 
			"UGry", // UIGrayscale 
			"UInd", // UIIndexed 
			"ULab", // UILab 
			"UMlt", // UIMultichannel 
			"URGB", // UIRGB 
			"Und ", // Undo 
			"Unfm", // Uniform 
			"Unfr", // UniformDistribution 
			"UntR", // UnitsRulersPreferences 
			"Upr ", // Upper 
			"UsrS", // UserStop 
			"VMPr", // VMPreferences 
			"Vrtc", // Vertical 
			"VrtO", // VerticalOnly 
			"Vlt ", // Violet 
			"WvSn", // WaveSine 
			"WvSq", // WaveSquare 
			"WvTr", // WaveTriangle 
			"Web ", // Web 
			"Wht ", // White 
			"Whts", // Whites 
			"WnTh", // WinThumbnail 
			"Wnd ", // Wind 
			"Win ", // Windows 
			"WndS", // WindowsSystem 
			"Wrp ", // Wrap 
			"WrpA", // WrapAround 
			"WrkP", // WorkPath 
			"Yllw", // Yellow 
			"Ylw ", // YellowColor 
			"Ylws", // Yellows 
			"Ys  ", // Yes 
			"ZpEn", // Zip 
			"Zm  ", // Zoom 
			"ZmIn", // ZoomIn 
			"ZmOt", // ZoomOut 
			// Event
			"TdT ", // _3DTransform 
			"Avrg", // Average 
			"ASty", // ApplyStyle 
			"Asrt", // Assert 
			"AccE", // AccentedEdges 
			"Add ", // Add 
			"AdNs", // AddNoise 
			"AddT", // AddTo 
			"Algn", // Align 
			"All ", // All 
			"AngS", // AngledStrokes 
			"AppI", // ApplyImage 
			"BsRl", // BasRelief 
			"Btch", // Batch 
			"BtcF", // BatchFromDroplet 
			"Blr ", // Blur 
			"BlrM", // BlurMore 
			"Brdr", // Border 
			"BrgC", // Brightness 
			"CnvS", // CanvasSize 
			"ChlC", // ChalkCharcoal 
			"ChnM", // ChannelMixer 
			"Chrc", // Charcoal 
			"Chrm", // Chrome 
			"Cler", // Clear 
			"Cls ", // Close 
			"Clds", // Clouds 
			"ClrB", // ColorBalance 
			"ClrH", // ColorHalftone 
			"ClrR", // ColorRange 
			"ClrP", // ColoredPencil 
			"CntC", // ConteCrayon 
			"Cntc", // Contract 
			"CnvM", // ConvertMode 
			"copy", // Copy 
			"CpFX", // CopyEffects 
			"CpyM", // CopyMerged 
			"CpTL", // CopyToLayer 
			"Crql", // Craquelure 
			"CrtD", // CreateDroplet 
			"Crop", // Crop 
			"Crsh", // Crosshatch 
			"Crst", // Crystallize 
			"Crvs", // Curves 
			"Cstm", // Custom 
			"cut ", // Cut 
			"CtTL", // CutToLayer 
			"Ct  ", // Cutout 
			"DrkS", // DarkStrokes 
			"Dntr", // DeInterlace 
			"DfnP", // DefinePattern 
			"Dfrg", // Defringe 
			"Dlt ", // Delete 
			"Dstt", // Desaturate 
			"Dslc", // Deselect 
			"Dspc", // Despeckle 
			"DfrC", // DifferenceClouds 
			"Dfs ", // Diffuse 
			"DfsG", // DiffuseGlow 
			"dlfx", // DisableLayerFX 
			"Dspl", // Displace 
			"Dstr", // Distribute 
			"Draw", // Draw 
			"DryB", // DryBrush 
			"Dplc", // Duplicate 
			"DstS", // DustAndScratches 
			"Embs", // Emboss 
			"Eqlz", // Equalize 
			"Exch", // Exchange 
			"Expn", // Expand 
			"Expr", // Export 
			"Extr", // Extrude 
			"Fct ", // Facet 
			"Fade", // Fade 
			"Fthr", // Feather 
			"Fbrs", // Fibers 
			"Fl  ", // Fill 
			"FlmG", // FilmGrain 
			"Fltr", // Filter 
			"FndE", // FindEdges 
			"FltI", // FlattenImage 
			"Flip", // Flip 
			"Frgm", // Fragment 
			"Frsc", // Fresco 
			"GsnB", // GaussianBlur 
			"getd", // Get 
			"Gls ", // Glass 
			"GlwE", // GlowingEdges 
			"Grdn", // Gradient 
			"GrMp", // GradientMap 
			"Grn ", // Grain 
			"GraP", // GraphicPen 
			"GrpL", // Group 
			"Grow", // Grow 
			"HlfS", // HalftoneScreen 
			"Hd  ", // Hide 
			"HghP", // HighPass 
			"HsbP", // HSBHSL 
			"HStr", // HueSaturation 
			"ImgS", // ImageSize 
			"Impr", // Import 
			"InkO", // InkOutlines 
			"Intr", // Intersect 
			"IntW", // IntersectWith 
			"Invs", // Inverse 
			"Invr", // Invert 
			"LnsF", // LensFlare 
			"Lvls", // Levels 
			"LghE", // LightingEffects 
			"Lnk ", // Link 
			"Mk  ", // Make 
			"Mxm ", // Maximum 
			"Mdn ", // Median 
			"Mrg2", // MergeLayers 
			"MrgL", // MergeLayersOld 
			"MSpt", // MergeSpotChannel 
			"MrgV", // MergeVisible 
			"Mztn", // Mezzotint 
			"Mnm ", // Minimum 
			"Msc ", // Mosaic 
			"MscT", // Mosaic_PLUGIN 
			"MtnB", // MotionBlur 
			"move", // Move 
			"NTSC", // NTSCColors 
			"NGlw", // NeonGlow 
			"Nxt ", // Next 
			"NtPr", // NotePaper 
			"Ntfy", // Notify 
			"null", // Null 
			"OcnR", // OceanRipple 
			"Ofst", // Offset 
			"Opn ", // Open 
			"PntD", // PaintDaubs 
			"PltK", // PaletteKnife 
			"past", // Paste 
			"PaFX", // PasteEffects 
			"PstI", // PasteInto 
			"PstO", // PasteOutside 
			"Ptch", // Patchwork 
			"Phtc", // Photocopy 
			"Pnch", // Pinch 
			"Plc ", // Place 
			"Plst", // Plaster 
			"PlsW", // PlasticWrap 
			"Ply ", // Play 
			"Pntl", // Pointillize 
			"Plr ", // Polar 
			"PstE", // PosterEdges 
			"Pstr", // Posterize 
			"Prvs", // Previous 
			"Prnt", // Print 
			"PrfT", // ProfileToProfile 
			"Prge", // Purge 
			"quit", // Quit 
			"RdlB", // RadialBlur 
			"Rstr", // Rasterize 
			"RstT", // RasterizeTypeSheet 
			"RmvB", // RemoveBlackMatte 
			"RmvL", // RemoveLayerMask 
			"RmvW", // RemoveWhiteMatte 
			"Rnm ", // Rename 
			"RplC", // ReplaceColor 
			"Rset", // Reset 
			"Rtcl", // Reticulation 
			"Rvrt", // Revert 
			"Rple", // Ripple 
			"Rtte", // Rotate 
			"RghP", // RoughPastels 
			"save", // Save 
			"slct", // Select 
			"SlcC", // SelectiveColor 
			"setd", // Set 
			"ShrE", // SharpenEdges 
			"Shrp", // Sharpen 
			"ShrM", // SharpenMore 
			"Shr ", // Shear 
			"Shw ", // Show 
			"Smlr", // Similar 
			"SmrB", // SmartBlur 
			"Smth", // Smooth 
			"SmdS", // SmudgeStick 
			"Slrz", // Solarize 
			"Spt ", // Spatter 
			"Sphr", // Spherize 
			"SplC", // SplitChannels 
			"Spng", // Sponge 
			"SprS", // SprayedStrokes 
			"StnG", // StainedGlass 
			"Stmp", // Stamp 
			"Stop", // Stop 
			"Strk", // Stroke 
			"Sbtr", // Subtract 
			"SbtF", // SubtractFrom 
			"Smie", // Sumie 
			"TkMr", // TakeMergedSnapshot 
			"TkSn", // TakeSnapshot 
			"TxtF", // TextureFill 
			"Txtz", // Texturizer 
			"Thrs", // Threshold 
			"Tls ", // Tiles 
			"TrnE", // TornEdges 
			"TrcC", // TraceContour 
			"Trnf", // Transform 
			"Trap", // Trap 
			"Twrl", // Twirl 
			"Undr", // Underpainting 
			"undo", // Undo 
			"Ungr", // Ungroup 
			"Unlk", // Unlink 
			"UnsM", // UnsharpMask 
			"Vrtn", // Variations 
			"Wait", // Wait 
			"WtrP", // WaterPaper 
			"Wtrc", // Watercolor 
			"Wave", // Wave 
			"Wnd ", // Wind 
			"ZgZg", // ZigZag 
			"BacL", // BackLight 
			"ColE", // ColorCast 
			"OpnU", // OpenUntitled 
			// Form
			"Clss", // Class 
			"Enmr", // Enumerated 
			"Idnt", // Identifier 
			"indx", // Index 
			"rele", // Offset 
			"prop", // Property 
			// Key
			"Alis", // _3DAntiAlias
			"A   ", // A
			"Adjs", // Adjustment
			"Algd", // Aligned
			"Algn", // Alignment
			"All ", // AllPS
			"AllE", // AllExcept
			"AlTl", // AllToolOptions
			"AChn", // AlphaChannelOptions
			"AlpC", // AlphaChannels
			"AmbB", // AmbientBrightness
			"AmbC", // AmbientColor
			"Amnt", // Amount
			"AmMx", // AmplitudeMax
			"AmMn", // AmplitudeMin
			"Anch", // Anchor
			"Angl", // Angle
			"Ang1", // Angle1
			"Ang2", // Angle2
			"Ang3", // Angle3
			"Ang4", // Angle4
			"AntA", // AntiAlias
			"Appe", // Append
			"Aply", // Apply
			"Ar  ", // Area
			"Arrw", // Arrowhead
			"As  ", // As
			"Asst", // AssetBin
			"AssC", // AssumedCMYK
			"AssG", // AssumedGray
			"AssR", // AssumedRGB
			"At  ", // At
			"Auto", // Auto
			"AuCo", // AutoContrast
			"Atrs", // AutoErase
			"AtKr", // AutoKern
			"AtUp", // AutoUpdate
			"SwMC", // ShowMenuColors
			"Axis", // Axis
			"B   ", // B
			"Bckg", // Background
			"BckC", // BackgroundColor
			"BckL", // BackgroundLevel
			"Bwd ", // Backward
			"Blnc", // Balance
			"Bsln", // BaselineShift
			"BpWh", // BeepWhenDone
			"BgnR", // BeginRamp
			"BgnS", // BeginSustain
			"bvlD", // BevelDirection
			"ebbl", // BevelEmboss
			"bvlS", // BevelStyle
			"bvlT", // BevelTechnique
			"BgNH", // BigNudgeH
			"BgNV", // BigNudgeV
			"BtDp", // BitDepth
			"Blck", // Black
			"BlcC", // BlackClip
			"Blcn", // BlackGeneration
			"BlcG", // BlackGenerationCurve
			"BlcI", // BlackIntensity
			"BlcL", // BlackLevel
			"BlcL", // BlackLimit
			"Bld ", // Bleed
			"Blnd", // BlendRange
			"Bl  ", // Blue
			"BlBl", // BlueBlackPoint
			"blueFloat", // BlueFloat
			"BlGm", // BlueGamma
			"BlWh", // BlueWhitePoint
			"BlX ", // BlueX
			"BlY ", // BlueY
			"blur", // Blur
			"BlrM", // BlurMethod
			"BlrQ", // BlurQuality
			"Bk  ", // Book
			"BrdT", // BorderThickness
			"Btom", // Bottom
			"Brgh", // Brightness
			"BrsD", // BrushDetail
			"Brsh", // Brushes
			"BrsS", // BrushSize
			"BrsT", // BrushType
			"BmpA", // BumpAmplitude
			"BmpC", // BumpChannel
			"By  ", // By
			"Byln", // Byline
			"BylT", // BylineTitle
			"BytO", // ByteOrder
			"CchP", // CachePrefs
			"Ckmt", // ChokeMatte
			"ClnS", // CloneSource
			"CMYS", // CMYKSetup
			"Clcl", // Calculation
			"Clbr", // CalibrationBars
			"Cptn", // Caption
			"CptW", // CaptionWriter
			"Ctgr", // Category
			"ClSz", // CellSize
			"Cntr", // Center
			"CntC", // CenterCropMarks
			"ChlA", // ChalkArea
			"Chnl", // Channel
			"ChMx", // ChannelMatrix
			"ChnN", // ChannelName
			"Chns", // Channels
			"ChnI", // ChannelsInterleaved
			"ChAm", // CharcoalAmount
			"ChrA", // CharcoalArea
			"ChFX", // ChromeFX
			"City", // City
			"ClrA", // ClearAmount
			"ClPt", // ClippingPath
			"ClpP", // ClippingPathEPS
			"ClpF", // ClippingPathFlatness
			"ClpI", // ClippingPathIndex
			"Clpg", // ClippingPathInfo
			"Clsp", // ClosedSubpath
			"Clr ", // Color
			"Clrh", // ColorChannels
			"ClrC", // ColorCorrection
			"ClrI", // ColorIndicates
			"ClMg", // ColorManagement
			"Clrr", // ColorPickerPrefs
			"ClrT", // ColorTable
			"Clrz", // Colorize
			"Clrs", // Colors
			"ClrL", // ColorsList
			"ClrS", // ColorSpace
			"ClmW", // ColumnWidth
			"CmdK", // CommandKey
			"Cmpn", // Compensation
			"Cmpr", // Compression
			"Cncv", // Concavity
			"Cndt", // Condition
			"Cnst", // Constant
			"Cnst", // Constrain
			"CnsP", // ConstrainProportions
			"Cfov", // ConstructionFOV
			"Cntg", // Contiguous
			"Cntn", // Continue
			"Cnty", // Continuity
			"Cntr", // Contrast
			"Cnvr", // Convert
			"Cpy ", // Copy
			"Cpyr", // Copyright
			"CprN", // CopyrightNotice
			"CrnC", // CornerCropMarks
			"Cnt ", // Count
			"CntN", // CountryName
			"CrcB", // CrackBrightness
			"CrcD", // CrackDepth
			"CrcS", // CrackSpacing
			"blfl", // CreateLayersFromLayerFX
			"Crdt", // Credit
			"Crss", // Crossover
			"Crnt", // Current
			"CrnH", // CurrentHistoryState
			"CrnL", // CurrentLight
			"CrnT", // CurrentToolOptions
			"Crv ", // Curve
			"CrvF", // CurveFile
			"Cstm", // Custom
			"CstF", // CustomForced
			"CstM", // CustomMatte
			"CstP", // CustomPalette
			"Cyn ", // Cyan
			"DrkI", // DarkIntensity
			"Drkn", // Darkness
			"DtCr", // DateCreated
			"Dt  ", // Datum
			"DCS ", // DCS
			"Dfnt", // Definition
			"Dnst", // Density
			"Dpth", // Depth
			"Dstl", // DestBlackMax
			"DstB", // DestBlackMin
			"DstM", // DestinationMode
			"Dstt", // DestWhiteMax
			"DstW", // DestWhiteMin
			"Dtl ", // Detail
			"Dmtr", // Diameter
			"DffD", // DiffusionDither
			"Drct", // Direction
			"DrcB", // DirectionBalance
			"DspF", // DisplaceFile
			"DspM", // DisplacementMap
			"DspP", // DisplayPrefs
			"Dstn", // Distance
			"Dstr", // Distortion
			"Dstr", // Distribution
			"Dthr", // Dither
			"DthA", // DitherAmount
			"Dthp", // DitherPreserve
			"Dthq", // DitherQuality
			"DocI", // DocumentID
			"DtGn", // DotGain
			"DtGC", // DotGainCurves
			"DPXf", // DPXFormat
			"DrSh", // DropShadow
			"Dplc", // Duplicate
			"DnmC", // DynamicColorSliders
			"Edg ", // Edge
			"EdgB", // EdgeBrightness
			"EdgF", // EdgeFidelity
			"EdgI", // EdgeIntensity
			"EdgS", // EdgeSimplicity
			"EdgT", // EdgeThickness
			"EdgW", // EdgeWidth
			"Effc", // Effect
			"EmbP", // EmbedProfiles
			"EmbC", // EmbedCMYK
			"EmbG", // EmbedGray
			"EmbL", // EmbedLab
			"EmbR", // EmbedRGB
			"EmlD", // EmulsionDown
			"enab", // Enabled
			"EGst", // EnableGestures
			"Encd", // Encoding
			"End ", // End
			"EndA", // EndArrowhead
			"EndR", // EndRamp
			"EndS", // EndSustain
			"Engn", // Engine
			"ErsK", // EraserKind
			"ErsT", // EraseToHistory
			"ExcP", // ExactPoints
			"Expr", // Export
			"ExpC", // ExportClipboard
			"Exps", // Exposure
			"Extd", // Extend
			"Extn", // Extension
			"ExtQ", // ExtensionsQuery
			"ExtD", // ExtrudeDepth
			"ExtM", // ExtrudeMaskIncomplete
			"ExtR", // ExtrudeRandom
			"ExtS", // ExtrudeSize
			"ExtF", // ExtrudeSolidFace
			"ExtT", // ExtrudeType
			"EyDr", // EyeDropperSample
			"FdtS", // FadeoutSteps
			"FdT ", // FadeTo
			"FlOf", // Falloff
			"FxCm", // FPXCompress
			"FxQl", // FPXQuality
			"FxSz", // FPXSize
			"FxVw", // FPXView
			"Fthr", // Feather
			"FbrL", // FiberLength
			"File", // File
			"FlCr", // FileCreator
			"FlIn", // FileInfo
			"FilR", // FileReference
			"FlSP", // FileSavePrefs
			"flst", // FilesList
			"FlTy", // FileType
			"Fl  ", // Fill
			"FlCl", // FillColor
			"FlNt", // FillNeutral
			"FlRs", // FilterLayerRandomSeed
			"FlPd", // FilterLayerPersistentData
			"Fngr", // Fingerpainting
			"FlrC", // FlareCenter
			"Fltn", // Flatness
			"Fltt", // Flatten
			"FlpV", // FlipVertical
			"Fcs ", // Focus
			"Fldr", // Folders
			"FntD", // FontDesignAxes
			"FntV", // FontDesignAxesVectors
			"FntN", // FontName
			"Scrp", // FontScript
			"FntS", // FontStyleName
			"FntT", // FontTechnology
			"FrcC", // ForcedColors
			"FrgC", // ForegroundColor
			"FrgL", // ForegroundLevel
			"Fmt ", // Format
			"Fwd ", // Forward
			"FrFX", // FrameFX
			"FrmW", // FrameWidth
			"FTcs", // FreeTransformCenterState
			"Frqn", // Frequency
			"From", // From
			"FrmB", // FromBuiltin
			"FrmM", // FromMode
			"FncK", // FunctionKey
			"Fzns", // Fuzziness
			"GmtW", // GamutWarning
			"GCR ", // GCR
			"GnrP", // GeneralPrefs
			"GFPT", // GIFColorFileType
			"GFCL", // GIFColorLimit
			"GFEC", // GIFExportCaption
			"GFMI", // GIFMaskChannelIndex
			"GFMV", // GIFMaskChannelInverted
			"GFPF", // GIFPaletteFile
			"GFPL", // GIFPaletteType
			"GFCS", // GIFRequiredColorSpaceType
			"GFIT", // GIFRowOrderType
			"GFTC", // GIFTransparentColor
			"GFTB", // GIFTransparentIndexBlue
			"GFTG", // GIFTransparentIndexGreen
			"GFTR", // GIFTransparentIndexRed
			"GFBM", // GIFUseBestMatch
			"Gmm ", // Gamma
			"gblA", // GlobalAngle
			"gagl", // GlobalLightingAngle
			"Glos", // Gloss
			"GlwA", // GlowAmount
			"GlwT", // GlowTechnique
			"Grad", // Gradient
			"Grdf", // GradientFill
			"Grn ", // Grain
			"Grnt", // GrainType
			"Grns", // Graininess
			"Gry ", // Gray
			"GrBh", // GrayBehavior
			"GrSt", // GraySetup
			"Grn ", // Green
			"GrnB", // GreenBlackPoint
			"greenFloat", // GreenFloat
			"GrnG", // GreenGamma
			"GrnW", // GreenWhitePoint
			"GrnX", // GreenX
			"GrnY", // GreenY
			"GrdC", // GridColor
			"Grds", // GridCustomColor
			"GrdM", // GridMajor
			"Grdn", // GridMinor
			"GrdS", // GridStyle
			"Grdt", // GridUnits
			"Grup", // Group
			"GrtW", // GroutWidth
			"GrwS", // GrowSelection
			"Gdes", // Guides
			"GdsC", // GuidesColor
			"Gdss", // GuidesCustomColor
			"GdsS", // GuidesStyle
			"GdPr", // GuidesPrefs
			"GttW", // GutterWidth
			"HlfF", // HalftoneFile
			"HlfS", // HalftoneScreen
			"Hlfp", // HalftoneSpec
			"HlSz", // HalftoneSize
			"Hrdn", // Hardness
			"HCdH", // HasCmdHPreference
			"Hdr ", // Header
			"Hdln", // Headline
			"Hght", // Height
			"HstN", // HostName
			"HghA", // HighlightArea
			"hglC", // HighlightColor
			"HghL", // HighlightLevels
			"hglM", // HighlightMode
			"hglO", // HighlightOpacity
			"HghS", // HighlightStrength
			"HstB", // HistoryBrushSource
			"HstP", // HistoryPrefs
			"HsSS", // HistoryStateSource
			"HsSt", // HistoryStates
			"Hrzn", // Horizontal
			"HrzS", // HorizontalScale
			"HstV", // HostVersion
			"H   ", // Hue
			"ICCE", // ICCEngine
			"ICCt", // ICCSetupName
			"Idnt", // ID
			"Idle", // Idle
			"ImgB", // ImageBalance
			"Impr", // Import
			"Imps", // Impressionist
			"In  ", // In
			"c@#^", // Inherits
			"InkC", // InkColors
			"Inks", // Inks
			"IrGl", // InnerGlow
			"glwS", // InnerGlowSource
			"IrSh", // InnerShadow
			"Inpt", // Input
			"kIBP", // InputBlackPoint
			"Inmr", // InputMapRange
			"Inpr", // InputRange
			"kIWP", // InputWhitePoint
			"Intn", // Intensity
			"Inte", // Intent
			"IntH", // InterfaceBevelHighlight
			"Intv", // InterfaceBevelShadow
			"IntB", // InterfaceBlack
			"Intd", // InterfaceBorder
			"Intk", // InterfaceButtonDarkShadow
			"Intt", // InterfaceButtonDownFill
			"InBF", // InterfaceButtonUpFill
			"ICBL", // InterfaceColorBlue2
			"ICBH", // InterfaceColorBlue32
			"ICGL", // InterfaceColorGreen2
			"ICGH", // InterfaceColorGreen32
			"ICRL", // InterfaceColorRed2
			"ICRH", // InterfaceColorRed32
			"IntI", // InterfaceIconFillActive
			"IntF", // InterfaceIconFillDimmed
			"Intc", // InterfaceIconFillSelected
			"Intm", // InterfaceIconFrameActive
			"Intr", // InterfaceIconFrameDimmed
			"IntS", // InterfaceIconFrameSelected
			"IntP", // InterfacePaletteFill
			"IntR", // InterfaceRed
			"IntW", // InterfaceWhite
			"IntT", // InterfaceToolTipBackground
			"ITTT", // InterfaceToolTipText
			"ITFg", // InterfaceTransparencyForeground
			"ITBg", // InterfaceTransparencyBackground
			"Intr", // Interlace
			"IntC", // InterlaceCreateType
			"IntE", // InterlaceEliminateType
			"Intr", // Interpolation
			"IntM", // InterpolationMethod
			"Invr", // Invert
			"InvM", // InvertMask
			"InvS", // InvertSource2
			"InvT", // InvertTexture
			"IsDr", // IsDirty
			"ItmI", // ItemIndex
			"JPEQ", // JPEGQuality
			"Krng", // Kerning
			"Kywd", // Keywords
			"Knd ", // Kind
			"LZWC", // LZWCompression
			"Lbls", // Labels
			"Lnds", // Landscape
			"LstT", // LastTransform
			"Lefx", // LayerEffects
			"lfxv", // LayerFXVisible
			"Lyr ", // Layer
			"LyrI", // LayerID
			"LyrN", // LayerName
			"Lyrs", // Layers
			"Ldng", // Leading
			"Left", // Left
			"Lngt", // Length
			"Lngt", // TermLength
			"Lns ", // Lens
			"Lvl ", // Level
			"Lvls", // Levels
			"LgDr", // LightDark
			"LghD", // LightDirection
			"LghI", // LightIntensity
			"LghP", // LightPosition
			"LghS", // LightSource
			"LghT", // LightType
			"LghG", // LightenGrout
			"Lght", // Lightness
			"Line", // Line
			"LnkL", // LinkedLayerIDs
			"lagl", // LocalLightingAngle
			"Lald", // LocalLightingAltitude
			"LclR", // LocalRange
			"Lctn", // Location
			"Log ", // Log
			"kLog", // Logarithmic
			"LwCs", // LowerCase
			"Lmnc", // Luminance
			"LTnm", // LUTAnimation
			"Mgnt", // Magenta
			"MkVs", // MakeVisible
			"Mfov", // ManipulationFOV
			"MpBl", // MapBlack
			"Mpng", // Mapping
			"MpgS", // MappingShape
			"Mtrl", // Material
			"Mtrx", // Matrix
			"MttC", // MatteColor
			"Mxm ", // Maximum
			"MxmS", // MaximumStates
			"MmrU", // MemoryUsagePercent
			"Mrge", // Merge
			"Mrgd", // Merged
			"Msge", // Message
			"Mthd", // Method
			"MztT", // MezzotintType
			"Mdpn", // Midpoint
			"MdtL", // MidtoneLevels
			"Mnm ", // Minimum
			"MsmC", // MismatchCMYK
			"MsmG", // MismatchGray
			"MsmR", // MismatchRGB
			"Md  ", // Mode
			"Mnch", // Monochromatic
			"MvT ", // MoveTo
			"Nm  ", // Name
			"Ngtv", // Negative
			"Nw  ", // New
			"Nose", // Noise
			"NnIm", // NonImageData
			"NnLn", // NonLinear
			"null", // Null
			"Nm L, // NumLights"
			"Nmbr", // Number
			"NCch", // NumberOfCacheLevels
			"NC64", // NumberOfCacheLevels64
			"NmbO", // NumberOfChannels
			"NmbC", // NumberOfChildren
			"NmbD", // NumberOfDocuments
			"NmbG", // NumberOfGenerators
			"NmbL", // NumberOfLayers
			"NmbL", // NumberOfLevels
			"NmbP", // NumberOfPaths
			"NmbR", // NumberOfRipples
			"NmbS", // NumberOfSiblings
			"ObjN", // ObjectName
			"Ofst", // Offset
			"On  ", // On
			"Opct", // Opacity
			"Optm", // Optimized
			"Ornt", // Orientation
			"OrgH", // OriginalHeader
			"OrgT", // OriginalTransmissionReference
			"OthC", // OtherCursors
			"OrGl", // OuterGlow
			"Otpt", // Output
			"kOBP", // OutputBlackPoint
			"kOWP", // OutputWhitePoint
			"OvrC", // OverprintColors
			"OvrO", // OverrideOpen
			"ObrP", // OverridePrinter
			"Ovrd", // OverrideSave
			"PnCK", // PaintCursorKind
			"PrIn", // ParentIndex
			"PrNm", // ParentName
			"PNGf", // PNGFilter
			"PGIT", // PNGInterlaceType
			"PMpf", // PageFormat
			"PgNm", // PageNumber
			"PgSt", // PageSetup
			"PgPs", // PagePosition
			"PntC", // PaintingCursors
			"PntT", // PaintType
			"Plt ", // Palette
			"PltF", // PaletteFile
			"PprB", // PaperBrightness
			"Path", // Path
			"PthC", // PathContents
			"PthN", // PathName
			"Pttn", // Pattern
			"Pncl", // PencilWidth
			"Prsp", // PerspectiveIndex
			"Phsp", // Phosphors
			"PckI", // PickerID
			"Pckr", // PickerKind
			"PPSz", // PixelPaintSize
			"Pltf", // Platform
			"PlgF", // PluginFolder
			"PlgP", // PluginPrefs
			"Pts ", // Points
			"Pstn", // Position
			"Pstr", // Posterization
			"PstS", // PostScriptColor
			"PrdC", // PredefinedColors
			"PrfB", // PreferBuiltin
			"PrsA", // PreserveAdditional
			"PrsL", // PreserveLuminosity
			"PrsT", // PreserveTransparency
			"Prs ", // Pressure
			"Prfr", // Preferences
			"Prvw", // Preview
			"PrvK", // PreviewCMYK
			"PrvF", // PreviewFullSize
			"PrvI", // PreviewIcon
			"PrvM", // PreviewMacThumbnail
			"PrvW", // PreviewWinThumbnail
			"PrvQ", // PreviewsQuery
			"PMps", // PrintSettings
			"PrfS", // ProfileSetup
			"PrvS", // ProvinceState
			"Qlty", // Quality
			"EQlt", // ExtendedQuality
			"QucM", // QuickMask
			"RGBS", // RGBSetup
			"Rds ", // Radius
			"RndS", // RandomSeed
			"Rt  ", // Ratio
			"Rcnf", // RecentFiles
			"Rd  ", // Red
			"RdBl", // RedBlackPoint
			"redFloat", // RedFloat
			"RdGm", // RedGamma
			"RdWh", // RedWhitePoint
			"RdX ", // RedX
			"RdY ", // RedY
			"RgsM", // RegistrationMarks
			"Rltv", // Relative
			"Rlf ", // Relief
			"Rfid", // RenderFidelity
			"Rsmp", // Resample
			"RWOZ", // ResizeWindowsOnZoom
			"Rslt", // Resolution
			"RsrI", // ResourceID
			"Rspn", // Response
			"RtnH", // RetainHeader
			"Rvrs", // Reverse
			"Rght", // Right
			"RplM", // RippleMagnitude
			"RplS", // RippleSize
			"Rtt ", // Rotate
			"Rndn", // Roundness
			"RlrH", // RulerOriginH
			"RlrV", // RulerOriginV
			"RlrU", // RulerUnits
			"Strt", // Saturation
			"SvAn", // SaveAndClose
			"SvCm", // SaveComposite
			"PltL", // SavePaletteLocations
			"SvPt", // SavePaths
			"SvPy", // SavePyramids
			"Svng", // Saving
			"Scl ", // Scale
			"SclH", // ScaleHorizontal
			"SclV", // ScaleVertical
			"Scln", // Scaling
			"Scns", // Scans
			"ScrD", // ScratchDisks
			"ScrF", // ScreenFile
			"ScrT", // ScreenType
			"ShdI", // ShadingIntensity
			"ShdN", // ShadingNoise
			"ShdS", // ShadingShape
			"ShpC", // ContourType
			"SrlS", // SerialString
			"Sprt", // Separations
			"sdwC", // ShadowColor
			"ShdI", // ShadowIntensity
			"ShdL", // ShadowLevels
			"sdwM", // ShadowMode
			"sdwO", // ShadowOpacity
			"Shp ", // Shape
			"Shrp", // Sharpness
			"ShrE", // ShearEd
			"ShrP", // ShearPoints
			"ShrS", // ShearSt
			"ShfK", // ShiftKey
			"ShKT", // ShiftKeyToolSwitch
			"ShrN", // ShortNames
			"ShwE", // ShowEnglishFontNames
			"ShwT", // ShowToolTips
			"ShTr", // ShowTransparency
			"Sz  ", // SizeKey
			"Skew", // Skew
			"SmBM", // SmartBlurMode
			"SmBQ", // SmartBlurQuality
			"Smoo", // Smooth
			"Smth", // Smoothness
			"SnpI", // SnapshotInitial
			"SfCl", // SoftClip
			"Sftn", // Softness
			"Sfts", // SmallFontType
			"Sftt", // OldSmallFontType
			"SoFi", // SolidFill
			"Srce", // Source
			"Src2", // Source2
			"SrcM", // SourceMode
			"Spcn", // Spacing
			"SpcI", // SpecialInstructions
			"SphM", // SpherizeMode
			"Spot", // Spot
			"SprR", // SprayRadius
			"SqrS", // SquareSize
			"Srcl", // SrcBlackMax
			"SrcB", // SrcBlackMin
			"Srcm", // SrcWhiteMax
			"SrcW", // SrcWhiteMin
			"Strt", // Start
			"StrA", // StartArrowhead
			"Stte", // State
			"srgh", // Strength
			"srgR", // StrengthRatio
			"Strg", // Strength_PLUGIN
			"StDt", // StrokeDetail
			"SDir", // StrokeDirection
			"StrL", // StrokeLength
			"StrP", // StrokePressure
			"StrS", // StrokeSize
			"StrW", // StrokeWidth
			"Styl", // Style
			"Stys", // Styles
			"StlP", // StylusIsPressure
			"StlC", // StylusIsColor
			"StlO", // StylusIsOpacity
			"StlS", // StylusIsSize
			"SbpL", // SubPathList
			"SplC", // SupplementalCategories
			"SstI", // SystemInfo
			"SstP", // SystemPalette
			"null", // Target
			"Trgp", // TargetPath
			"TrgP", // TargetPathIndex
			"Txt ", // Text
			"TxtC", // TextClickPoint
			"TxtD", // TextData
			"TxtS", // TextStyle
			"Txtt", // TextStyleRange
			"Txtr", // Texture
			"TxtC", // TextureCoverage
			"TxtF", // TextureFile
			"TxtT", // TextureType
			"Thsh", // Threshold
			"TlNm", // TileNumber
			"TlOf", // TileOffset
			"TlSz", // TileSize
			"Ttl ", // Title
			"T   ", // To
			"TBl ", // ToBuiltin
			"ToLk", // ToLinked
			"TMd ", // ToMode
			"TglO", // ToggleOthers
			"Tlrn", // Tolerance
			"Top ", // Top
			"TtlL", // TotalLimit
			"Trck", // Tracking
			"TrnS", // TransferSpec
			"TrnG", // TransparencyGrid
			"TrnF", // TransferFunction
			"Trns", // Transparency
			"TrnC", // TransparencyGridColors
			"TrnG", // TransparencyGridSize
			"TrnP", // TransparencyPrefs
			"TrnS", // TransparencyShape
			"TrnI", // TransparentIndex
			"TrnW", // TransparentWhites
			"Twst", // Twist
			"Type", // Type
			"UC  ", // UCA
			"UntP", // UnitsPrefs
			"URL ", // URL
			"UndA", // UndefinedArea
			"Undl", // Underline
			"Untl", // Untitled
			"UppY", // UpperY
			"Urgn", // Urgency
			"AcrS", // UseAccurateScreens
			"AdPl", // UseAdditionalPlugins
			"UsCc", // UseCacheForHistograms
			"UsCr", // UseCurves
			"UsDf", // UseDefault
			"uglg", // UseGlobalAngle
			"UsIC", // UseICCProfile
			"UsMs", // UseMask
			"UsrM", // UserMaskEnabled
			"Usrs", // UserMaskLinked
			"lnkE", // LinkEnable
			"Usng", // Using
			"Vl  ", // Value
			"Vrnc", // Variance
			"Vct0", // Vector0
			"Vct1", // Vector1
			"VctC", // VectorColor
			"VrsF", // VersionFix
			"VrsM", // VersionMajor
			"VrsN", // VersionMinor
			"Vrtc", // Vertical
			"VrtS", // VerticalScale
			"Vdlp", // VideoAlpha
			"Vsbl", // Visible
			"WtcS", // WatchSuspension
			"watr", // Watermark
			"Wvtp", // WaveType
			"WLMx", // WavelengthMax
			"WLMn", // WavelengthMin
			"WbdP", // WebdavPrefs
			"Wtdg", // WetEdges
			"What", // What
			"WhtC", // WhiteClip
			"WhtI", // WhiteIntensity
			"WhHi", // WhiteIsHigh
			"WhtL", // WhiteLevel
			"WhtP", // WhitePoint
			"WhPt", // WholePath
			"Wdth", // Width
			"WndM", // WindMethod
			"With", // With
			"WrPt", // WorkPath
			"WrkP", // WorkPathIndex
			"X   ", // X
			"Y   ", // Y
			"Ylw ", // Yellow
			"ZZTy", // ZigZagType
			"lSNs", // LegacySerialString
			// Unknown keys
			"comp", // Comp
			// Type
			"#Act", // ActionReference 
			"ActD", // ActionData 
			"ADSt", // AlignDistributeSelector 
			"Alg ", // Alignment 
			"Amnt", // Amount 
			"Annt", // AntiAlias 
			"ArSl", // AreaSelector 
			"AssO", // AssumeOptions 
			"BESs", // BevelEmbossStampStyle 
			"BESl", // BevelEmbossStyle 
			"BtDp", // BitDepth 
			"BlcG", // BlackGeneration 
			"BlnM", // BlendMode 
			"BlrM", // BlurMethod 
			"BlrQ", // BlurQuality 
			"BrsT", // BrushType 
			"BltP", // BuiltinProfile 
			"BltC", // BuiltInContour 
			"CMYE", // CMYKSetupEngine 
			"Clcn", // Calculation 
			"Chnl", // Channel 
			"#ChR", // ChannelReference 
			"Chck", // CheckerboardSize 
			"#Clr", // ClassColor 
			"#ClE", // ClassElement 
			"#Cle", // ClassExport 
			"#ClF", // ClassFormat 
			"#HsV", // ClassHueSatHueSatV2 
			"#ClI", // ClassImport 
			"#ClM", // ClassMode 
			"#ClS", // ClassStringFormat 
			"#CTE", // ClassTextExport 
			"#ClT", // ClassTextImport 
			"Clr ", // Color 
			"#ClC", // ColorChannel 
			"ClrP", // ColorPalette 
			"ClrS", // ColorSpace 
			"Clry", // ColorStopType 
			"Clrs", // Colors 
			"Cmpn", // Compensation 
			"CntE", // ContourEdge 
			"Cnvr", // Convert 
			"CrcM", // CorrectionMethod 
			"CrsK", // CursorKind 
			"DCS ", // DCS 
			"DpDp", // DeepDepth 
			"Dpth", // Depth 
			"DfsM", // DiffuseMode 
			"Drct", // Direction 
			"DspM", // DisplacementMap 
			"Dstr", // Distribution 
			"Dthr", // Dither 
			"Dthq", // DitherQuality 
			"#DcR", // DocumentReference 
			"EPSP", // EPSPreview 
			"#ElR", // ElementReference 
			"Encd", // Encoding 
			"ErsK", // EraserKind 
			"ExtR", // ExtrudeRandom 
			"ExtT", // ExtrudeType 
			"EyDp", // EyeDropperSample 
			"FxCm", // FPXCompress 
			"Fl  ", // Fill 
			"FlCl", // FillColor 
			"FlCn", // FillContents 
			"FlMd", // FillMode 
			"FrcC", // ForcedColors 
			"FrFl", // FrameFill 
			"FStl", // FrameStyle 
			"GFPT", // GIFColorFileType 
			"GFPL", // GIFPaletteType 
			"GFCS", // GIFRequiredColorSpaceType 
			"GFIT", // GIFRowOrderType 
			"GlbC", // GlobalClass 
			"GlbO", // GlobalObject 
			"GrdT", // GradientType 
			"GrdF", // GradientForm 
			"Grnt", // GrainType 
			"GrBh", // GrayBehavior 
			"GdGr", // GuideGridColor 
			"GdGS", // GuideGridStyle 
			"HstS", // HistoryStateSource 
			"HrzL", // HorizontalLocation 
			"#ImR", // ImageReference 
			"IGSr", // InnerGlowSource 
			"#inC", // IntegerChannel 
			"Inte", // Intent 
			"IntC", // InterlaceCreateType 
			"IntE", // InterlaceEliminateType 
			"Intp", // Interpolation 
			"Klvn", // Kelvin 
			"#Klv", // KelvinCustomWhitePoint 
			"Lns ", // Lens 
			"LghD", // LightDirection 
			"LghP", // LightPosition 
			"LghT", // LightType 
			"#Lct", // LocationReference 
			"MskI", // MaskIndicator 
			"MttC", // MatteColor 
			"BETE", // MatteTechnique 
			"MnIt", // MenuItem 
			"Mthd", // Method 
			"MztT", // MezzotintType 
			"Md  ", // Mode 
			"Ntfy", // Notify 
			"Objc", // Object 
			"obj ", // ObjectReference 
			"OnOf", // OnOff 
			"Ordn", // Ordinal 
			"Ornt", // Orientation 
			"PNGf", // PNGFilter 
			"PGIT", // PNGInterlaceType 
			"PgPs", // PagePosition 
			"PthK", // PathKind 
			"#PtR", // PathReference 
			"Phsp", // Phosphors 
			"#Phs", // PhosphorsCustomPhosphors 
			"PckK", // PickerKind 
			"PPSz", // PixelPaintSize 
			"Pltf", // Platform 
			"Prvw", // Preview 
			"Prvt", // PreviewCMYK 
			"PrfM", // ProfileMismatch 
			"PrgI", // PurgeItem 
			"QCSt", // QuadCenterState 
			"Qlty", // Quality 
			"QurS", // QueryState 
			"RGBS", // RGBSetupSource 
			"tdta", // RawData 
			"RplS", // RippleSize 
			"RlrU", // RulerUnits 
			"ScrT", // ScreenType 
			"Shp ", // Shape 
			"SmBM", // SmartBlurMode 
			"SmBQ", // SmartBlurQuality 
			"Cndn", // SourceMode 
			"SphM", // SpherizeMode 
			"Stte", // State 
			"#StC", // StringClassFormat 
			"#sth", // StringChannel 
			"#Stm", // StringCompensation 
			"#Stf", // StringFSS 
			"#StI", // StringInteger 
			"StrD", // StrokeDirection 
			"StrL", // StrokeLocation 
			"TxtT", // TextureType 
			"Trnl", // TransparencyGridColors 
			"TrnG", // TransparencyGridSize 
			"#TyM", // TypeClassModeOrClassMode 
			"UndA", // UndefinedArea 
			"UntF", // UnitFloat 
			"Urgn", // Urgency 
			"UsrM", // UserMaskOptions 
			"VlLs", // ValueList 
			"VrtL", // VerticalLocation 
			"Wvtp", // WaveType 
			"WndM", // WindMethod 
			"YsN ", // YesNo 
			"ZZTy", // ZigZagType 
			// Unit
			"#Ang", // Angle 
			"#Rsl", // Density 
			"#Rlt", // Distance 
			"#Nne", // _None 
			"#Prc", // Percent 
			"#Pxl", // Pixels 
			"#Mlm", // Millimeters 
			"#Pnt" // Points
		};
	}
}

PSAPI_NAMESPACE_END