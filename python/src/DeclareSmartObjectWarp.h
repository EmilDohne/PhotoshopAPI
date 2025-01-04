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




// Generate a LayeredFile python class from our struct adjusting some
// of the methods 
void declare_smart_object_warp(py::module& m)
{
	using Class = SmartObject::Warp;
	py::class_<Class> smart_object_warp(m, "SmartObjectWarp", py::dynamic_attr(), py::buffer_protocol());


}