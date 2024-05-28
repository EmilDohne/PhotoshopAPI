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

	photoshopFile.doc() = R"pbdoc(

		This class represents the low-level File Structure of the Photoshop document itself.
		In the python bindings we explicitly do not expose all of its sub-classes as the implementation
		details are currently not meant to be accessed

	)pbdoc";

	photoshopFile.def(py::init<>());

	photoshopFile.def("read", [](PhotoshopFile& self, File& document)
		{
			ProgressCallback callback{};
			self.read(document, callback);
			
		}, py::arg("document"), R"pbdoc(

		Read the PhotoshopFile class from a File instance, this file must be a valid .psd or .psb file.

        :param document: The file object used for reading
        :type document: :class:`psapi.util.File`

        :rtype: None

	)pbdoc");

	photoshopFile.def("write", [](PhotoshopFile& self, File& document)
		{
			ProgressCallback callback{};
			self.write(document, callback);

		}, py::arg("document"), R"pbdoc(

		Write the PhotoshopFile class to disk using a instance, this file must be a valid .psd or .psb file.

        :param document: The file object used for writing
        :type document: :class:`psapi.util.File`

        :rtype: None

	)pbdoc");

	photoshopFile.def_static("find_bitdepth", [](const std::filesystem::path& path)
		{
			File document{ path };
			FileHeader header;
			header.read(document);
			return header.m_Depth;
		}, py::arg("path"), R"pbdoc(

		Find the bit depth of a Photoshop file from the given filepath.
        This function has basically no runtime cost as it simply reads the first 26 bytes of the document
        and uses that to extract the bit depth. The intention of this function is to provide an interface
        to quickly check which psapi.LayeredFile instance to construct. For example

        .. code-block:: python

            depth = psapi.PhotoshopFile.find_bitdepth("SomeFile.psb")
            if (depth == psapi.enum.BitDepth.bd_8):
                layered_file = psapi.LayeredFile_8bit.read("SomeFile.psb")
            # etc...

        :param filepath: The path to the Photoshop file.
        :type filepath: str

        :return: The bit depth of the Photoshop file as an Enum::BitDepth.
        :rtype: :class:`psapi.enum.BitDepth`

	)pbdoc");
}