#include "LayeredFile/LayeredFile.h"
#include "Macros.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
#include <pybind11/iostream.h>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


// Generate a LayeredFile python class from our struct adjusting some
// of the methods 
template <typename T>
void declareLayeredFile(py::module& m, const std::string& extension) {
	using Class = LayeredFile<T>;
	std::string className = "LayeredFile" + extension;

	py::class_<Class> layeredFile(m, className.c_str(), py::dynamic_attr());

	layeredFile.def(py::init<>(), "Initialize an empty LayeredFile instance");

	// Layer Manipulation
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	layeredFile.def("find_layer", &Class::findLayer, "Find a layer based on the given path.");
	layeredFile.def("add_layer", &Class::addLayer, "Add a layer to the layered file.");
	layeredFile.def("move_layer", py::overload_cast<std::shared_ptr<Layer<T>>, std::shared_ptr<Layer<T>>>(&Class::moveLayer), "Move a layer to a new parent node.");
	layeredFile.def("move_layer", py::overload_cast<const std::string, const std::string>(&Class::moveLayer), "Move a layer to a new parent node.");
	layeredFile.def("remove_layer", py::overload_cast<std::shared_ptr<Layer<T>>>(&Class::removeLayer), "Remove a layer from the layered file.");
	layeredFile.def("remove_layer", py::overload_cast<const std::string>(&Class::removeLayer), "Remove a layer from the layered file.");
	
	// Properties
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	layeredFile.def_property("compression", nullptr, &Class::setCompression, "Property for setting compression of all sublayers.");
	layeredFile.def_property_readonly("num_channels", &Class::getNumChannels, "Get the total number of channels in the document.");
	layeredFile.def_property_readonly("bit_depth", [](const Class& self) { return self.m_BitDepth; }, "Get the bit depth of the document");
	layeredFile.def_property("dpi",
			[](const Class& self) { return self.m_DotsPerInch; },
			[](Class& self, float dpi) { self.m_DotsPerInch = dpi; }, "Property for DPI settings."
	);
	layeredFile.def_property("width",
			[](const Class& self) { return self.m_Width; },
			[](Class& self, uint64_t width) { self.m_Width = width; }, "Property for width settings."
	);
	layeredFile.def_property("height",
			[](const Class& self) { return self.m_Height; },
			[](Class& self, uint64_t  height) { self.m_Height = height; }, "Property for height settings."
	);
	layeredFile.def("generate_flat_layers", &Class::generateFlatLayers, "Generate a flat layer stack from the layered file.");
	layeredFile.def("is_layer_in_document", &Class::isLayerInDocument, "Check if a layer exists in the document.");

	// Read/write functionality
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	layeredFile.def_static("read", &Class::read, "Read and create a LayeredFile from disk.");

	// wrap the write function to no longer be static as we dont have move semantics and it makes the signature
	// a bit awkward otherwise so that now you can just call LayeredFile.write("SomeFile.psd")
	layeredFile.def("write", [](Class& self, const std::filesystem::path& filePath, const bool forceOverwrite = true) 
	{
		self.write(std::move(self), filePath, forceOverwrite);
	}, py::arg("path"), py::arg("force_overwrite") = true, "Write the LayeredFile instance to disk.");
}


PYBIND11_MODULE(psapi, m) {
	declareLayeredFile<bpp8_t>(m, "_8bit");
	declareLayeredFile<bpp16_t>(m, "_16bit");
	declareLayeredFile<bpp32_t>(m, "_32bit");
}