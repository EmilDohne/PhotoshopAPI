#include "LayeredFile/LayerTypes/ImageLayer.h"
#include "LayeredFile/LayerTypes/Layer.h"
#include "Util/Enum.h"
#include "PyUtil/ImageConversion.h"
#include "PyUtil/Transformation.h"
#include "Implementation/ImageDataLayerType.h"
#include "Macros.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include <pybind11/iostream.h>

#include <unordered_map>
#include <iostream>
#include <vector>

#include <fmt/format.h>

// If we compile with C++<20 we replace the stdlib implementation with the compatibility
// library
#if (__cplusplus < 202002L)
#include "tcb_span.hpp"
#else
#include <span>
#endif

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;



// Generate a LayeredFile python class from our struct adjusting some
// of the methods 
template <typename T>
void declare_smart_object_layer(py::module& m, const std::string& extension) 
{
    using Class = SmartObjectLayer<T>;
    std::string className = "SmartObjectLayer" + extension;
    py::class_<Class, _ImageDataLayerType<T>, std::shared_ptr<Class>> smart_object_layer(m, className.c_str(), py::dynamic_attr(), py::buffer_protocol());

    smart_object_layer.doc() = R"pbdoc(

        Smart objects are Photoshops' way of non-destructive image data edits while keeping a live link to the original file.
        
        We expose not only ways to replace this linked image data but also have functionality to recreate and store the warps 
        applied to these objects (with more features coming in the future). 
        We currently support recreating all the warps found in the Edit->Transform tab. We do not yet support the `Edit->Puppet Warp`
        and `Edit->Perspective Warp` which are stored as Smart Filters.
        
        Smart objects store their original image data on the `LayeredFile` while storing a decoded preview the size of the layer on
        the layer itself. We provide multiple methods to get both the scaled and warped image data as well as the full size image 
        data.
        
        Image Data:
        ------------
        
            Due to how SmartObjects work, image data is read-only and all write methods will raise an exception if you try to access them.
            In order to modify the underlying image data you should use the `replace()` method which will actually replace the underlying 
            file the smart object is linked to.
        
            Getting the image data can be done via the `get_image_data()`, `get_channel()` and `original_image_data()` functions. 
            These will retrieve the transformed and warped image data. If you modify these you can requery these functions and 
            get up to date image data.
        
        Transformations:
        -----------------
        
            Unlike normal layers, SmartObjects have slightly different transformation rules. As they link back to a file in memory or on disk
            the transformations are stored 'live' and can be modified without negatively impacting the quality of the image. We expose a variety
            of transformation options to allow you to express this freedom. 
        
            Since we have both the original image data, and the rescaled image data to worry about there is two different widths and heights available:
        
            - `original_width()` / `original_height()`
        	    These represent the resolution of the original file image data, irrespective of what transforms are applied to it.
        	    If you are e.g. loading a 4000x2000 jpeg these will return 4000 and 2000 respectively. These values may not be written to
        
            - `width()` / `height()`
        	    These represent the final dimensions of the SmartObject with the warp and any transformations applied to it. 
        
            For actually transforming the layer we expose the following methods:
        
            - `move()`
            - `rotate()`
            - `scale()`
            - `transform()`
        
            These are all individually documented and abstract away the underlying implementation of these operations. 
            You likely will not have to dive deeper than these.
        
        Warp:
        -----------
        
            Smart objects can also store warps which we implement using the `SmartObjectWarp` structure. These warps are stored as bezier surfaces with transformations applied on top of them.
            The transformations should be disregarded by the user as we provide easier functions on the SmartObjectLayer directly (see above). The warp itself is stored as a bezier
            surface. You may transfer these warps from one layer to another, modify them (although this requires knowledge of how bezier surfaces work), or clear them entirely.
        
            For the latter we provide the reset_transform()` and `reset_warp()` functions.
    
        Attributes
        -----------

        warp : SmartObjectWarp
            Property holding the warp (and transformation) information. May be modified,
            although for transforming the layer it is recommended to use the transformation
            functions such as `move`, `rotate`, `scale` and `transform`.
        linkage : psapi.enum.LinkedLayerType
            The linkage of the backing image file, if this is set to `psapi.enum.LinkedLayerType.data` 
            the image is stored in the file while if it is set to `psapi.enum.LinkedLayerType.external`
            it links to the file on disk and only stores the transformed image on file.
        image_data : dict[int, numpy.ndarray]
            Read-only property: A dictionary of the image data mapped by int.
            Accessing this will load all the image data into memory so use it sparingly and 
            instead try using the num_channels or channels properties.
        num_channels: int
            Read-only property: The number of channels held by image_data
        channels: list[int]
            Read-only property: The channel indices held by this image layer. 
            Unlike accessing image_data this does not extract the image data and is therefore
            near-zero cost.
        name : str
            The name of the layer, cannot be longer than 255
        layer_mask : LayerMask_*bit
            The pixel mask applied to the layer
        blend_mode : enum.BlendMode
            The blend mode of the layer, 'Passthrough' is reserved for group layers
        opacity : int
            The layers opacity from 0-255 with 255 being 100%
        width : int
            The width of the layer ranging up to 30,000 for PSD and 300,000 for PSB,
            this does not have to match the files width
        height : int
            The height of the layer ranging up to 30,000 for PSD and 300,000 for PSB,
            this does not have to match the files height
        center_x : float
            The center of the layer in regards to the canvas, a layer at center_x = 0 is
            perfectly centered around the document
        center_y : float
            The center of the layer in regards to the canvas, a layer at center_y = 0 is
            perfectly centered around the document
        is_locked: bool
            The locked state of the layer, this locks all pixel channels
        is_visible: bool
            Whether the layer is visible
        
    )pbdoc";

    // Constructors
    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------

    smartObjectLayer.def(py::init([](
        LayeredFile<T>& layered_file,
        std::string path,
        std::string layer_name,
        LinkedLayerType link_type,
        std::optional<SmartObject::Warp> warp,
        std::optional<py::array_t<T>> layer_mask,
        Enum::BlendMode blend_mode,
        int opacity,
        Enum::Compression compression,
        Enum::ColorMode color_mode,
        bool is_visible,
        bool is_locked
        ) {
            Layer<T>::Params params;

			if (layer_name.size() > 255)
			{
				throw py::value_error("layer_name parameter cannot exceed a length of 255");
			}
            if (opacity < 0 || opacity > 255)
            {
                throw py::value_error(fmt::format("opacity parameter must be between 0 and 255, instead got {}", opacity));
            }

            // Decode the layer mask.
			if (layer_mask)
			{
                auto& mask = layer_mask.value();
                auto& shape = Util::Impl::shape_from_py_array<T>(mask, { 2 }, mask.size());

                if (shape.size() != 2)
                {
                    throw py::value_error(
                        fmt::format(
                            "layer_mask parameter must be a 2-dimensional ndarray with height as the first dimension and width as the second." \
                            " Got shape ({}) but expected (height, width)", fmt::join(shape, ", ")
                            )
                        );
                }

                auto width = shape[1];
                auto height = shape[0];
                params.mask = Util::vector_from_py_array<T>(mask, width, height);
                params.width = width;
                params.height = height;
			}
			params.name = layer_name;
			params.blendmode = blend_mode;
			params.opacity = opacity;
			params.compression = compression;
			params.colormode = color_mode;
			params.visible = is_visible;
			params.locked = is_locked;

            if (warp)
            {
                return std::make_shared<SmartObjectLayer<T>>(file, std::move(params), path, warp.value(), link_type);
            }
            return std::make_shared<SmartObjectLayer<T>>(file, std::move(params), path, link_type);
        }
    ),
    py::arg("layered_file"),
    py::arg("path"),
    py::arg("layer_name"),
    py::arg("link_type") = LinkedLayerType::data,
    py::arg("warp") = py::none(),
    py::arg("layer_mask") = py::none(),
    py::arg("blend_mode") = Enum::BlendMode::Normal,
    py::arg("opacity") = 255,
    py::arg("compression") = Enum::Compression::ZipPrediction,
    py::arg("color_mode") = Enum::ColorMode::RGB,
    py::arg("is_visible") = true,
    py::arg("is_locked") = false, R"pbdoc(

        Construct a SmartObjectLayer from the given filepath, linking the layer according to the link type.
        Accepts an optional warp object to construct the layer with. If None is passed we default initialize 
        the warp.

        :param layered_file: 
            The file into which the layer will be inserted. This needs to be present as the actual link to the
            image file is stored globally and not on the layer itself.
        :type layered_file: LayeredFile_*bit

        :param path: The path to the image file to link into this SmartObject. This must be a valid file on disk.
        :type path: str

        :param layer_name: The name of the group, its length must not exceed 255
        :type layer_name: str

        :param layer_mask: 
            Optional layer mask, must have the same dimensions as height * width as a 2-dimensional array with row-major ordering (for a numpy
            2D array this would mean with a shape of (height, width)
        :type layer_mask: numpy.ndarray

        :param blend_mode: Optional, the blend mode of the layer, 'Passthrough' is the default for groups.
        :type blend_mode: psapi.enum.BlendMode

        :param opacity: The opacity of the layer from 0-255 where 0 is 0% and 255 is 100%. Defaults to 255
        :type opacity: int

        :param compression: The compression to apply to all the channels of the layer, including mask channels
        :type compression: psapi.enum.Compression

        :param color_mode: The color mode of the Layer, this must be identical to the color mode of the document. Defaults to RGB
        :type color_mode: psapi.enum.ColorMode

        :param is_visible: Whether the group is visible
        :type is_visible: bool

        :param is_locked: Whether the group is locked
        :type is_locked: bool

        :raises:
            ValueError: if length of layer name is greater than 255

            ValueError: if opacity is not between 0-255

	)pbdoc");

	// Attributes
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------

    smart_object_layer.def_property("linkage", [](Class& self)
        {
            auto type = self.linked_externally() ? LinkedLayerType::external : LinkedLayerType::data;
            return type;
        }, [](Class& self, LinkedLayerType type)
		{
			self.set_linkage(type);
		});

    smart_object_layer.def_property("warp", [](Class& self)
        {
            return self.warp();
        }, [](Class& self, SmartObject::Warp& _warp)
		{
            self.warp(warp);
		});


	// Functions
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------

    smart_object_layer.def("replace", [](Class& self, std::string path, bool link_externally = false)
        {
            self.replace(path, link_externally);
        }, py::arg("path"), py::arg("link_externally") = false, R"pbdoc(

        Replace the smart object with the given path keeping transformations as well as warp in place.

        :param path: 
            The new filepath to link to, this must be a file format recognized both by Photoshop and OpenImageIO
        :param link_externally:
            Whether to link the file externally or store the raw file bytes on the 
	        photoshop document itself. Keeping this at its default `False` is recommended
	        for sharing these files. If the file already exists as another smart object layer
            this parameter is ignored.

	    )pbdoc");

    smart_object_layer.def("hash", &Class::hash, R"pbdoc(

        Retrieve the hashed value associated with the layer, this is what is used to identify the
	    linked layer associated with this smart object (where the original image data is stored)

	    )pbdoc");

    smart_object_layer.def("filename", &Class::filename, R"pbdoc(

        Retrieve the filename associated with this smart object.

	    )pbdoc");

	smart_object_layer.def("filepath", &Class::filepath, R"pbdoc(

        Retrieve the filepath associated with this smart object. Depending on how the 
	    Smart object is linked (`external` or `data`) this may not be written to disk.
        If the file is linked as `data` this path may not represent the actual filepath 
        on disk as this information is no longer present.

	    )pbdoc");

	// Image data.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------

    smart_object_layer.def("original_image_data", [](Class& self)
        {
            auto data = self.original_image_data();
            std::unordered_map<int, py::array_t<T>> out_data;
            for (auto& [key, value] : data)
            {
                out_data[key.index] = to_py_array(std::move(value), self.original_width(), self.original_height());
            }
            return out_data;
        }, R"pbdoc(

        Extract all the channels of the original image data.
	    
	    Unlike the accessors `get_image_data()` and `get_channel()` this function gets the full resolution
	    image data that is stored on the smart object, i.e. the original image data. This may be smaller
	    or larger than the layers `width` or `height`. To get the actual resolution you can query: `original_width()` and `original_height()`

	    )pbdoc");

    smart_object_layer.def("original_width", &Class::original_width, R"pbdoc(

        Retrieve the original image datas' width.
	    
	    This does not have the same limitation as Photoshop layers of being limited
	    to 30,000 or 300,000 pixels depending on the file type
	    
	    :raises RuntimeError: if the hash defined by `hash()` is not valid for the document
	    
	    :returns: The width of the original image data

	    )pbdoc");

    smart_object_layer.def("original_height", &Class::original_height, R"pbdoc(

        Retrieve the original image datas' height.
	    
	    This does not have the same limitation as Photoshop layers of being limited
	    to 30,000 or 300,000 pixels depending on the file type
	    
	    :raises RuntimeError: if the hash defined by `hash()` is not valid for the document
	    
	    :returns: The height of the original image data

	    )pbdoc");

	// Transformations.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------

    smart_object_layer.def("move", [](Class& self, double x_offset, double y_offset)
        {
            Geometry::Point2D<double> point(x_offset, y_offset);
            self.move(point);
		}, py::arg("x_offset"), py::arg("y_offset"), R"pbdoc(

        Move the layer (including any warps) by the given x and y offset.

	    )pbdoc");

	smart_object_layer.def("rotate", [](Class& self, double angle, double x, double y)
		{
			Geometry::Point2D<double> point(x, y);
			self.rotate(angle, point);
		}, py::arg("angles"), py::arg("x"), py::arg("y"), R"pbdoc(

        Rotate the layer (including any warps) by the given angle (in degrees) around
        the point defined by the x and y coordinate. If you wish to rotate around the
        layers center you can call the function as follows:

        `layer.rotate(45, layer.center_x, layer.center_y)`

        :param angle: The angle to rotate with in degrees
        :param x:     The x position to rotate about
        :param y:     The y position to rotate about

	    )pbdoc");

	smart_object_layer.def("scale", [](Class& self, double x_scalar, double y_scalar, double x, double y)
		{
			Geometry::Point2D<double> scalar(x_scalar, y_scalar);
			Geometry::Point2D<double> point(x, y);
			self.scale(scalar, point);
		}, py::arg("x_scalar"), py::arg("y_scalar"), py::arg("x"), py::arg("y"), R"pbdoc(

        Scale the layer (including any warps) by the given x and y scalar around
        the point defined by the x and y coordinate. If you wish to scale around the
        layers center you can call the function as follows:

        `layer.scale(1.0, 1.0, layer.center_x, layer.center_y)`

        :param x_scalar: The x component of the scalar
        :param y_scalar: The y component of the scalar
        :param x:        The x position to scale about
        :param y:        The y position to scale about

	    )pbdoc");

	smart_object_layer.def("transform", [](Class& self, py::array_t<double> matrix)
		{
            auto mat = Util::matrix_from_py_array(matrix);
			self.transform(mat);
		}, py::arg("matrix"), R"pbdoc(

        Apply the transformation matrix to the smart object layer. This must be a 3x3 matrix
        which can contain both affine and non affine transformations.

        :param matrix: The matrix to transform by, as a 3x3 matrix of np.double
        :type matrix: np.ndarray

	    )pbdoc");

	smart_object_layer.def("reset_warp", &Class::reset_warp, R"pbdoc(

        Reset the warp (not the transformations) applied to the Smart Object.
	    
	    If you instead wish to clear the transformations you can use the `reset_transform()` function.
	    
	    These two may be used in combination and sequence, so it is perfectly valid to call `reset_transform`
	    and `reset_warp` in any order

	    )pbdoc");

	smart_object_layer.def("reset_transform", &Class::reset_transform, R"pbdoc(

        Reset all the transformations (not the warp) applied to the layer to map it back to the original square 
	    from [0 - `original_width()`] and [0 - `original_height()`]. This does not reset the warp itself so if 
        you had a warp applied it will stay.
	    
	    If you instead wish to clear the warp you can use `reset_warp()`.
	    
	    These two may be used in combination and sequence, so it is perfectly valid to call `reset_transform`
	    and `reset_warp` in any order.

	    )pbdoc");
}