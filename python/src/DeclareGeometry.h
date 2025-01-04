#pragma once

#include "Util/Enum.h"
#include "Macros.h"
#include "Core/Geometry/Point.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
#include <pybind11/iostream.h>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


void declare_point2d(py::module& m)
{
	using Class = Geometry::Point2D<double>;
	py::class_<Class> point_2d(m, "Point2D", py::dynamic_attr(), py::buffer_protocol());

	point_2d.doc() = R"pbdoc(

		2D Point with basic geometric functions

		Attributes
		-------------

		x : float
		y : float

	)pbdoc");

}