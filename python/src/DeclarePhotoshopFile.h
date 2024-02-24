#include "PhotoshopFile/PhotoshopFile.h"
#include "Macros.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
#include <pybind11/iostream.h>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


void declarePhotoshopFile(py::module& m)
{
	py::class_<PhotoshopFile> photoshopFile(m, "PhotoshopFile");

	photoshopFile.def(py::init<>());
	photoshopFile.def("read", &PhotoshopFile::read, py::arg("document"));
	photoshopFile.def("write", &PhotoshopFile::write, py::arg("document"));
	photoshopFile.def_static("find_bitdepth", [](const std::filesystem::path& path)
		{
			File document{ path };
			FileHeader header;
			header.read(document);
			return header.m_Depth;
		}, py::arg("path"));
}