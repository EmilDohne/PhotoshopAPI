#pragma once

#include "Util/Enum.h"
#include "Macros.h"
#include "Core/Geometry/Point.h"
#include "Core/Geometry/MeshOperations.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
#include <pybind11/iostream.h>

#include <variant>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


void declare_point2d(py::module& m)
{
	using Class = Geometry::Point2D<double>;
	py::class_<Class> point_2d(m, "Point2D", py::dynamic_attr(), py::buffer_protocol());

	point_2d.doc() = R"pbdoc(

		2D Point with basic arithmetic and geometric functions

		Attributes
		-------------

		x : float
		y : float

	)pbdoc";


	point_2d.def(py::init<double, double>(), py::arg("x"), py::arg("y"));

	// Attributes
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------

	point_2d.def_readwrite("x", &Class::x);
	point_2d.def_readwrite("y", &Class::y);

	// Comparison operators
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------

	point_2d.def("__eq__", [](Class& self, Class other)
		{
			return self == other;
		});
	point_2d.def("__ne__", [](Class& self, Class other)
		{
			return self != other;
		});

	point_2d.def("__repr__", [](Class& self)
		{
			return fmt::format("[{}, {}]", self.x, self.y);
		});

	point_2d.def("__str__", [](Class& self)
		{
			return fmt::format("[{}, {}]", self.x, self.y);
		});

	point_2d.def("__hash__", [](Class& self)
		{
			return self.hash();
		});

	// Arithmetic operators
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------

	point_2d.def("__len__", [](Class& self) { return 2; });

	point_2d.def("__add__", [](Class& self, std::variant<Class, Class::value_type> other)
		{
			if (std::holds_alternative<Class>(other))
			{
				return self + std::get<Class>(other);
			}
			else
			{
				return self + std::get<Class::value_type>(other);
			}
		});
	point_2d.def("__sub__", [](Class& self, std::variant<Class, Class::value_type> other)
		{
			if (std::holds_alternative<Class>(other))
			{
				return self - std::get<Class>(other);
			}
			else
			{
				return self - std::get<Class::value_type>(other);
			}
		});
	point_2d.def("__neg__", [](Class& self) { return -self; });
	point_2d.def("__mul__", [](Class& self, std::variant<Class, Class::value_type> other)
		{
			if (std::holds_alternative<Class>(other))
			{
				return self * std::get<Class>(other);
			}
			else
			{
				return self * std::get<Class::value_type>(other);
			}
		});
	point_2d.def("__truediv__", [](Class& self, std::variant<Class, Class::value_type> other)
		{
			if (std::holds_alternative<Class>(other))
			{
				auto val = std::get<Class>(other);
				if (val.x == 0.0f || val.y == 0.0f)
				{
					throw py::value_error("Unable to divide Point2D by zero");
				}
				return self / val;
			}
			else
			{
				auto val = std::get<Class::value_type>(other);
				if (val == 0.0f)
				{
					throw py::value_error("Unable to divide Point2D by zero");
				}
				return self / val;
			}
		});

	// Geometric operators
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------

	point_2d.def("distance", [](Class& self, Class other)
		{
			return self.distance(other);
		});

	point_2d.def_static("lerp", [](Class a, Class b, double t)
		{
			if (t < 0.0f || t > 1.0f)
			{
				throw py::value_error("t interpolation value must be between 0 and 1");
			}
			return Class::lerp(a, b, t);
		});
}

void declare_geometry_operations(py::module& m)
{
	m.def("create_normalized_quad", &Geometry::create_normalized_quad<double>, R"pbdoc(

		Generate a normalized quad in the 0 - 1 range with the points in the following order:
		
		top-left, top-right, bot-left, bot-right

	)pbdoc");

	m.def("create_quad", &Geometry::create_quad<double>, py::arg("width"), py::arg("height"), R"pbdoc(

		Generate a normalized quad in the 0 - width/height range with the points in the following order:
		
		top-left, top-right, bot-left, bot-right

	)pbdoc");

	m.def("create_homography", [](std::array<Geometry::Point2D<double>, 4> source_points, std::array<Geometry::Point2D<double>, 4> destination_points)
		{
			auto mat = Geometry::Operations::create_homography_matrix<double>(source_points, destination_points);
			return Util::matrix_to_py_array(mat);
		}, py::arg("source_points"), py::arg("destination_points"), R"pbdoc(

		Compute a 3x3 homography transformation matrix based on the given source and destination quad which will define
		the transformation matrix of getting the source quad to the destination quad.
		
		:param source_points:      The source points to the first quad, must have a length of 4
        :param destination_points: The source points to the second quad, must have a length of 4

		:raises ValueError: If the source or destination points do not have a length of 4

	)pbdoc");
}