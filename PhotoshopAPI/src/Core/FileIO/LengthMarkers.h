#pragma once

#include "Macros.h"

#include "Core/Struct/File.h"
#include "PhotoshopFile/FileHeader.h"

#include <type_traits>
#include <optional>

PSAPI_NAMESPACE_BEGIN

namespace Impl
{

	/// Constexpr variadic size for psd and psb which have different size requirements.
	/// 
	/// Example construction would be like this: `VariadicSize<uint32_t, uint64_t>`.
	/// Keep in mind that only some classes support this type. Only use this if its explicitly mentioned.
	template <typename psd, typename psb>
	struct VariadicSize
	{
		using psd_t = psd;
		using psb_t = psb;

		constexpr static std::size_t narrow = sizeof(psd_t);
		constexpr static std::size_t wide	= sizeof(psb_t);
	};

	template <typename T, typename = void>
	struct IsVariadicSize : std::false_type {};

	template <typename T>
	struct IsVariadicSize<T, std::void_t<typename T::psd_t, typename T::psb_t>>
		: std::is_same<T, VariadicSize<typename T::psd_t, typename T::psb_t>> {};

	template <typename T>
	constexpr bool is_variadic_size_v = IsVariadicSize<T>::value;

	
	/// Scoped length block that should be initialized where the length marker gets written instead of any write operation
	/// and on destruction will then write the length block with the actual size that was written to (plus any padding).
	/// Additionally the start may be overridded which is helpful for e.g. tagged blocks where there is first 8 bytes of 
	/// other information but the length block is intended to include that as well.
	/// The Length marker written will be constructed as if started at that position so it includes the number of bytes written
	/// by the length marker itself.
	/// 
	/// An example of how one could use the ScopedLengthBlock:
	///		
	///		void write_some_data(File& document)
	///		{
	///			ScopedLengthBlock length_block(document, 2u);
	/// 
	///			// Do some write operations here, may be as long as you want
	///			// ...
	/// 
	///			// ~ScopedLengthBlock is called, will rewind back to the place the ScopedLengthBlock was initialized
	///			// and write the length block (padded to the current amount). After which it rewinds back to where the destructor 
	///			// was called and writes the padding bytes.
	///		}
	/// 
	/// If you wish to additionally have the length block write in position a but the count to start at position b the 
	/// `start_count_override` argument handles this. As an example if you have the following structure:
	/// 
	/// uint32_t type
	/// uint32_t signature
	/// uint32_t length block	<-- This should include the above positions
	/// 
	/// You can use the ScopedLengthBlock as follows:
	/// 
	///		WriteBinaryData(document, type);
	///		WriteBinaryData(document, signature);
	///		ScopedLengthBlock(document, 2u, document.get_offset() - 8u);	// Subtract the size of the two previous blocks
	/// 
	///		// Lets say here we now write an additional 24 bytes of data. The total size of the ScopedLengthBlock written
	///		// will be 24 + 8 + 4 bytes = 36 bytes.
	///		
	/// An additional use case is when trying to write a variadic size marker (where e.g. psd is uint32_t and psd is uint64_t).
	/// This can be done by initializing the ScopedLengthBlock with T = VariadicSize like this:
	/// 
	///		ScopedLengthBlock<VariadicSize<uint32_t, uint64_t>> length_block(document, header, 2u);
	/// 
	/// Note that the header needs to be passed as the destructor needs to know what the header type is to write
	/// the appropriate type.
	template <typename T> requires std::is_integral_v<T> || std::is_same_v<T, VariadicSize<typename T::psd_t, typename T::psb_t>>
	class ScopedLengthBlock
	{
	public:

		/// Initialize the ScopedLengthBlock at the given position. This is not thread safe.
		///
		/// Specialization for any integral types.
		/// 
		/// \param document 
		///		The file to write to, this file must remain valid for the entire scope of this class as we hold an internal
		///		reference to it for the destructor.
		/// \param padding
		///		The number to pad (align) the section to.
		/// \param include_marker
		///		Whether to include the size of the marker in the length block, defaults to `false`
		/// \param start_count_override
		///		A logical file position from where the counting is supposed to happen. If e.g. the length marker is only at
		///		position 8 of the section this should be `document.get_offset() - 8`. Defaults to `std::nullopt`
		ScopedLengthBlock(
			File& document, 
			std::size_t padding, 
			bool include_marker = false,
			std::optional<std::size_t> start_count_override = std::nullopt
		) requires std::is_integral_v<T>
		{
			m_Document = &document;
			m_Padding = padding;

			m_IncludeMarkerSize = include_marker;

			m_StartOffset = document.get_offset();
			m_CountOffset = start_count_override.value_or(m_StartOffset);

			// Write the explicit length field so any further write operations have the 
			// right offset.
			WriteBinaryData<T>(document, static_cast<T>(0));
		}


		/// Initialize the ScopedLengthBlock at the given position. This is not thread safe.
		///
		/// Specialization for the VariadicSize template type writing a variadic number of bytes
		/// 
		/// \param document 
		///		The file to write to, this file must remain valid for the entire scope of this class as we hold an internal
		///		reference to it for the destructor.
		/// \param header
		///		The header, decides on the type written by VariadicSize.
		/// \param padding
		///		The number to pad (align) the section to.
		/// \param include_marker
		///		Whether to include the size of the marker in the length block, defaults to `false`
		/// \param start_count_override
		///		A logical file position from where the counting is supposed to happen. If e.g. the length marker is only at
		///		position 8 of the section this should be `document.get_offset() - 8`.
		ScopedLengthBlock(
			File& document, 
			FileHeader header, 
			std::size_t padding, 
			bool include_marker = false,
			std::optional<std::size_t> start_count_override = std::nullopt
		) requires std::is_same_v<T, VariadicSize<typename T::psd_t, typename T::psb_t>>
		{
			m_Document = &document;
			m_FileHeader.emplace(header);

			m_Padding = padding;
			m_IncludeMarkerSize = include_marker;

			m_StartOffset = document.get_offset();
			m_CountOffset = start_count_override.value_or(m_StartOffset);

			// Write the explicit length field so any further write operations have the 
			// right offset.
			if (header.m_Version == Enum::Version::Psd)
			{
				WriteBinaryData<typename T::psd_t>(document, static_cast<typename T::psd_t>(0));
			}
			else
			{
				WriteBinaryData<typename T::psb_t>(document, static_cast<typename T::psb_t>(0));
			}
		}

		/// Destruct the ScopedLengthBlock. This is where the main logic lives to compute the 
		/// size of the block and write it as the appropriate type.
		~ScopedLengthBlock()
		{
			if (m_Document->get_offset() < m_StartOffset)
			{
				PSAPI_LOG_WARNING(
					"ScopedLengthBlock", "Tried to write a length marker while the document's position is before where the"  
					" ScopedLengthBlock was initialized. This would lead to a negative length block which is not allowed." 
					" Please ensure you are not skipping backwards in the file. The written file will be unusable"
				);
				return;
			}

			// Pad the section
			std::size_t size = m_Document->get_offset() - m_CountOffset;
			WritePadddingBytes(*m_Document, RoundUpToMultiple<std::size_t>(size, m_Padding) - size);
			size = m_Document->get_offset() - m_CountOffset;


			// Store the offset we are at so we can set it back to it after.
			std::size_t end_offset = m_Document->get_offset();

			// Set our internal marker to the length block and write the padded length.
			// add an extra specialization for variadic types.
			m_Document->set_offset(m_StartOffset);
			if constexpr (is_variadic_size_v<T>)
			{
				if (!m_FileHeader.has_value())
				{
					PSAPI_LOG_WARNING("ScopedLengthBlock",
						"Variadic size initialization but no header provided. Aborting writing of length block and the file will be unusable.");
					return;
				}
				const FileHeader header = m_FileHeader.value();

				if (header.m_Version == Enum::Version::Psd)
				{
					using psd_t = typename T::psd_t;
					size = handle_size_adjustment<psd_t>(size, m_IncludeMarkerSize);
					WriteBinaryData<psd_t>(*m_Document, static_cast<psd_t>(size));
				}
				else
				{
					using psb_t = typename T::psb_t;
					size = handle_size_adjustment<psb_t>(size, m_IncludeMarkerSize);
					WriteBinaryData<psb_t>(*m_Document, static_cast<psb_t>(size));
				}
			}
			else
			{
				size = handle_size_adjustment<T>(size, m_IncludeMarkerSize);
				WriteBinaryData<T>(*m_Document, static_cast<T>(size));
			}

			// Set the offset back to the end offset and finish.
			m_Document->set_offset(end_offset);
		}

	private:
		// An optional file header, required when instantiated with VariadicSize<psd, psb>
		std::optional<FileHeader> m_FileHeader = std::nullopt;

		// The document we are writing into
		File* m_Document;

		// The start offset at which we will write the length marker
		std::size_t m_StartOffset;
		// The offset at which we will start counting the length blocks size.
		// Does not have to be equal to m_StartOffset but it may.
		std::size_t m_CountOffset;

		// The padding bytes to apply.
		std::size_t m_Padding;

		// Whether to include the size of the marker into the calculation. 
		// To better illustrate this, if this is true the following is the case:
		// 
		//	uint32_t marker	<-- Will be sizeof(uint32_t) + sizeof(float) * 32
		//	float[32] data
		//
		// Otherwise the following is true:
		// 
		//  uint32_t marker	<-- Will be sizeof(float) * 32
		//	float[32] data
		//
		bool m_IncludeMarkerSize;

	private:

		// Helper function for size adjustment and bounds checking
		template <typename SizeType>
		std::size_t handle_size_adjustment(std::size_t size, bool include_marker) const 
		{
			if (!include_marker) 
			{
				size -= sizeof(SizeType);
			}

			if (size > std::numeric_limits<SizeType>::max()) 
			{
				throw std::overflow_error("size would overflow max value of SizeType, aborting.");
			}
			return size;
		}
	};


	/// Write a length block that is either 4- or 8-bytes by simply subtracting the end and start offset
	/// and re-writing the length block at the given offset. If the size does not match the padding we insert padding bytes and write those too
	template <typename T> requires std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>
	void writeLengthBlock(File& document, size_t lenBlockOffset, size_t endOffset, size_t padding)
	{
		if (endOffset < lenBlockOffset)
		{
			PSAPI_LOG_ERROR("TaggedBlock", "Internal Error: Unable to write length block as end offset is supposedly before the length block");
		}

		auto size = endOffset - lenBlockOffset;
		WritePadddingBytes(document, size % padding);
		size += size % padding;
		endOffset = document.getOffset();


		if (size > std::numeric_limits<T>::max())
		{
			PSAPI_LOG_ERROR("TaggedBlock",
				"Unable to write out length block as its size would exceed the size of the numeric limits of T, can at most write %zu bytes but tried to write %zu bytes instead",
				static_cast<size_t>(std::numeric_limits<T>::max()), size);
		}

		document.setOffset(lenBlockOffset);
		WriteBinaryData<T>(document, static_cast<T>(size));
		document.setOffset(endOffset);
	}

}

PSAPI_NAMESPACE_END