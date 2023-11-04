#pragma once

#include "../../Macros.h"
#include "../../PhotoshopFile/LayerAndMaskInformation.h"
#include "../Enum.h"
#include "File.h"
#include "Signature.h"

#include <memory>

PSAPI_NAMESPACE_BEGIN

namespace TaggedBlock
{
	namespace
	{
		struct Base
		{
			Signature m_Signature;
			uint64_t m_Offset;
		protected:
			Enum::TaggedBlockKey m_Key;
		};

		// Generic TaggedBlock base, if no match is found construct this one
		// ---------------------------------------------------------------------------------------------------------------------
		// ---------------------------------------------------------------------------------------------------------------------
		struct Base_32 : Base
		{
			uint32_t m_Length;
			// Leave the data up to the implementation itself
		};

		// Use this struct if the keys are:
		// "LMsk", "Lr16", "Lr32", "Layr", "Mt16", "Mt32", "Mtrn", "Alph", "FMsk", "lnk2", "FEid", "FXid", "PxSD" or "cinf"
		struct Base_64 : Base
		{
			uint64_t m_Length;
			// Leave the data up to the implementation itself
		};
	}

	struct Generic_32 : Base_32
	{
		std::vector<uint8_t> m_Data;

		Generic_32(File& document, const uint64_t offset, const Signature signature, const Enum::TaggedBlockKey key);
	};

	struct Generic_64 : Base_64
	{
		std::vector<uint8_t> m_Data;

		Generic_64(File& document, const uint64_t offset, const Signature signature, const Enum::TaggedBlockKey key);
	};


	// 16-bit files store this tagged block at the end of the layer and mask information section which contains the 
	// layer info section
	struct Lr16 : Base_64
	{
		Enum::TaggedBlockKey m_Key = Enum::TaggedBlockKey::Lr16;
		LayerInfo m_Data;

		Lr16(File& document, const FileHeader& header, const uint64_t offset, const Signature signature);
	};


	// 32-bit files store this tagged block at the end of the layer and mask information section which contains the 
	// layer info section
	struct Lr32 : Base_64
	{
		Enum::TaggedBlockKey m_Key = Enum::TaggedBlockKey::Lr32;
		LayerInfo m_Data;

		Lr32(File& document, const FileHeader& header, const uint64_t offset, const Signature signature);
	};


	// Unknown what this type is
	struct Layr : Generic_64
	{
		Layr(File& document, const uint64_t offset, const Signature signature)
			: Generic_64(document, offset, signature, Enum::TaggedBlockKey::Layr) {};
	};


	// Probably some alpha information but undocumented
	struct Alpha : Generic_64
	{
		Alpha(File& document, const uint64_t offset, const Signature signature)
			: Generic_64(document, offset, signature, Enum::TaggedBlockKey::Alph) {};
	};


	// This should just be a flag indicating that the merged data contains an alpha channel.
	// However, the length field being 8 bytes wide is a bit weird
	struct LayerSavingMergedTransparency : Generic_64
	{
		LayerSavingMergedTransparency(File& document, const uint64_t offset, const Signature signature)
			: Generic_64(document, offset, signature, Enum::TaggedBlockKey::lrSavingMergedTransparency) {};
	};


	// TODO add proper parsing
	struct LayerFilterMask : Generic_64
	{
		LayerFilterMask(File& document, const uint64_t offset, const Signature signature)
			: Generic_64(document, offset, signature, Enum::TaggedBlockKey::lrFilterMask) {};
	};


	// TODO add proper parsing
	struct LayerFilterEffects : Generic_64
	{
		LayerFilterEffects(File& document, const uint64_t offset, const Signature signature)
			: Generic_64(document, offset, signature, Enum::TaggedBlockKey::lrFilterEffects) {};
	};

	// TODO add proper parsing
	struct LinkedLayer_8Byte : Generic_64
	{
		LinkedLayer_8Byte(File& document, const uint64_t offset, const Signature signature)
			: Generic_64(document, offset, signature, Enum::TaggedBlockKey::lrLinked_8Byte) {};
	};


	// Pixel data for 3d and video layers
	struct PixelSourceData : Generic_64
	{
		PixelSourceData(File& document, const uint64_t offset, const Signature signature)
			: Generic_64(document, offset, signature, Enum::TaggedBlockKey::lrPixelSourceData) {};
	};


	// Pixel data for 3d and video layers
	struct CompositorUsed : Generic_64
	{
		CompositorUsed(File& document, const uint64_t offset, const Signature signature)
			: Generic_64(document, offset, signature, Enum::TaggedBlockKey::lrCompositorUsed) {};
	};

}

// Return a pointer to a Tagged block based on the key it reads
std::unique_ptr<TaggedBlock::Base> readTaggedBlock(File& document, const FileHeader& header);

PSAPI_NAMESPACE_END