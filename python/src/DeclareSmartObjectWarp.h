#include "Util/Enum.h"

#include "PyUtil/ImageConversion.h"
#include "PyUtil/Transformation.h"

#include "Core/Warp/SmartObjectWarp.h"
#include "Macros.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <vector>

#include <fmt/format.h>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


void declare_smart_object_warp(py::module& m)
{
	using Class = SmartObject::Warp;
	py::class_<Class> smart_object_warp(m, "SmartObjectWarp");

	smart_object_warp.doc() = R"pbdoc(

		Base warp structure, these encompass the warps found under Edit -> Transform which are:
		
		- Edit/Transform/Skew
		- Edit/Transform/Distort
		- Edit/Transform/Perspective
		- Edit/Transform/Warp
		
		The warp is stored as a combination of a 2D Bezier surface and both affine and non affine transformations.
		E.g. a Skew or Perspective warp are simply transformations while the Warp is a Bezier surface. The Bezier
		surface is made up of quadratic bezier patches and these are stored as a 2d grid of `points` on the warp.

		To modify the warp you can modify the points by transforming them in-place and setting them back on the warp.
		The points are scored in scanline order. While you can transform the warp directly on this struct, it is 
		instead recommended to use the SmartObjectLayer's transformation functions such as `move`, `rotate`, `scale`
		and `transform`.


        Attributes
        -----------
		
		points: List[psapi.geometry.Point2D]
			A 2D array of all the warp points, stored in scanline order from top left to bottom right. When modifying these
			you must explicitly assign the points back to the warp struct. These points describe a 2D bezier surface with 
			with `u_dims` * `v_dims` points. Modifying the number of points is invalid and if you wish to generate a different
			warp you must do so by re-initializing an instance.
			The points are stored in the coordinate space of the original image data (in the case of a SmartObjectLayer) meaning
			if the image is from [0 - 5000] in width the points will be relative to that coordinate space rather than to the 
			coordinate space of the transformed image. The points will later be transformed by `affine_transform` and
			`non_affine_transform`.
		u_dims: Read-only int
			The number of horizontal dimensions.
		v_dims: Read-only int
			The number of vertical dimensions.
		affine_transform: List[psapi.geometry.Point2D]
			A 4-point quad defining the affine transformation applied on top of the warp points. The affine transform 
			describes translation, scaling, rotation and shearing. It does not handle any perspective transforms.
		non_affine_transform: List[psapi.geometry.Point2D]
			A 4-point quad defining the non affine transformation applied on top of the affine transformation. This 
			additionally describes perspective transformations (where lines that were previously parallel no longer are).
			As this transformation is applied on top of `affine_transform` a no-op would be the same transformation as
			the `affine_transform`. 

        
    )pbdoc";

	smart_object_warp.def(py::init<std::vector<Geometry::Point2D<double>>, size_t, size_t>(), py::arg("points"), py::arg("u_dims"), py::arg("v_dims"), R"pbdoc(
	
		Initialize the warp struct from a set of geometric points describing a bezier surface with
		one or more quadratic bezier patches. These points are in scanline order (i.e. going first along the horizontal
		axis, then across the vertical axis). 
		
		Being a set of quadratic bezier patches the dimensions across the u and v (x and y) must be `4` or `4 + n * 3`
		where `n` is the number of subdivisions and is greater than one. In simple terms, this means a valid number of 
		points per axis is 4, 7, 10, 13 etc.
		
		:param warp: The warp points in scanline order.
		:param u_dims: The dimensions across the u (x)
		:param v_dims: The dimensions across the v (y)

	    )pbdoc");
	
	

	// Attributes
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	
	smart_object_warp.def_property("points", [](Class& self)
		{
			return self.points();
		}, [](Class& self, std::vector<Geometry::Point2D<double>> _points)
		{
			self.points(_points);
		});

	smart_object_warp.def_property_readonly("u_dims", &Class::u_dimensions);
	smart_object_warp.def_property_readonly("v_dims", &Class::v_dimensions);

	smart_object_warp.def_property("affine_transform", [](Class& self)
		{
			return self.affine_transform();
		}, [](Class& self, std::array<Geometry::Point2D<double>, 4> _array)
		{
			self.affine_transform(_array);
		});
	smart_object_warp.def_property("non_affine_transform", [](Class& self)
		{
			return self.non_affine_transform();
		}, [](Class& self, std::array<Geometry::Point2D<double>, 4> _array)
		{
			self.non_affine_transform(_array);
		});

	// Functions
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------

	smart_object_warp.def("no_op", &Class::no_op, R"pbdoc(
	
		Check if the warp resolves to a no-op. This means that all points for a given row/column
		lie on a single line and the non-affine transform is also a no op. If this is the case applying
		a warp can be skipped

	    )pbdoc");

	smart_object_warp.def("valid", &Class::valid, R"pbdoc(
	
		Deprecated: always returns true.

	    )pbdoc");

	smart_object_warp.def_static("generate_default", [](
		int width, 
		int height,
		int u_dims,
		int v_dims
		)
		{
			if (width < 0)
			{
				throw py::value_error("Unable to construct warp with a negative width");
			}
			if (height < 0)
			{
				throw py::value_error("Unable to construct warp with a negative height");
			}
			if (u_dims < 0)
			{
				throw py::value_error("Unable to construct warp with a negative amount of u_dims");
			}
			if (v_dims < 0)
			{
				throw py::value_error("Unable to construct warp with a negative amount of v_dims");
			}

			if (u_dims == 4 && v_dims == 4)
			{
				return Class::generate_default(static_cast<size_t>(width), static_cast<size_t>(height));
			}
			return Class::generate_default(
				static_cast<size_t>(width), 
				static_cast<size_t>(height), 
				static_cast<size_t>(u_dims), 
				static_cast<size_t>(v_dims));
		}, py::arg("width"), py::arg("height"), py::arg("u_dims") = 4, py::arg("v_dims") = 4, R"pbdoc(
	
		Generate and return a default warp structure for the given width and height, optionally being
		able to override the u and v dimensions. 

		Internally this will author a u*v grid describing a collection of cubic bezier patches whose points can be 
		transformed by retrieving `points` and applying the given transformation to them. 
		These grid points are laid out as follows (for a 4x4 grid, for other dimensions this would change accordingly):
		
		0  1  2  3

		4  5  6  7
		
		8  9  10 11
		
		12 13 14 15
		
		Here point 0 would be the top left corner point. And points 1 and 4 the handles to the bezier.
		You now might wonder what the purpose of the center points are as they are not exposed in 
		Photoshop itself. 
		
		These appear to be added to form a quad that is a parallelogram. We don't currently expose 
		any functionality for making this parallelogram from 3 points as the output is ambiguous.
		So if we continue with our previous example, to form the parallelogram we would have to
		modify point 5
		
		If you wish to see how a 4x4 grid can look check out this page:
		https://github.com/EmilDohne/PhotoshopAPI/issues/90#issuecomment-2441823792

		:raises ValueError: If any of the given parameters is below 0
		:raises RuntimeError: If the u or v dims do not describe a cubic bezier.

		:param width: 
			The width of the warp, logically this should be the full image width.
			In the context of a smart object for example this would be the width of the whole image,
			not of the generated preview.
		:param height: 
			The height of the warp, logically this should be the full image height.
			In the context of a smart object for example this would be the height of the whole image,
			not of the generated preview.
		:param u_dimensions: 
			The divisions in the u (x) dimension. These must follow the formula 4 + n * 3
			where n represents the number of horizontal bezier patches - 1. So if you wish
			to construct 3 bezier patches horizontally this would be 10.
		:param v_dimensions: 
			The divisions in the v (y) dimension. These must follow the formula 4 + n * 3
			where n represents the number of vertical bezier patches - 1. So if you wish
			to construct 3 bezier patches vertically this would be 10.

	    )pbdoc");

}