#pragma once

#include "Util/Enum.h"
#include "Macros.h"
#include "Core/Struct/OCIOProfile.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
#include <pybind11/iostream.h>

#include <variant>


namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


void declare_ocio_profile(py::module& m)
{
	using Class = OCIOProfile;
	py::class_<Class> ocio_profile(m, "OcioProfile", py::dynamic_attr(), py::buffer_protocol());

	ocio_profile.doc() = R"pbdoc(

		OCIO Profile representation.

		Attributes
		-------------

		config : PyOpenColorIO.Config
		working_space : str
		view_transform : str
		display_transform : str

	)pbdoc";


	ocio_profile.def(
		py::init<str, str, str, str>(), 
		py::arg("config"), 
		py::arg("working_space"),
		py::arg("view_transform"),
		py::arg("display_transform"),
		);

	// Attributes
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------

	ocio_profile.def_property_readonly("config", &Class::config);
	ocio_profile.def_property_readonly("working_space", &Class::working_space);
	ocio_profile.def_property_readonly("view_transform", &Class::view_transform);
	ocio_profile.def_property_readonly("display_transform", &Class::display_transform);

}