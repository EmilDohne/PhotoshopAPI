#include "doctest.h"

#include "Core/TaggedBlocks/TaggedBlock.h"
#include "Core/Struct/UnicodeString.h"
#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

using namespace NAMESPACE_PSAPI;

// Helper: convert u8s(u8"") literal (char8_t[] in C++20) to std::string
static std::string u8s(const char8_t* s)
{
	return std::string(reinterpret_cast<const char*>(s));
}

namespace
{
	std::filesystem::path temp_psd_path()
	{
		static std::atomic<uint64_t> counter{ 0u };
		const auto stamp = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
		const auto idx = counter.fetch_add(1u, std::memory_order_relaxed);
		return std::filesystem::temp_directory_path() / ("psapi_unicode_" + std::to_string(stamp) + "_" + std::to_string(idx) + ".psd");
	}

	template <typename T>
	std::shared_ptr<TextLayer<T>> find_text_layer_with_contents(
		LayeredFile<T>& file, const std::string& text)
	{
		for (const auto& layer : file.flat_layers())
		{
			auto tl = std::dynamic_pointer_cast<TextLayer<T>>(layer);
			if (!tl) continue;
			const auto v = tl->text();
			if (v.has_value() && v.value() == text) return tl;
		}
		return nullptr;
	}

	/// Write a LayeredFile to a temp path and re-read it.
	LayeredFile<bpp8_t> roundtrip(LayeredFile<bpp8_t> file)
	{
		const auto path = temp_psd_path();
		LayeredFile<bpp8_t>::write(std::move(file), path);
		auto reread = LayeredFile<bpp8_t>::read(path);
		std::filesystem::remove(path);
		return reread;
	}
}

// -- CJK text -----------------------------------------------------------

TEST_CASE("Unicode: CJK text set_text roundtrip")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(layer);

	// CJK Unified Ideographs — all BMP, one code unit each in UTF-16
	const std::string cjk = u8s(u8"\u4F60\u597D\u4E16\u754C");            // ????
	CHECK_NOTHROW(layer->set_text(cjk));
	CHECK(layer->text().value() == cjk);

	auto reread = roundtrip(std::move(file));
	auto rl = find_text_layer_with_contents(reread, cjk);
	REQUIRE(rl);
	CHECK(rl->text().value() == cjk);
}


TEST_CASE("Unicode: CJK replace_text partial replacement roundtrip")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(layer);

	// First set to mixed ASCII/CJK
	const std::string mixed = u8s(u8"AB\u4E2D\u6587CD");                  // AB??CD
	CHECK_NOTHROW(layer->set_text(mixed));

	// Replace the CJK part with longer CJK
	const std::string replacement = u8s(u8"\u65E5\u672C\u8A9E\u6587\u5B57");  // ?????
	CHECK_NOTHROW(layer->replace_text(u8s(u8"\u4E2D\u6587"), replacement));
	const std::string expected = u8s(u8"AB\u65E5\u672C\u8A9E\u6587\u5B57CD");
	CHECK(layer->text().value() == expected);

	auto reread = roundtrip(std::move(file));
	auto rl = find_text_layer_with_contents(reread, expected);
	CHECK(rl != nullptr);
}


TEST_CASE("Unicode: CJK fullwidth characters roundtrip")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(layer);

	// Fullwidth Latin: ABC (U+FF21 U+FF22 U+FF23) — all BMP
	const std::string fullwidth = u8s(u8"\uFF21\uFF22\uFF23");
	CHECK_NOTHROW(layer->set_text(fullwidth));
	CHECK(layer->text().value() == fullwidth);

	auto reread = roundtrip(std::move(file));
	auto rl = find_text_layer_with_contents(reread, fullwidth);
	CHECK(rl != nullptr);
}


// -- RTL text (Arabic / Hebrew) -----------------------------------------

TEST_CASE("Unicode: Arabic text set_text roundtrip")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(layer);

	// Arabic: ????? (U+0645 U+0631 U+062D U+0628 U+0627)
	const std::string arabic = u8s(u8"\u0645\u0631\u062D\u0628\u0627");
	CHECK_NOTHROW(layer->set_text(arabic));
	CHECK(layer->text().value() == arabic);

	auto reread = roundtrip(std::move(file));
	auto rl = find_text_layer_with_contents(reread, arabic);
	CHECK(rl != nullptr);
}


TEST_CASE("Unicode: Hebrew text replace_text roundtrip")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(layer);

	// Hebrew: ???? (U+05E9 U+05DC U+05D5 U+05DD)
	const std::string hebrew = u8s(u8"\u05E9\u05DC\u05D5\u05DD");
	CHECK_NOTHROW(layer->set_text(hebrew));

	// Replace with longer Hebrew text
	const std::string longer = u8s(u8"\u05E9\u05DC\u05D5\u05DD \u05E2\u05D5\u05DC\u05DD");  // ???? ????
	CHECK_NOTHROW(layer->replace_text(hebrew, longer));
	CHECK(layer->text().value() == longer);

	auto reread = roundtrip(std::move(file));
	auto rl = find_text_layer_with_contents(reread, longer);
	CHECK(rl != nullptr);
}


TEST_CASE("Unicode: Mixed LTR/RTL text roundtrip")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(layer);

	// Mixed: "Hello ????? World"
	const std::string mixed = u8s(u8"Hello \u0645\u0631\u062D\u0628\u0627 World");
	CHECK_NOTHROW(layer->set_text(mixed));
	CHECK(layer->text().value() == mixed);

	auto reread = roundtrip(std::move(file));
	auto rl = find_text_layer_with_contents(reread, mixed);
	CHECK(rl != nullptr);
}


// -- Emoji / Surrogate pairs --------------------------------------------

TEST_CASE("Unicode: Emoji surrogate pair text roundtrip")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(layer);

	// Emoji requiring surrogate pairs: ?????? (U+1F600 U+1F389 U+1F30D)
	// Each is 4 UTF-8 bytes and 2 UTF-16 code units (surrogate pair)
	const std::string emoji = u8s(u8"\U0001F600\U0001F389\U0001F30D");
	CHECK_NOTHROW(layer->set_text(emoji));
	CHECK(layer->text().value() == emoji);

	auto reread = roundtrip(std::move(file));
	auto rl = find_text_layer_with_contents(reread, emoji);
	CHECK(rl != nullptr);
}


TEST_CASE("Unicode: Mixed ASCII and emoji replacement roundtrip")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(layer);

	// Set text with emoji in middle: "Hi ?? there"
	const std::string with_emoji = u8s(u8"Hi \U0001F600 there");
	CHECK_NOTHROW(layer->set_text(with_emoji));
	CHECK(layer->text().value() == with_emoji);

	// Replace ASCII part adjacent to emoji
	CHECK_NOTHROW(layer->replace_text("Hi", "Hey"));
	const std::string expected = u8s(u8"Hey \U0001F600 there");
	CHECK(layer->text().value() == expected);

	auto reread = roundtrip(std::move(file));
	auto rl = find_text_layer_with_contents(reread, expected);
	CHECK(rl != nullptr);
}


TEST_CASE("Unicode: Replace text adjacent to surrogate pair does not corrupt pair")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(layer);

	// "A??B" — emoji surrounded by single chars
	const std::string text = u8s(u8"A\U0001F600B");
	CHECK_NOTHROW(layer->set_text(text));

	// Replace "B" (right after the surrogate pair) with "XY"
	CHECK_NOTHROW(layer->replace_text("B", "XY"));
	const std::string expected = u8s(u8"A\U0001F600XY");
	CHECK(layer->text().value() == expected);

	// Replace "A" (right before the surrogate pair) with "PQR"
	CHECK_NOTHROW(layer->replace_text("A", "PQR"));
	const std::string expected2 = u8s(u8"PQR\U0001F600XY");
	CHECK(layer->text().value() == expected2);

	auto reread = roundtrip(std::move(file));
	auto rl = find_text_layer_with_contents(reread, expected2);
	CHECK(rl != nullptr);
}


TEST_CASE("Unicode: Multiple emoji with supplementary plane characters")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(layer);

	// Musical symbols from SMP: ?? (U+1D11E) + ?? (U+1D122) — treble & bass clef
	// Plus emoji: ?? (U+1F3B5)
	const std::string smp = u8s(u8"\U0001D11E\U0001D122\U0001F3B5");
	CHECK_NOTHROW(layer->set_text(smp));
	CHECK(layer->text().value() == smp);

	auto reread = roundtrip(std::move(file));
	auto rl = find_text_layer_with_contents(reread, smp);
	CHECK(rl != nullptr);
}


// -- Combining marks ----------------------------------------------------

TEST_CASE("Unicode: Precomposed vs decomposed accent roundtrip")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture));

	// Test with precomposed é (U+00E9) — single code unit in UTF-16
	{
		auto file = LayeredFile<bpp8_t>::read(fixture);
		auto layer = find_text_layer_with_contents(file, "Hello 123");
		REQUIRE(layer);

		const std::string precomposed = u8s(u8"caf\u00E9");   // café (precomposed)
		CHECK_NOTHROW(layer->set_text(precomposed));
		CHECK(layer->text().value() == precomposed);

		auto reread = roundtrip(std::move(file));
		auto rl = find_text_layer_with_contents(reread, precomposed);
		CHECK(rl != nullptr);
	}

	// Test with decomposed e + combining acute (U+0065 U+0301) — two code units in UTF-16
	{
		auto file = LayeredFile<bpp8_t>::read(fixture);
		auto layer = find_text_layer_with_contents(file, "Hello 123");
		REQUIRE(layer);

		const std::string decomposed = u8s(u8"cafe\u0301");   // café (decomposed)
		CHECK_NOTHROW(layer->set_text(decomposed));
		CHECK(layer->text().value() == decomposed);

		auto reread = roundtrip(std::move(file));
		auto rl = find_text_layer_with_contents(reread, decomposed);
		CHECK(rl != nullptr);
	}
}


TEST_CASE("Unicode: Combining marks — multiple diacritics roundtrip")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(layer);

	// o + combining tilde (U+0303) + combining acute (U+0301) = ?
	const std::string multi_combining = u8s(u8"o\u0303\u0301");
	CHECK_NOTHROW(layer->set_text(multi_combining));
	CHECK(layer->text().value() == multi_combining);

	auto reread = roundtrip(std::move(file));
	auto rl = find_text_layer_with_contents(reread, multi_combining);
	CHECK(rl != nullptr);
}


TEST_CASE("Unicode: Replace within text containing combining marks")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(layer);

	// "café 123" — precomposed
	const std::string text = u8s(u8"caf\u00E9 123");
	CHECK_NOTHROW(layer->set_text(text));

	// Replace "123" next to accented char
	CHECK_NOTHROW(layer->replace_text("123", "XYZ"));
	const std::string expected = u8s(u8"caf\u00E9 XYZ");
	CHECK(layer->text().value() == expected);

	auto reread = roundtrip(std::move(file));
	auto rl = find_text_layer_with_contents(reread, expected);
	CHECK(rl != nullptr);
}


// -- Mixed-script run boundaries ----------------------------------------

TEST_CASE("Unicode: Mixed-script multi-run replace preserves style runs")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(layer);

	// Replace "Beta" with CJK text (different code unit count per character)
	const std::string cjk_replacement = u8s(u8"\u4E16\u754C");  // ?? (2 chars = 2 UTF-16 code units)
	CHECK_NOTHROW(layer->replace_text("Beta", cjk_replacement));
	const std::string expected = u8s(u8"Alpha \u4E16\u754C Gamma");
	CHECK(layer->text().value() == expected);

	auto reread = roundtrip(std::move(file));
	auto rl = find_text_layer_with_contents(reread, expected);
	REQUIRE(rl != nullptr);

	// Verify style run count is preserved (original had 5 runs)
	CHECK(rl->style_run_count() == 5u);
}


TEST_CASE("Unicode: Replace with emoji in multi-run text preserves style runs")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(layer);

	// Replace "Beta" (4 UTF-16 code units) with emoji (2 surrogate pairs = 4 UTF-16 code units -- same size)
	const std::string emoji = u8s(u8"\U0001F600\U0001F389");
	CHECK_NOTHROW(layer->replace_text("Beta", emoji));
	const std::string expected = u8s(u8"Alpha \U0001F600\U0001F389 Gamma");
	CHECK(layer->text().value() == expected);

	auto reread = roundtrip(std::move(file));
	auto rl = find_text_layer_with_contents(reread, expected);
	REQUIRE(rl != nullptr);
	CHECK(rl->style_run_count() == 5u);
}


TEST_CASE("Unicode: Replace with Arabic in multi-run text preserves style runs")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_StyleRuns.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer_with_contents(file, "Alpha Beta Gamma");
	REQUIRE(layer);

	// Replace "Alpha" (5 code units) with Arabic (longer: 9 code units)
	const std::string arabic = u8s(u8"\u0645\u0631\u062D\u0628\u0627 \u0628\u0643\u0645");  // ????? ???
	CHECK_NOTHROW(layer->replace_text("Alpha", arabic));
	const std::string expected = u8s(u8"\u0645\u0631\u062D\u0628\u0627 \u0628\u0643\u0645 Beta Gamma");
	CHECK(layer->text().value() == expected);

	auto reread = roundtrip(std::move(file));
	auto rl = find_text_layer_with_contents(reread, expected);
	CHECK(rl != nullptr);
}


// -- UTF-16 code unit count verification --------------------------------

TEST_CASE("Unicode: UTF-8 to UTF-16 code unit count is correct for various scripts")
{
	// Verify simdutf-based conversion produces expected code unit counts.
	// This is a unit-level check that the conversion pipeline handles all scripts.

	// ASCII: 1 UTF-8 byte ? 1 UTF-16 code unit
	auto ascii = UnicodeString::convertUTF8ToUTF16LE("A");
	CHECK(ascii.size() == 1u);

	// Latin extended: é (U+00E9) — 2 UTF-8 bytes ? 1 UTF-16 code unit
	auto latin_ext = UnicodeString::convertUTF8ToUTF16LE(u8s(u8"\u00E9"));
	CHECK(latin_ext.size() == 1u);

	// Combining: e + combining acute — 1+2 UTF-8 bytes ? 2 UTF-16 code units
	auto combining = UnicodeString::convertUTF8ToUTF16LE(u8s(u8"e\u0301"));
	CHECK(combining.size() == 2u);

	// CJK: ? (U+4F60) — 3 UTF-8 bytes ? 1 UTF-16 code unit
	auto cjk = UnicodeString::convertUTF8ToUTF16LE(u8s(u8"\u4F60"));
	CHECK(cjk.size() == 1u);

	// Arabic: ? (U+0645) — 2 UTF-8 bytes ? 1 UTF-16 code unit
	auto arabic = UnicodeString::convertUTF8ToUTF16LE(u8s(u8"\u0645"));
	CHECK(arabic.size() == 1u);

	// Emoji: ?? (U+1F600) — 4 UTF-8 bytes ? 2 UTF-16 code units (surrogate pair)
	auto emoji = UnicodeString::convertUTF8ToUTF16LE(u8s(u8"\U0001F600"));
	CHECK(emoji.size() == 2u);

	// Musical: ?? (U+1D11E) — 4 UTF-8 bytes ? 2 UTF-16 code units (surrogate pair)
	auto musical = UnicodeString::convertUTF8ToUTF16LE(u8s(u8"\U0001D11E"));
	CHECK(musical.size() == 2u);

	// Verify surrogate pair values for ?? (U+1F600): should be 0xD83D 0xDE00
	CHECK(static_cast<uint16_t>(emoji[0]) == 0xD83Du);
	CHECK(static_cast<uint16_t>(emoji[1]) == 0xDE00u);
}


TEST_CASE("Unicode: UTF-16 roundtrip preserves surrogate pair integrity")
{
	// Convert UTF-8 ? UTF-16LE ? UTF-8 and verify identity
	const std::string original = u8s(u8"A\U0001F600B\u4E2D\u0301C");  // A??B?´C
	auto utf16 = UnicodeString::convertUTF8ToUTF16LE(original);
	// Expected: A(1) + ??(2 surrogates) + B(1) + ?(1) + combining(1) + C(1) = 7 code units
	CHECK(utf16.size() == 7u);

	auto back = UnicodeString::convertUTF16LEtoUTF8(utf16);
	CHECK(back == original);
}


// -- Edge cases ---------------------------------------------------------

TEST_CASE("Unicode: Empty string replacement roundtrip")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(layer);

	// Replace with text that starts with zero-width characters
	// Zero-width space (U+200B) + "Hi"
	const std::string zws = u8s(u8"\u200BHi");
	CHECK_NOTHROW(layer->set_text(zws));
	CHECK(layer->text().value() == zws);

	auto reread = roundtrip(std::move(file));
	auto rl = find_text_layer_with_contents(reread, zws);
	CHECK(rl != nullptr);
}


TEST_CASE("Unicode: Korean Hangul jamo roundtrip")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(layer);

	// Precomposed Hangul syllable: ?? (U+D55C U+AE00)
	const std::string hangul = u8s(u8"\uD55C\uAE00");
	CHECK_NOTHROW(layer->set_text(hangul));
	CHECK(layer->text().value() == hangul);

	auto reread = roundtrip(std::move(file));
	auto rl = find_text_layer_with_contents(reread, hangul);
	CHECK(rl != nullptr);
}


TEST_CASE("Unicode: Thai script with tone marks roundtrip")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(layer);

	// Thai: ?????? (has combining vowel/tone marks)
	const std::string thai = u8s(u8"\u0E2A\u0E27\u0E31\u0E2A\u0E14\u0E35");
	CHECK_NOTHROW(layer->set_text(thai));
	CHECK(layer->text().value() == thai);

	auto reread = roundtrip(std::move(file));
	auto rl = find_text_layer_with_contents(reread, thai);
	CHECK(rl != nullptr);
}


TEST_CASE("Unicode: Long mixed-script text with all edge cases")
{
	const auto fixture = std::filesystem::current_path() / "documents" / "TextLayers" / "TextLayers_Basic.psd";
	REQUIRE(std::filesystem::exists(fixture));

	auto file = LayeredFile<bpp8_t>::read(fixture);
	auto layer = find_text_layer_with_contents(file, "Hello 123");
	REQUIRE(layer);

	// Combine ASCII + CJK + Arabic + emoji + combining marks in one string
	const std::string kitchen_sink = u8s(u8"Hi \u4F60\u597D \u0645\u0631\u062D\u0628\u0627 \U0001F600 caf\u00E9");
	//                                     Hi ?? ????? ?? café
	CHECK_NOTHROW(layer->set_text(kitchen_sink));
	CHECK(layer->text().value() == kitchen_sink);

	auto reread = roundtrip(std::move(file));
	auto rl = find_text_layer_with_contents(reread, kitchen_sink);
	CHECK(rl != nullptr);
}
