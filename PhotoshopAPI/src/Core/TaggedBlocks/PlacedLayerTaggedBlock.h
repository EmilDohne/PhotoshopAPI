/// This header file covers both the PlacedLayerTaggedBlock as well as the PlacedLayerDataTaggedBlock for simplicity.

#pragma once

#include "Macros.h"
#include "TaggedBlock.h"

#include "Core/Struct/File.h"
#include "Core/Struct/BidirectionalMap.h"
#include "Core/Struct/DescriptorStructure.h"
#include "Core/Struct/Signature.h"
#include "Util/ProgressCallback.h"
#include "Util/Enum.h"
#include "Util/Logger.h"

PSAPI_NAMESPACE_BEGIN


namespace PlacedLayer
{
	enum class Type
	{
		Unknown = 0,
		Vector = 1,
		Raster = 2,
		ImageStack = 3
	};

	static bidirectional_unordered_map<uint32_t, Type> s_TypeMap =
	{
		{0, Type::Unknown},
		{1, Type::Vector},
		{2, Type::Raster},
		{3, Type::ImageStack}
	};

	static bidirectional_unordered_map<uint32_t, std::string> s_TypeStrMap =
	{
		{0, "Unknown"},
		{1, "Vector"},
		{2, "Raster"},
		{3, "ImageStack"}
	};

	struct Point
	{
		double x{};
		double y{};

		void read(File& document);
		void write(File& document);
	};

	struct Transform
	{
		Point topleft;
		Point topright;
		Point bottomleft;
		Point bottomright;

		void read(File& document);
		void write(File& document);
	};
}

/// Placed layer tagged blocks are the per-layer counterparts to the global LinkedLayerTaggedBlock. These hold information on
/// the uuid associated with the image data as well as transforms, warp information etc.
struct PlacedLayerTaggedBlock : TaggedBlock
{
	PascalString m_UniqueID{};
	PlacedLayer::Type m_Type{};
	PlacedLayer::Transform m_Transform{};
	Descriptors::Descriptor m_WarpInformation;

	void read(File& document, const uint64_t offset, const Enum::TaggedBlockKey key, const Signature signature);
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) override;


private:
	uint32_t m_Version = 3;
	uint32_t m_PageNumber{};
	uint32_t m_TotalPages{};
	uint32_t m_AnitAliasPolicy{};

};


/// This supposedly supersedes the PlacedLayerTaggedBlock since Photoshop CS3 but it appears that those two are always there in conjunction.
/// Likely to keep backwards compatibility
struct PlacedLayerDataTaggedBlock : TaggedBlock
{
	Descriptors::Descriptor m_Descriptor;

	PlacedLayerDataTaggedBlock() = default;
	PlacedLayerDataTaggedBlock(Descriptors::Descriptor descriptor)
		: m_Descriptor(descriptor) {}

	void read(File& document, const uint64_t offset, const Enum::TaggedBlockKey key, const Signature signature);
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) override;

private:
	uint32_t m_Version = 4;

};


PSAPI_NAMESPACE_END