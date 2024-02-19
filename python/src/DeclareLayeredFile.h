#include "LayeredFile/LayeredFile.h"
#include "Macros.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
#include <pybind11/iostream.h>

#include <iostream>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


struct LayeredFileWrapper
{
	using LayeredFileVariant = std::variant<LayeredFile<bpp8_t>, LayeredFile<bpp16_t>, LayeredFile<bpp32_t>>;

	inline static LayeredFileVariant read(const std::filesystem::path& filePath)
	{
		auto inputFile = File(filePath);
		auto psDocumentPtr = std::make_unique<PhotoshopFile>();
		psDocumentPtr->read(inputFile);
		if (psDocumentPtr->m_Header.m_Depth == Enum::BitDepth::BD_8)
		{
			LayeredFile<bpp8_t> layeredFile = { std::move(psDocumentPtr) };
			return layeredFile;
		}
		else if (psDocumentPtr->m_Header.m_Depth == Enum::BitDepth::BD_16)
		{
			LayeredFile<bpp16_t> layeredFile = { std::move(psDocumentPtr) };
			return layeredFile;
		}
		else if (psDocumentPtr->m_Header.m_Depth == Enum::BitDepth::BD_32)
		{
			LayeredFile<bpp32_t> layeredFile = { std::move(psDocumentPtr) };
			return layeredFile;
		}
		else
		{
			PSAPI_LOG_ERROR("LayeredFileWrapper", "Unable to extract the LayeredFile specialization from the fileheader");
		}
	}
};


// Declare the wrapper class for the LayeredFile instance
void declareLayeredFileWrapper(py::module& m)
{
	py::class_<LayeredFileWrapper> layeredFileWrapper(m, "LayeredFile", R"pbdoc(
		A wrapper class for the different LayeredFile subtypes that we can call read() on to
		return the appropriate LayeredFile instance.

		.. warning::
			
			The psapi.LayeredFile class' only job is to simplify the read of a LayeredFile_*bit from 
			disk with automatic type deduction. It does not however hold any of the data itself.
			
	)pbdoc");

	layeredFileWrapper.def_static("read", &LayeredFileWrapper::read, R"pbdoc(
		Read a layeredfile into the appropriate type based on the actual bit-depth
		of the document

		:param path: The path to the Photoshop file
		:type path: str

		:rtype: :class:`psapi.LayeredFile_8bit` | :class:`psapi.LayeredFile_16bit` | :class:`psapi.LayeredFile_32bit`
	)pbdoc", py::arg("path"));
}



// Generate a LayeredFile python class from our struct adjusting some
// of the methods 
template <typename T>
void declareLayeredFile(py::module& m, const std::string& extension) {
	using Class = LayeredFile<T>;
	std::string className = "LayeredFile" + extension;
	py::class_<Class> layeredFile(m, className.c_str(), py::dynamic_attr());

	layeredFile.doc() = R"pbdoc(
		This class defines a layered file structure, where each file contains a hierarchy of layers. Layers can be grouped and organized within this structure.
	)pbdoc";

	layeredFile.def(py::init<>(), "Initialize an empty LayeredFile instance");

	// Layer Manipulation
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	
	// We must wrap this as it otherwise returns a nullptr which we cannot have
	layeredFile.def("find_layer",[](const Class& self, const std::string& path)
		{
			auto layer = self.findLayer(path);
			if (layer)
			{
				return layer;
			}
			throw py::value_error("Path '" + path + "' is not valid in the layered_file");
		}, R"pbdoc(
			Find a layer based on the given path

			:param path: The path to the requested layer
			:type path: str

			:return: The requested layer

			:raises:
				ValueError: If the path is not a valid path to a layer
	)pbdoc");
	layeredFile.def("__getitem__", [](const Class& self, const std::string& name)
		{
			for (const auto layer : self.m_Layers)
			{
				// Get the layer name and recursively check the path
				if (layer->m_LayerName == name)
				{
					return layer;
				}
			}
			throw py::key_error("Unable to find layer '" + name + "' in the LayeredFile");
		}, R"pbdoc(
			Get the specified layer from the root of the layered file. Unlike :func:`find_layer` this does not 
			accept a path but rather a single layer located in the root layer. This is to make chaining of paths
			more pythonic since group layers also implement a __getitem__ function

			.. code-block:: python

				layered_file: LayeredFile_8bit = # Our layered file instance
				nested_img_layer = layered_file["Group"]["Image"]

			:param name: The name of the layer to search for
			:type name: str

			:raises:
				KeyError: If the requested layer is not found

			:return: The requested layer instance
		)pbdoc");


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


