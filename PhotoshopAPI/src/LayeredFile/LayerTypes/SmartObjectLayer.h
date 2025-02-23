#pragma once

#include "Macros.h"
#include "Layer.h"
#include "_ImageDataLayerType.h"

#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "PhotoshopFile/AdditionalLayerInfo.h"

#include "LayeredFile/LinkedData/LinkedLayerData.h"

#include "Core/Struct/DescriptorStructure.h"
#include "Core/Struct/TaggedBlock.h"
#include "Core/Warp/SmartObjectWarp.h"
#include "Core/Render/Render.h"
#include "Util/Enum.h"

#include <fstream>
#include <string>

#include "fmt/core.h"
#include <OpenImageIO/imageio.h>


PSAPI_NAMESPACE_BEGIN

// Forward declare LayeredFile here
template <typename T>
struct LayeredFile;


/// Smart objects are Photoshops' way of non-destructive image data edits while keeping a live link to the original file.
///
/// We expose not only ways to replace this linked image data but also have functionality to recreate and store the warps 
/// applied to these objects (with more features coming in the future). 
/// We currently support recreating all the warps found in the Edit->Transform tab. We do not yet support the `Edit->Puppet Warp`
/// and `Edit->Perspective Warp` which are stored as Smart Filters.
/// 
/// Smart objects store their original image data on the `LayeredFile<T>` while storing a decoded preview the size of the layer on
/// the layer itself. We provide multiple methods to get both the scaled and warped image data as well as the full size image 
/// data.
template <typename T>
struct SmartObjectLayer : public _ImageDataLayerType<T>
{
	using _ImageDataLayerType<T>::data_type;
	using _ImageDataLayerType<T>::storage_type;

	SmartObjectLayer() = default;
	
	/// Initialize a SmartObject layer from a filepath.
	///
	/// This will internally load the given file (assuming it exists) into memory, decoding the full resolution
	/// image data as well as generating a resampled image data based on the resolution provided in the layers'
	/// parameters. Requires the `LayeredFile` to be passed so we can keep track of this global state of 
	/// linked layer data.
	/// 
	/// \param file The LayeredFile this SmartObject is to be associated with
	/// \param parameters The Layers' parameters
	/// \param filepath The path of the file to load, this must be a file format Photoshop knows about and can decode.
	///					If `link_externally` is set to true it is highly recommended to keep this file local to the output directory.
	///					I.e. if the file gets written to `C:/PhotoshopFiles/file.psb` The file should be in `C:/PhotoshopFiles/`
	///					(same applies to linux). To learn more about how photoshop resolves these linkes head to this page:
	///					https://helpx.adobe.com/photoshop/using/create-smart-objects.html#linking_logic
	/// \param link_externally Whether to link the file externally (without saving it in the document). While this does reduce 
	///						   file size, due to linking limitations it is usually recommended to leave this at its default `false`.
	///						   If the given file already exists on the `LayeredFile<T>` e.g. when you link 2 layers with the same filepath
	///						   the settings for the first layer are used instead of overriding the behaviour
	SmartObjectLayer(const LayeredFile<T>& file, Layer<T>::Params& parameters, std::filesystem::path filepath, bool link_externally = false)
	{
		auto warp = SmartObject::Warp{};
		this->construct(file, parameters, filepath, warp);
	}

	SmartObjectLayer(const LayeredFile<T>& file, Layer<T>::Params& parameters, std::filesystem::path filepath, const SmartObject::Warp& warp)
	{
		this->construct(file, parameters, filepath, warp);
	}

	/// Generate a SmartObjectLayer from a Photoshop File object. This is for internal uses and not intended to be used by users directly. Please use the other
	/// constructors instead.
	SmartObjectLayer(const LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header, const AdditionalLayerInfo& globalAdditionalLayerInfo) 
		: _ImageDataLayerType<T>(layerRecord, channelImageData, header)
	{
		// Local and global additional layer info in this case refer to the one stored on the individual layer and the one 
		// stored on the LayerAndMaskInfo section respectively
		if (layerRecord.m_AdditionalLayerInfo)
		{
			const auto& localAdditionalLayerInfo = layerRecord.m_AdditionalLayerInfo.value();
			decode(localAdditionalLayerInfo, globalAdditionalLayerInfo, Layer<T>::m_LayerName);
			{
				auto descriptor = generate_placed_layer_data();
				// Temporarily write the descriptor json to file so we can easily debug it.
				std::ofstream file(fmt::format("C:/Users/emild/Desktop/linkedlayers/warp/placed_layer_descriptor_{}_out.json", Layer<T>::m_LayerName));
				file << descriptor.to_json().dump(4);
				file.flush();
			}
		}
		else
		{
			PSAPI_LOG_ERROR("SmartObject", "Internal Error: Expected smart object layer to contain an AdditionalLayerInfo section");
		}
	}

	/// Retrieve the warp object that is stored on this layer. 
	SmartObject::Warp& warp() noexcept { return m_SmartObjectWarp; }
	const SmartObject::Warp& warp() const noexcept{ return m_SmartObjectWarp; }

	/// Check whether the original image file stored by this smart object is linked externally
	bool linked_externally(const LayeredFile<T>& file) const
	{
		const auto& linkedlayer_ptr = file.linked_layers().at(m_Hash);
		return linkedlayer_ptr->type() == LinkedLayerType::external;
	}

	/// Set the type of linkage for the original image data.
	///
	/// This can be data or external where data is storing the raw file bytes on the 
	/// file itself while external will reference a file from disk. As this property
	/// only has an effect on-write this can be modified as many times as wanted.
	/// 
	/// As this affects not the layer directly but the shared linked layer if any other
	/// layers refer to this same file we modify that too.
	/// 
	/// \param file The file on which the LinkedLayer is stored
	/// \param linkage The type of linkage we want to set
	void set_linkage(LayeredFile<T>& file, LinkedLayerType linkage)
	{
		auto& linkedlayer_ptr = file.linked_layers().at(m_Hash);
		linkedlayer_ptr->type(linkage);
	}

	/// Retrieve the hashed value associated with the layer, this is what is used to identify the
	/// linked layer associated with this smart object (where the original image data is stored)
	const std::string& hash() const noexcept { return m_Hash; }

	/// Retrieve the filename associated with this smart object. 
	/// 
	/// Note that it is a filename, not a filepath as Photoshop doesn't store it that way. 
	/// If the layer is linked externally it is usually a good idea to have the file relative to the output 
	/// psd/psb file as otherwise photoshop won't be able to link it.
	const std::string& filename() const noexcept { return m_Filename; }

	/// Extract all the channels of the original image data.
	/// 
	/// Unlike the accessors `image_data()` and `channel()` this function gets the full resolution
	/// image data that is stored on the smart object, i.e. the original image data. This may be smaller
	/// or larger than the layers `width` or `height`. To get the actual resolution you can query: `original_width()` and `original_height()`
	/// 
	/// \tparam ExecutionPolicy the execution policy to get the image data with
	/// 
	/// \param document The document associated with this layer, this is required to query the original image data
	/// \param policy The execution policy for the image data decompression, defaults to parallel
	template <typename ExecutionPolicy = std::execution::parallel_policy, std::enable_if_t<std::is_execution_policy_v<ExecutionPolicy>, int> = 0>
	_ImageDataLayerType<T>::data_type original_image_data(const LayeredFile<T>& document, const ExecutionPolicy policy = std::execution::par)
	{
		const auto& linkedlayers = document.linked_layers();
		const auto& layer = linkedlayers.at(m_Hash);

		return layer.get_image_data();
	}


	/// Retrieve the original image datas' width.
	///
	/// This does not have the same limitation as Photoshop layers of being limited
	/// to 30,000 or 300,000 pixels depending on the file type
	/// 
	/// \throws std::runtime_error if the hash defined by `hash()` is not valid for the document
	/// 
	/// \returns The width of the original image data
	size_t original_width(const LayeredFile<T>& document) const
	{
		const auto& linkedlayers = document.linked_layers();
		const auto& layer = linkedlayers.at(m_Hash);
		return layer.width();
	}

	/// Retrieve the original image datas' height.
	///
	/// This does not have the same limitation as Photoshop layers of being limited
	/// to 30,000 or 300,000 pixels depending on the file type
	/// 
	/// \param document The document where the LinkedLayer is stored
	/// 
	/// \throws std::runtime_error if the hash defined by `hash()` is not valid for the document
	/// 
	/// \return The height of the original image data
	size_t original_height(const LayeredFile<T>& document) const
	{
		const auto& linkedlayers = document.linked_layers();
		const auto& layer = linkedlayers.at(m_Hash);
		return layer.width();
	}


private:

	/// The warp on the object, may be a no-op in the case of no warp.
	SmartObject::Warp m_SmartObjectWarp;

	/// The hash of the file, this is the same as what is stored on the LinkedLayerData
	/// and identical files are automatically de-duplicated
	std::string m_Hash;

	/// The filename that the Smart Object was constructed with.
	std::string m_Filename;

	/// The orginal width and height of the image data
	std::array<double, 2> m_OriginalSize;

	/// The overall transform applied to the smart object.
	/// While m_Width, m_Height and m_CenterX/m_CenterY describe the optimized bounds
	/// at the final applied stage while this transform signifies the transform before any
	/// warps, disregarding any transparent pixels. So while photoshop may optimize
	/// away transparent pixels this does not.
	/// Points are in order: top-left, top-right, bot-right, bot-left
	std::array<Geometry::Point2D<double>, 4> m_Transform{};

	/// Resolution in DPI
	double m_Resolution = 72.0f;

	/// Internal values for roundtripping:

	/// Hash of the layer itself, doesn't seem to relate back to the LinkedLayers
	std::string _m_LayerHash;

	int32_t _m_PageNum = 1;
	int32_t _m_NumPages = 1;

	int32_t _m_Crop = 1;	// Appears to always be 1

	int32_t _m_FrameStepNumerator = 0;
	int32_t _m_FrameStepDenominator = 600;
	int32_t _m_DurationStepNumerator = 0;
	int32_t _m_DurationStepDenominator = 600;
	int32_t _m_FrameCount = 1;

	int32_t _m_AntiAliasing = 16; // Maybe the sample radius for rescaling? I.e. 4x4 = 16 = bicubic?

	int32_t _m_Type = 2;	// Appears to always be 2

	int32_t _m_Comp = -1;
	int32_t _m_CompInfoID = -1;
	int32_t _m_CompInfoOriginalID = -1;
	

	void construct(const LayeredFile<T>& file, Layer<T>::Params& parameters, std::filesystem::path filepath, const SmartObject::Warp& warp)
	{
		PSAPI_PROFILE_FUNCTION();

		// Insert (or find) the linked layer and create a rescaled version of the image data.
		const auto& linkedlayer = file.linked_layers().insert(filepath);
		const auto& image_data = linkedlayer.get_image_data(parameters.width, parameters.height);
		std::for_each(std::execution::par, image_data.begin(), image_data.end(), [&](const auto& pair)
			{
				const auto& key = pair.first;
				const auto& channel = pair.second;
				_ImageDataLayerType<T>::m_ImageData[key] = std::make_unique<ImageChannel<T>>(
					parameters.compression,
					channel,
					key,
					parameters.width,
					parameters.height,
					parameters.center_x,
					parameters.center_y);
			});


		Layer<T>::m_ColorMode = parameters.colormode;
		Layer<T>::m_LayerName = parameters.name;
		if (parameters.blendmode == Enum::BlendMode::Passthrough)
		{
			PSAPI_LOG_WARNING("ImageLayer", "The Passthrough blend mode is reserved for groups, defaulting to 'Normal'");
			Layer<T>::m_BlendMode = Enum::BlendMode::Normal;
		}
		else
		{
			Layer<T>::m_BlendMode = parameters.blendmode;
		}
		Layer<T>::m_Opacity = parameters.opacity;
		Layer<T>::m_IsVisible = parameters.visible;
		Layer<T>::m_IsLocked = parameters.locked;
		Layer<T>::m_CenterX = parameters.center_x;
		Layer<T>::m_CenterY = parameters.center_y;
		Layer<T>::m_Width = parameters.width;
		Layer<T>::m_Height = parameters.height;
		Layer<T>::parse_mask(parameters);
	}

	
	void decode(const AdditionalLayerInfo& local, const AdditionalLayerInfo& global, const std::string& name)
	{
		// Get the LinkedLayers from the global additional layer info
		auto g_linkedLayer = global.getTaggedBlock<LinkedLayerTaggedBlock>(Enum::TaggedBlockKey::lrLinked);
		if (!g_linkedLayer)
		{
			g_linkedLayer = global.getTaggedBlock<LinkedLayerTaggedBlock>(Enum::TaggedBlockKey::lrLinked_8Byte);
		}
		const auto l_placedLayer = local.getTaggedBlock<PlacedLayerTaggedBlock>(Enum::TaggedBlockKey::lrPlaced);
		const auto l_placedLayerData = local.getTaggedBlock<PlacedLayerDataTaggedBlock>(Enum::TaggedBlockKey::lrPlacedData);

		// Prefer decoding via placed layer data as that is more up to date
		if (g_linkedLayer && l_placedLayerData)
		{
			decode_placed_layer_data(l_placedLayerData.value(), g_linkedLayer.value(), name);
		}
		// Fallback to placed layer if placed layer data wasn't present
		else if (g_linkedLayer && l_placedLayer)
		{
			// This currently throws a runtime_error. Perhaps we get around to adding
			// this in the future but most files should have PlacedLayerData anyways
			decode_placed_layer(l_placedLayer.value(), g_linkedLayer.value(), name);
		}
		else
		{
			PSAPI_LOG_ERROR("SmartObject", "Internal Error: Unable to decode SmartObject layer '%s' as we couldn't find the appropriate tagged blocks", name.c_str());
		}
	}


	/// Decode the smart object from the PlacedLayerData Tagged Block, this contains information such as 
	void decode_placed_layer_data(const std::shared_ptr<PlacedLayerDataTaggedBlock>& local, const std::shared_ptr<LinkedLayerTaggedBlock>& global, const std::string& name)
	{
		const auto& descriptor = local->m_Descriptor;
		{
			// Temporarily write the descriptor json to file so we can easily debug it.
			std::ofstream file(fmt::format("C:/Users/emild/Desktop/linkedlayers/warp/placed_layer_descriptor_{}.json", name));
			file << descriptor.to_json().dump(4);
			file.flush();
		}

		if (descriptor.contains("filterFX"))
		{
			PSAPI_LOG_WARNING("SmartObject", "Filter based warps are not supported at the moment (Edit->Puppet Warp and Edit->Perspective Warp)." \
				" These will not be represented properly in the API");
			return;
		}

		m_Hash = descriptor.at<UnicodeString>("Idnt").getString();	// The identifier that maps back to the LinkedLayer

		// These we all ignore for the time being, we store them locally and just rewrite them back out later
		// This isn't necessarily in order
		{
			_m_LayerHash = descriptor.at<UnicodeString>("placed").getString();
			_m_PageNum = descriptor.at<int32_t>("PgNm");
			_m_NumPages = descriptor.at<int32_t>("totalPages");
			_m_Crop = descriptor.at<int32_t>("Crop");

			const auto& _frame_step = descriptor.at<Descriptors::Descriptor>("frameStep");
			_m_FrameStepNumerator = _frame_step.at<int32_t>("numerator");
			_m_FrameStepDenominator = _frame_step.at<int32_t>("denominator");

			const auto& _duration = descriptor.at<Descriptors::Descriptor>("duration");
			_m_DurationStepNumerator = _duration.at<int32_t>("numerator");
			_m_DurationStepDenominator = _duration.at<int32_t>("denominator");

			_m_FrameCount = descriptor.at<int32_t>("frameCount");
			_m_AntiAliasing = descriptor.at<int32_t>("Annt");

			_m_Type = descriptor.at<int32_t>("Type");

			_m_Comp = descriptor.at<int32_t>("comp");

			const auto& _comp_info = descriptor.at<Descriptors::Descriptor>("compInfo");
			_m_CompInfoID = _comp_info.at<int32_t>("compID");
			_m_CompInfoOriginalID = _comp_info.at<int32_t>("originalCompID");
		}

		const auto& size = descriptor.at<Descriptors::Descriptor>("Sz  ");		// The spaces are not a mistake
		m_OriginalSize = { size.at<double>("Wdth"), size.at<double>("Hght") };
		const auto& resolution = descriptor.at<Descriptors::UnitFloat>("Rslt");	// In DPI
		m_Resolution = resolution.m_Value;

		const auto& transform = descriptor.at<Descriptors::List>("Trnf");
		{
			assert(transform.m_Items.size() == 8);
			auto transform_items = transform.as<double>();

			m_Transform[0].x = transform_items[0];
			m_Transform[0].y = transform_items[1];
			m_Transform[1].x = transform_items[2];
			m_Transform[1].y = transform_items[3];
			m_Transform[2].x = transform_items[4];
			m_Transform[2].y = transform_items[5];
			m_Transform[3].x = transform_items[6];
			m_Transform[3].y = transform_items[7];
		}
		const auto& non_affine_transform = descriptor.at<Descriptors::List>("nonAffineTransform");

		// The warp struct is present on all descriptors, if it is however a warp with a non-standard
		// number of subdivisions (i.e. not 4x4) the warp struct will be empty and instead we will be dealing with a quilt warp
		const auto& warp = descriptor.at<Descriptors::Descriptor>("warp");
		SmartObject::Warp warpStruct;
		
		if (descriptor.contains("quiltWarp"))
		{
			warpStruct = SmartObject::Warp::deserialize(descriptor.at<Descriptors::Descriptor>("quiltWarp"), transform, non_affine_transform, SmartObject::Warp::quilt_warp{});
			//renderMesh(warpStruct, fmt::format("C:/Users/emild/Desktop/linkedlayers/warp/warp_grid{}.jpg", name));
		}
		else
		{
			warpStruct = SmartObject::Warp::deserialize(warp, transform, non_affine_transform, SmartObject::Warp::normal_warp{});
			//renderMesh(warpStruct, fmt::format("C:/Users/emild/Desktop/linkedlayers/warp/warp_grid{}.jpg", name));
		}
		m_SmartObjectWarp = warpStruct;
		
	}


	/// Generate a PlacedLayerData descriptor from the SmartObject checking whether the layers' linked data is still valid throwing otherwise.
	Descriptors::Descriptor generate_placed_layer_data() const
	{
		Descriptors::Descriptor placed_layer("null");

		placed_layer.insert("Idnt", UnicodeString(m_Hash, 2u));
		placed_layer.insert("placed", UnicodeString(_m_LayerHash, 2u));

		placed_layer.insert("PgNm", _m_PageNum);
		placed_layer.insert("totalPages", _m_NumPages);

		placed_layer.insert("Crop", _m_Crop);
		
		auto frame_step = Descriptors::Descriptor("null");
		frame_step.insert("numerator", _m_FrameStepNumerator);
		frame_step.insert("denominator", _m_FrameStepDenominator);
		placed_layer.insert("frameStep", frame_step);

		auto duration = Descriptors::Descriptor("null");
		duration.insert("numerator", _m_DurationStepNumerator);
		duration.insert("denominator", _m_DurationStepDenominator);
		placed_layer.insert("duration", duration);

		placed_layer.insert("frameCount", _m_FrameCount);
		placed_layer.insert("Annt", _m_AntiAliasing);
		placed_layer.insert("Type", _m_Type);

		// Store the Transformation and non-affine transformation. 
		{
			auto [affine_transform, non_affine_transform] = m_SmartObjectWarp.generate_transform_descriptors(m_Transform);

			placed_layer.insert("Trnf", affine_transform);
			placed_layer.insert("nonAffineTransform", non_affine_transform);
		}

		// Store the warp, in the case of a quilt warp this would hold 2 descriptors
		// with the "warp" descriptor just being default initialized
		{
			if (m_SmartObjectWarp.warp_type() == SmartObject::Warp::WarpType::quilt)
			{
				auto quilt_descriptor = m_SmartObjectWarp.serialize();
				auto warp_descriptor = m_SmartObjectWarp.serialize_default(m_OriginalSize[0], m_OriginalSize[1]);
			
				placed_layer.insert("quiltWarp", quilt_descriptor);
				placed_layer.insert("warp", warp_descriptor);
			}
			else
			{
				auto warp_descriptor = m_SmartObjectWarp.serialize();

				placed_layer.insert("warp", warp_descriptor);
			}
		}

		{
			Descriptors::Descriptor size_descriptor("Pnt ");

			size_descriptor.insert("Wdth", m_OriginalSize[0]);
			size_descriptor.insert("Hght", m_OriginalSize[1]);

			placed_layer.insert("Sz  ", size_descriptor);
		}
		{
			Descriptors::UnitFloat resolution_descriptor(
				"Rslt",
				Descriptors::Impl::descriptorKeys.at(Descriptors::Impl::OSTypes::UnitFloat),
				Descriptors::Impl::UnitFloatType::Density,
				m_Resolution
				);

			placed_layer.insert("Rslt", resolution_descriptor);
		}

		placed_layer.insert("comp", _m_Comp);

		{
			Descriptors::Descriptor comp_info_descriptor("null");

			comp_info_descriptor.insert("compID", _m_CompInfoID);
			comp_info_descriptor.insert("originalCompID", _m_CompInfoOriginalID);

			placed_layer.insert("compInfo", comp_info_descriptor);
		}
		return placed_layer;
		
	}


	void decode_placed_layer(const std::shared_ptr<PlacedLayerTaggedBlock>& local, const std::shared_ptr<LinkedLayerTaggedBlock>& global, const std::string& name)
	{
		PSAPI_LOG_ERROR("SmartObject", "Parsing of the PlacedLayerTaggedBlock is currently unimplemented, this is likely due to trying to open an older file.");
	}


	void renderMesh(const SmartObject::Warp& warp, std::string filename)
	{
		auto bounds = warp.bounds();
		auto height = bounds.size().y;
		auto width = bounds.size().x;

		auto surface = warp.surface();
		auto subdivided_mesh = surface.mesh(100, 100, warp.non_affine_mesh());
		{
			std::vector<T> pixels(height * width, 0);
			auto buffer = Render::ImageBuffer<T>(pixels, width, height);
			
			/*Render::render_mesh<T>(
				buffer,
				Geometry::Mesh<double>(warp.m_SmartObject::WarpPoints, warp.u_dimensions(), warp.v_dimensions()),
				255,
				"C:/Windows/Fonts/arial.ttf",
				true);*/
			//Render::render_bezier_surface<T>(buffer, surface, 255, 100, 100);
			Render::render_mesh<T>(buffer, subdivided_mesh, 255);

			std::unique_ptr<OIIO::ImageOutput> out = OIIO::ImageOutput::create(filename);
			if (!out)
				return;  // error
			OIIO::ImageSpec spec(width, height, 1, OIIO::TypeDesc::UINT8);
			out->open(filename, spec);
			out->write_image(OIIO::TypeDesc::UINT8, pixels.data());
			out->close();
		}

		{
			std::vector<T> pixels(height * width, 0);
			auto buffer = Render::ImageBuffer<T>(pixels, width, height);

			// Gradient buffer for left to right (U coordinate)
			std::vector<T> u_coordinate_buffer(height * width);
			size_t cellSize = 200;
			for (size_t y = 0; y < height; ++y)
			{
				for (size_t x = 0; x < width; ++x)
				{
					// Determine the checkerboard color
					bool isBlack = (x / cellSize + y / cellSize) % 2 == 0;
					u_coordinate_buffer[y * width + x] = static_cast<T>(isBlack ? 255 : 0);
				}
			}
			const auto u_buffer = Render::ImageBuffer<T, true>(u_coordinate_buffer, width, height);
			warp.apply<T>(buffer, u_buffer, subdivided_mesh);

			{
				std::string filename_u = "C:/Users/emild/Desktop/linkedlayers/warp/warp_grid_u.jpg";
				std::unique_ptr<OIIO::ImageOutput> out = OIIO::ImageOutput::create(filename_u);
				if (!out)
					return;  // error
				OIIO::ImageSpec spec(width, height, 1, OIIO::TypeDesc::UINT8);
				out->open(filename_u, spec);
				out->write_image(OIIO::TypeDesc::UINT8, pixels.data());
				out->close();
			}
		}
		{
			std::vector<T> pixels(height * width, 0);
			auto buffer = Render::ImageBuffer<T>(pixels, width, height);

			// Gradient buffer for top to bottom (V coordinate)
			std::vector<T> v_coordinate_buffer(height * width);
			for (size_t y = 0; y < height; ++y) {
				for (size_t x = 0; x < width; ++x) {
					// Linear interpolation for gradient
					v_coordinate_buffer[y * width + x] = static_cast<T>((255 * y) / (height - 1));
				}
			}

			const auto v_buffer = Render::ImageBuffer<T, true>(v_coordinate_buffer, width, height);
			warp.apply<T>(buffer, v_buffer, subdivided_mesh);
			{
				std::string filename_v = "C:/Users/emild/Desktop/linkedlayers/warp/warp_grid_v.jpg";
				std::unique_ptr<OIIO::ImageOutput> out = OIIO::ImageOutput::create(filename_v);
				if (!out)
					return;  // error
				OIIO::ImageSpec spec(width, height, 1, OIIO::TypeDesc::UINT8);
				out->open(filename_v, spec);
				out->write_image(OIIO::TypeDesc::UINT8, pixels.data());
				out->close();
			}
		}

	}

};


PSAPI_NAMESPACE_END