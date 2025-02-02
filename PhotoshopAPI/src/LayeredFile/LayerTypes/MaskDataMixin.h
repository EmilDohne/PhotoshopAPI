#include "Macros.h"

#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "PhotoshopFile/AdditionalLayerInfo.h"

#include "Core/Struct/ImageChannel.h"
#include "Core/Geometry/BoundingBox.h"

#include <unordered_map>
#include <vector>
#include <span>

#include <fmt/format.h>

PSAPI_NAMESPACE_BEGIN


/// \brief A mixin struct for handling mask information on layers
/// 
/// This struct provides a standardized way to manage mask channels, including
/// storing, retrieving and setting mask data. It is designed to be inherited by layer
/// classes that require mask storage and processing (Any layer inheriting from Layer<T>).
/// 
/// The struct is entirely standalone and classes do not need to implement any functions.
/// When additionally subclassing from `ImageDataMixin<T>` however one must be wary to account
/// for the presence of mask channels
/// 
/// \tparam T The data type used for pixel values (e.g., uint8_t, uint16_t, float).
template <typename T>
struct MaskMixin
{
	using channel_type = std::unique_ptr<ImageChannel>;

	/// Colormode independent mask index as Enum::ChannelIDInfo that may be used 
	static constexpr auto s_mask_index = Enum::ChannelIDInfo{ Enum::ChannelID::RealUserSuppliedLayerMask, -2 };

	MaskMixin() = default;
	MaskMixin(
		channel_type data,
		bool relative_to_layer = false,
		bool disabled = false,
		uint8_t default_color = 255,
		std::optional<uint8_t> density = std::nullopt,
		std::optional<uint8_t> feather = std::nullopt
	)
	{
		m_MaskData = std::move(data);
		m_MaskRelativeToLayer = relative_to_layer;
		m_MaskDisabled = disabled;
		m_MaskDefaultColor = default_color;
		m_MaskDensity = density;
		m_MaskFeather = feather;
	}


	/** @name Masks
	 */
	 //@{

	 /// Checks whether the layer has a pixel mask.
	 /// 
	 /// \returns `true` if a mask is present, otherwise `false`.
	bool has_mask() const noexcept { return m_MaskData.has_value() && m_MaskData.value(); }

	/// Retrieves the mask channel data, if present.
	/// 
	/// If the layer does not have a mask, this function returns an empty vector.  
	/// The returned vector contains `mask_width() * mask_height()` elements.
	std::vector<T> get_mask() const
	{
		if (this->has_mask())
		{
			return m_MaskData.value()->template getData<T>();
		}
		PSAPI_LOG_WARNING("Mask", "No mask channel exists on the layer, get_mask() will return an empty channel");
		return std::vector<T>();
	}

	/// Fills a preallocated buffer with the mask channel data, if present.
	/// 
	/// If no mask is present, the buffer remains unchanged.
	/// 
	/// \param buffer A preallocated buffer expected to have exactly `mask_width() * mask_height()` elements.
	void get_mask(std::span<T> buffer) const
	{
		if (this->has_mask())
		{
			return m_MaskData.value()->template getData<T>(buffer);
		}
		PSAPI_LOG_WARNING("Mask", "No mask channel exists on the layer, get_mask() will return an empty channel");
	}

	/// Sets the layer's mask to the given buffer.
	/// 
	/// If no mask was previously held the inserted mask will be at the top-left of the canvas.
	/// Use `mask_position` to adjust this.
	/// 
	/// \param buffer The image data for the mask, provided in scanline order.
	/// \param width  The width of the new mask.
	/// \param height The height of the new mask.
	/// 
	/// \throws std::invalid_argument If the buffer size does not match the expected dimensions.
	void set_mask(std::span<const T> buffer, size_t width, size_t height)
	{
		float center_x = static_cast<float>(width) / 2;
		float center_y = static_cast<float>(height) / 2;
		if (this->has_mask())
		{
			center_x = this->m_MaskData.value()->getCenterX();
			center_y = this->m_MaskData.value()->getCenterY();
		}

		if (buffer.size() != width_val * height_val)
		{
			throw std::invalid_argument(
				fmt::format("Invalid data size encountered while calling set_mask(), expected <{}x{} = {:L}> but instead got <{:L}>"),
				width_val, height_val, width_val * height_val, buffer.size());
		}

		auto channel = std::make_unique<ImageChannel>(Enum::Compression::ZipPrediction, data, this->mask_index, width_val, height_val, center_x, center_y);
		m_MaskData.emplace(std::move(channel));
	}

	/// Sets the layer's mask to the given buffer.
	/// 
	/// If the layer previously had a mask, the new mask will be centered at the same position.  
	/// If no mask was previously present, the new mask will be centered on the document.
	/// 
	/// \param document The file to which this mask layer belongs.
	/// \param buffer The image data for the mask, provided in scanline order.
	///               If `width` and `height` are unspecified, the buffer must have exactly `mask_width() * mask_height()` elements.
	///               If no dimensions are specified and the layer previously had no mask, an exception is thrown.
	/// \param width (Optional) The width of the new mask. Required if replacing with a differently sized mask.
	/// \param height (Optional) The height of the new mask. Required if replacing with a differently sized mask.
	/// 
	/// \throws std::invalid_argument If the buffer size does not match the expected dimensions.
	/// \throws std::runtime_error If no mask existed and no explicit dimensions were provided.
	void set_mask(const LayeredFile<T>& document, std::span<const T> buffer, std::optional<size_t> width = std::nullopt, std::optional<size_t> height = std::nullopt)
	{
		if (width ^ height)
		{
			PSAPI_LOG_WARNING("Mask",
				"Passed only width or height but not both to set_mask(). Will ignore this argument and instead try and deduce the dimensions from the previously held value."
			);
		}

		// Either center around the document or pick up the previous center
		auto [center_x, center_y] = document.bbox().center();
		if (this->has_mask())
		{
			center_x = this->m_MaskData.value()->getCenterX();
			center_y = this->m_MaskData.value()->getCenterY();
		}

		// Deduce dimensions from passed width and height
		if (width && height)
		{
			auto width_val = width.value();
			auto height_val = height.value();
			if (buffer.size() != width_val * height_val)
			{
				throw std::invalid_argument(
					fmt::format("Invalid data size encountered while calling set_mask(), expected <{}x{} = {:L}> but instead got <{:L}>"),
					width_val, height_val, width_val * height_val, buffer.size());
			}

			auto channel = std::make_unique<ImageChannel>(Enum::Compression::ZipPrediction, data, this->mask_index, width_val, height_val, center_x, center_y);
			m_MaskData.emplace(std::move(channel));
		}
		// Deduce dimensions from previously held width and height
		else
		{
			if (!this->has_mask())
			{
				throw std::runtime_error(
					"When calling set_mask() without an explicit width or height it is assumed that the layer previously held a mask to deduce these from."\
					" This was not the case, please provide an explicit width and height"
				);
			}

			const size_t width_val = m_MaskData.value()->getWidth();
			const size_t height_val = m_MaskData.value()->getHeight();
			if (buffer.size() != width_val * height_val)
			{
				throw std::invalid_argument(
					fmt::format("Invalid data size encountered while calling set_mask(), expected <{}x{} = {:L}> but instead got <{:L}>"),
					width_val, height_val, width_val * height_val, buffer.size());
			}

			auto channel = std::make_unique<ImageChannel>(Enum::Compression::ZipPrediction, data, this->mask_index, width_val, height_val, center_x, center_y);
			m_MaskData.emplace(std::move(channel));
		}
	}

	/// \brief Set the masks write compression
	///
	/// If `has_mask()` evaluates to false this is a no-op.
	/// 
	/// \param _compcode The compression codec to apply on-write.
	void set_mask_compression(Enum::Compression _compcode) noexcept
	{
		if (this->has_mask())
		{
			this->m_MaskData.value()->m_Compression = _compcode;
		}
	}

	/// Retrieves the bounding box of the mask, if present.
	/// 
	/// If no mask exists, this function returns a zero-sized bounding box.
	/// 
	/// \returns The mask's bounding box, or an empty bounding box if no mask is present.
	Geometry::BoundingBox<double> mask_bbox() const
	{
		if (this->has_mask())
		{
			auto bbox = Geometry::BoundingBox<double>(
				Geometry::Point2D<double>(0, 0),
				Geometry::Point2D<double>(this->m_MaskData.value()->getWidth(), this->m_MaskData.value()->getHeight())
			);
			bbox.offset(this->mask_position());
			return bbox;
		}
		return Geometry::BoundingBox<double>{};
	}

	/// Retrieves the position of the mask, defined as its center.
	/// 
	/// If no mask is present, this function returns `{-1.0, -1.0}`.
	/// 
	/// \returns The center position of the mask, or `{-1.0, -1.0}` if no mask exists.
	Geometry::Point2D<double> mask_position() const
	{
		if (this->has_mask())
		{
			return Geometry::Point2D<double>(m_MaskData.value()->getCenterX(), m_MaskData.value()->getCenterY());
		}
		return Geometry::Point2D<double>(-1.0f, -1.0f);
	}

	/// Sets the center position of the mask.
	/// 
	/// If no mask is present, this function does nothing.
	/// 
	/// \param position The new center position of the mask.
	void mask_position(Geometry::Point2D<double> position)
	{
		if (this->has_mask())
		{
			m_MaskData.value()->setCenterX(position.x);
			m_MaskData.value()->setCenterX(position.y);
		}
	}

	/// Checks whether the mask is relative to the layer.
	/// 
	/// \returns `true` if the mask is relative to the layer, otherwise `false`.
	bool mask_relative_to_layer() { return m_MaskRelativeToLayer; }

	/// Sets whether the mask should be relative to the layer.
	/// 
	/// \param value `true` to make the mask relative to the layer, otherwise `false`.
	void mask_relative_to_layer(bool value) { m_MaskRelativeToLayer = value; }

	/// Checks whether the mask is disabled.
	/// 
	/// \returns `true` if the mask is disabled, otherwise `false`.
	bool mask_disabled() { return m_MaskDisabled; }

	/// Enables or disables the mask.
	/// 
	/// \param value `true` to disable the mask, otherwise `false`.
	void mask_disabled(bool value) { m_MaskDisabled = value; }

	/// Retrieves the mask's default fill color.
	/// 
	/// \returns The default mask color.
	uint8_t mask_default_color() { return m_MaskDefaultColor; }

	/// Sets the mask's default fill color.
	/// 
	/// \param value The new default color value.
	void mask_default_color(uint8_t value) { m_MaskDefaultColor = value; }

	/// Retrieves the mask density, if specified.
	/// 
	/// \returns The mask density value, or an empty optional if unspecified.
	std::optional<uint8_t> mask_density() { return m_MaskDensity; }

	/// Sets the mask density.
	/// 
	/// \param value The new mask density.
	void mask_density(uint8_t value) { m_MaskDensity.emplace(value); }

	/// Sets the mask density, allowing for removal.
	/// 
	/// \param value An optional new mask density.
	void mask_density(std::optional<uint8_t> value) { m_MaskDensity = value; }

	/// Retrieves the mask feathering amount, if specified.
	/// 
	/// \returns The mask feather value, or an empty optional if unspecified.
	std::optional<float64_t> mask_feather() { return m_MaskFeather; }

	/// Sets the mask feathering amount.
	/// 
	/// \param value The new mask feather value.
	void mask_feather(float64_t value) { m_MaskFeather.emplace(value); }
	void mask_feather(std::optional<float64_t> value) { m_MaskFeather = value; }

	//@}

protected:

	/// \brief Generates the LayerMaskData struct from the layer mask (if provided).
	/// 
	/// Part of the internal API, not expected to be used by users.
	///
	/// \return An optional containing LayerMaskData if a layer mask is present; otherwise, std::nullopt.
	std::optional<LayerRecords::LayerMaskData> internal_generate_mask_data()
	{
		auto mask_data = LayerRecords::LayerMaskData();

		// We dont have support for vector masks so far so we do not consider them
		mask_data.m_VectorMask = std::nullopt;
		if (!this->has_mask())
		{
			mask_data.m_LayerMask = std::nullopt;
		}
		else
		{
			LayerRecords::LayerMask lr_mask = LayerRecords::LayerMask{};

			float center_x = this->mask_position().x;
			float center_y = this->mask_position().y;
			int32_t width = static_cast<int32_t>(this->mask_bbox().width());
			int32_t height = static_cast<int32_t>(this->mask_bbox().height());
			ChannelExtents extents = generate_extents(ChannelCoordinates(width, height, center_x, center_y));
			lr_mask.addSize(16u);

			// Default color
			lr_mask.addSize(1u);

			// This is the size for the mask bitflags
			lr_mask.addSize(1u);
			// This is the size for the mask parameters
			lr_mask.addSize(1u);
			bool hasMaskDensity = m_LayerMask.value().density.has_value();
			uint8_t maskDensity = 0u;
			if (hasMaskDensity)
			{
				maskDensity = m_LayerMask.value().density.value();
				lr_mask.addSize(1u);
			}

			bool hasMaskFeather = m_LayerMask.value().feather.has_value();
			float64_t maskFeather = 0.0f;
			if (hasMaskFeather)
			{
				maskFeather = m_LayerMask.value().feather.value();
				lr_mask.addSize(8u);

			}

			lr_mask.m_Top = extents.top;
			lr_mask.m_Left = extents.left;
			lr_mask.m_Bottom = extents.bottom;
			lr_mask.m_Right = extents.right;

			lr_mask.m_DefaultColor = this->mask_default_color();
			lr_mask.m_Disabled = this->mask_disabled();
			lr_mask.m_PositionRelativeToLayer = this->mask_relative_to_layer();

			lr_mask.m_HasMaskParams = this->mask_feather().has_value() || this->mask_density().has_value();
			lr_mask.m_HasUserMaskDensity = this->mask_density().has_value();
			lr_mask.m_HasUserMaskFeather = this->mask_feather().has_value();

			lr_mask.m_UserMaskDensity = this->mask_density();
			lr_mask.m_UserMaskFeather = this->mask_feather();

			mask_data.m_LayerMask.emplace(std::move(lr_mask));
		}

		if (!mask_data.m_LayerMask.has_value() && !mask_data.m_VectorMask.has_value())
		{
			return std::nullopt;
		}
		return std::make_optional(mask_data);
	}

	/// \brief Extract the layer mask into a tuple of channel information and image data
	///
	/// Part of the internal API, not expected to be used by users.
	/// 
	/// \return An optional containing a tuple of ChannelInformation and a unique_ptr to ImageChannel.
	std::optional<std::tuple<LayerRecords::ChannelInformation, std::unique_ptr<ImageChannel>>> internal_extract_mask()
	{
		if (!this->has_mask())
		{
			return std::nullopt;
		}

		std::unique_ptr<ImageChannel> mask_channel = std::move(m_MaskData.value());
		LayerRecords::ChannelInformation channel_info{ this->s_mask_index, mask_channel->m_OrigByteSize };

		auto data = std::make_tuple(channel_info, std::move(mask_channel));
		return std::make_optional(std::move(data));
	}


private:
	/// The optional mask data associated with this layer.
	/// If no mask is present, this will be `std::nullopt`.
	std::optional<channel_type> m_MaskData = std::nullopt;

	/// Whether the mask is positioned relative to the layer.
	/// If `true`, the mask moves with the layer; otherwise, it's positioned independently.
	bool m_MaskRelativeToLayer = false;

	/// Whether the mask is disabled.
	/// If `true`, the mask has no effect on the layer.
	bool m_MaskDisabled = false;

	/// The default fill color for the mask when created.
	/// Typically `255` (white) for a fully visible mask.
	uint8_t m_MaskDefaultColor = 255;

	/// The density (opacity) of the mask, from `0` (fully transparent) to `255` (fully opaque).
	/// If `std::nullopt`, the default mask density is used.
	std::optional<uint8_t> m_MaskDensity = std::nullopt;

	/// The feathering amount applied to the mask edges, in pixels.
	/// If `std::nullopt`, no feathering is applied.
	std::optional<float64_t> m_MaskFeather = std::nullopt;
};

extern template struct MaskMixin<bpp8_t>;
extern template struct MaskMixin<bpp16_t>;
extern template struct MaskMixin<bpp32_t>;

PSAPI_NAMESPACE_END