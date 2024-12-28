#include "doctest.h"

#include "PhotoshopFile/PhotoshopFile.h"
#include "LayeredFile/LayeredFile.h"
#include "Macros.h"

#include <filesystem>


/*
These test cases simply check if we can read -> write -> read again parsing through the LayeredFile struct. Unfortunately these written files do have to be checked by
hand as we internally can read files that Photoshop sometimes cannot.
*/


void checkFileRoundtripping(const std::filesystem::path& inDir, const std::filesystem::path& outDir, const std::filesystem::path& psFile)
{
	using namespace NAMESPACE_PSAPI;

	std::filesystem::path fullInPath = inDir / psFile;
	std::filesystem::path fullOutPath = outDir / psFile;
	ProgressCallback callback;

	// Load the input file
	auto inputFile = File(fullInPath);
	auto psDocumentPtr = std::make_unique<PhotoshopFile>();
	psDocumentPtr->read(inputFile, callback);
	if (psDocumentPtr->m_Header.m_Depth == Enum::BitDepth::BD_8)
	{
		LayeredFile<bpp8_t> layeredFile = { std::move(psDocumentPtr), fullInPath };

		// Write to disk
		File::FileParams params = File::FileParams();
		params.doRead = false;
		params.forceOverwrite = true;
		auto outputFile = File(fullOutPath, params);
		auto psdOutDocumentPtr = layered_to_photoshop(std::move(layeredFile), fullOutPath);
		psdOutDocumentPtr->write(outputFile, callback);

		// Read back into LayeredFile
		auto inputFileRoundtripped = File(fullOutPath);
		auto psDocumentPtrRoundtripped = std::make_unique<PhotoshopFile>();
		psDocumentPtrRoundtripped->read(inputFileRoundtripped, callback);

		LayeredFile<bpp8_t> layeredFileRoundtripped = { std::move(psDocumentPtrRoundtripped), fullOutPath };
	}
	else if (psDocumentPtr->m_Header.m_Depth == Enum::BitDepth::BD_16)
	{
		LayeredFile<bpp16_t> layeredFile = { std::move(psDocumentPtr), fullInPath };

		// Write to disk
		File::FileParams params = File::FileParams();
		params.doRead = false;
		params.forceOverwrite = true;
		auto outputFile = File(fullOutPath, params);
		auto psdOutDocumentPtr = layered_to_photoshop(std::move(layeredFile), fullOutPath);
		psdOutDocumentPtr->write(outputFile, callback);

		// Read back into LayeredFile
		auto inputFileRoundtripped = File(fullOutPath);
		auto psDocumentPtrRoundtripped = std::make_unique<PhotoshopFile>();
		psDocumentPtrRoundtripped->read(inputFileRoundtripped, callback);

		LayeredFile<bpp16_t> layeredFileRoundtripped = { std::move(psDocumentPtrRoundtripped), fullOutPath };
	}
	else if (psDocumentPtr->m_Header.m_Depth == Enum::BitDepth::BD_32)
	{
		LayeredFile<bpp32_t> layeredFile = { std::move(psDocumentPtr), fullInPath };

		// Write to disk
		File::FileParams params = File::FileParams();
		params.doRead = false;
		params.forceOverwrite = true;
		auto outputFile = File(fullOutPath, params);
		auto psdOutDocumentPtr = layered_to_photoshop(std::move(layeredFile), fullOutPath);
		psdOutDocumentPtr->write(outputFile, callback);

		// Read back into LayeredFile
		auto inputFileRoundtripped = File(fullOutPath);
		auto psDocumentPtrRoundtripped = std::make_unique<PhotoshopFile>();
		psDocumentPtrRoundtripped->read(inputFileRoundtripped, callback);

		LayeredFile<bpp32_t> layeredFileRoundtripped = { std::move(psDocumentPtrRoundtripped), fullOutPath };
	}
}


TEST_CASE("Check Roundtripping Compression")
{
	static std::vector<std::filesystem::path> fileNames =
	{
		"Compression_Mixed_8bit.psd",
		"Compression_Mixed_8bit.psd",
		"Compression_RAW_8bit.psb",
		"Compression_RAW_8bit.psd",
		"Compression_RLE_8bit.psb",
		"Compression_RLE_8bit.psd",
		"Compression_ZipPrediction_MaximizeCompatibilityOff_16bit.psb",	// The MaximizeCompatibility setting will be ignored by us but it is another test case
		"Compression_ZipPrediction_MaximizeCompatibilityOff_16bit.psd",	// The MaximizeCompatibility setting will be ignored by us but it is another test case
		"Compression_ZipPrediction_MaximizeCompatibilityOff_32bit.psb",	// The MaximizeCompatibility setting will be ignored by us but it is another test case
		"Compression_ZipPrediction_MaximizeCompatibilityOff_32bit.psd",	// The MaximizeCompatibility setting will be ignored by us but it is another test case
		"Compression_ZipPrediction_16bit.psb",
		"Compression_ZipPrediction_16bit.psd",
		"Compression_ZipPrediction_32bit.psb",
		"Compression_ZipPrediction_32bit.psd"
	};

	const std::filesystem::path inDir = std::filesystem::current_path() / "documents/Compression";
	const std::filesystem::path outDir = std::filesystem::current_path() / "documents/TestRoundtrippingOutput";

	for (const auto& fileName : fileNames)
	{
		checkFileRoundtripping(inDir, outDir, fileName);
	}
}


TEST_CASE("Check Roundtripping DPI")
{
	static std::vector<std::filesystem::path> fileNames =
	{
		"300dpi.psd",
		"300_point_5_dpi.psd",
		"700dpi.psd"
	};

	const std::filesystem::path inDir = std::filesystem::current_path() / "documents/DPI";
	const std::filesystem::path outDir = std::filesystem::current_path() / "documents/TestRoundtrippingOutput";

	for (const auto& fileName : fileNames)
	{
		checkFileRoundtripping(inDir, outDir, fileName);
	}
}


TEST_CASE("Check Roundtripping Groups")
{
	static std::vector<std::filesystem::path> fileNames =
	{
		"Groups_8bit.psb",
		"Groups_8bit.psd",
		"Groups_16bit.psb",
		"Groups_16bit.psd",
		"Groups_32bit.psb",
		"Groups_32bit.psd"
	};

	const std::filesystem::path inDir = std::filesystem::current_path() / "documents/Groups";
	const std::filesystem::path outDir = std::filesystem::current_path() / "documents/TestRoundtrippingOutput";

	for (const auto& fileName : fileNames)
	{
		checkFileRoundtripping(inDir, outDir, fileName);
	}
}


TEST_CASE("Check Roundtripping ICCProfiles")
{
	static std::vector<std::filesystem::path> fileNames =
	{
		"AdobeRGB1998.psb",
		"AppleRGB.psb",
		"CIERGB.psb"
	};

	const std::filesystem::path inDir = std::filesystem::current_path() / "documents/ICCProfiles";
	const std::filesystem::path outDir = std::filesystem::current_path() / "documents/TestRoundtrippingOutput";

	for (const auto& fileName : fileNames)
	{
		checkFileRoundtripping(inDir, outDir, fileName);
	}
}


TEST_CASE("Check Roundtripping Masks")
{
	static std::vector<std::filesystem::path> fileNames =
	{
		"SingleLayer_8bit.psb",
		"SingleLayer_8bit.psd",
		"SingleLayer_8bit_MaximizeCompatibilityOff.psb",	// The MaximizeCompatibility setting will be ignored by us but it is another test case
		"SingleLayer_8bit_MaximizeCompatibilityOff.psd",	// The MaximizeCompatibility setting will be ignored by us but it is another test case
		"SingleLayer_16bit.psb",
		"SingleLayer_16bit.psd",
		"SingleLayer_16bit_MaximizeCompatibilityOff.psb",	// The MaximizeCompatibility setting will be ignored by us but it is another test case
		"SingleLayer_16bit_MaximizeCompatibilityOff.psd",	// The MaximizeCompatibility setting will be ignored by us but it is another test case
		"SingleLayer_32bit.psb",
		"SingleLayer_32bit.psd",
		"SingleLayer_32bit_MaximizeCompatibilityOff.psb",	// The MaximizeCompatibility setting will be ignored by us but it is another test case
		"SingleLayer_32bit_MaximizeCompatibilityOff.psd"	// The MaximizeCompatibility setting will be ignored by us but it is another test case
	};

	const std::filesystem::path inDir = std::filesystem::current_path() / "documents/SingleLayer";
	const std::filesystem::path outDir = std::filesystem::current_path() / "documents/TestRoundtrippingOutput";

	for (const auto& fileName : fileNames)
	{
		checkFileRoundtripping(inDir, outDir, fileName);
	}
}


TEST_CASE("Check Roundtripping CMYK")
{
	static std::vector<std::filesystem::path> fileNames =
	{
		"CMYK_8.psd",
		"CMYK_8.psb",
		"CMYK_16.psd",
		"CMYK_16.psb"
	};

	const std::filesystem::path inDir = std::filesystem::current_path() / "documents/CMYK";
	const std::filesystem::path outDir = std::filesystem::current_path() / "documents/TestRoundtrippingOutput";

	for (const auto& fileName : fileNames)
	{
		checkFileRoundtripping(inDir, outDir, fileName);
	}
}


TEST_CASE("Check Roundtripping Grayscale")
{
	static std::vector<std::filesystem::path> fileNames =
	{
		"Grayscale_8.psd",
		"Grayscale_8.psb",
		"Grayscale_16.psd",
		"Grayscale_16.psb",
		"Grayscale_32.psd",
		"Grayscale_32.psb"
	};

	const std::filesystem::path inDir = std::filesystem::current_path() / "documents/Grayscale";
	const std::filesystem::path outDir = std::filesystem::current_path() / "documents/TestRoundtrippingOutput";

	for (const auto& fileName : fileNames)
	{
		checkFileRoundtripping(inDir, outDir, fileName);
	}
}