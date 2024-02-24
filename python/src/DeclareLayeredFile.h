#include "LayeredFile/LayeredFile.h"
#include "Macros.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
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
	py::class_<LayeredFileWrapper> layeredFileWrapper(m, "LayeredFile");
	layeredFileWrapper.def_static("read", &LayeredFileWrapper::read, py::arg("path"));
}

// Generate a LayeredFile python class from our struct adjusting some
// of the methods 
template <typename T>
void declareLayeredFile(py::module& m, const std::string& extension) {
	using Class = LayeredFile<T>;
	std::string className = "LayeredFile" + extension;
	py::class_<Class> layeredFile(m, className.c_str(), py::dynamic_attr());

	layeredFile.def(py::init<>());
	layeredFile.def(py::init<Enum::ColorMode, uint64_t, uint64_t>(), py::arg("color_mode"), py::arg("width"), py::arg("height"));

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
		}, py::arg("path"));
	layeredFile.def("__getitem__", [](Class& self, const std::string name)
		{
			for (auto& layer : self.m_Layers)
			{
				// Get the layer name and recursively check the path
				if (layer->m_LayerName == name)
				{
					return layer;
				}
			}
			throw py::key_error("Unable to find layer '" + name + "' in the LayeredFile");
		}, py::arg("name"));


	layeredFile.def("add_layer", &Class::addLayer, py::arg("layer"));
	layeredFile.def("move_layer", py::overload_cast<std::shared_ptr<Layer<T>>, std::shared_ptr<Layer<T>>>(&Class::moveLayer), 
		py::arg("child"), 
		py::arg("parent") = py::none().cast<std::shared_ptr<Layer<T>>>());
	layeredFile.def("move_layer", py::overload_cast<const std::string, const std::string>(&Class::moveLayer), 
		py::arg("child"), 
		py::arg("parent") = "");
	layeredFile.def("remove_layer", py::overload_cast<std::shared_ptr<Layer<T>>>(&Class::removeLayer), py::arg("layer"));
	layeredFile.def("remove_layer", py::overload_cast<const std::string>(&Class::removeLayer), py::arg("layer"));
	
	// Properties
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	layeredFile.def_property("icc", [](const Class& self)
		{
			uint8_t* ptr = self.m_ICCProfile.getData().data();
			std::vector<size_t> shape = { self.m_ICCProfile.getDataSize() };
			return py::array_t<uint8_t>(shape, ptr);
		}, [](Class& self, const std::filesystem::path& path)
		{
			self.m_ICCProfile = ICCProfile(path);
		});
	layeredFile.def_property("compression", nullptr, &Class::setCompression);
	layeredFile.def_property_readonly("num_channels", &Class::getNumChannels);
	layeredFile.def_property_readonly("layers", [](const Class& self) {return self.m_Layers; });
	layeredFile.def_property_readonly("bit_depth", [](const Class& self) { return self.m_BitDepth; });
	layeredFile.def_property("dpi",
			[](const Class& self) { return self.m_DotsPerInch; },
			[](Class& self, float dpi) { self.m_DotsPerInch = dpi; }
	);
	layeredFile.def_property("width",
			[](const Class& self) { return self.m_Width; },
			[](Class& self, uint64_t width) { self.m_Width = width; }
	);
	layeredFile.def_property("height",
			[](const Class& self) { return self.m_Height; },
			[](Class& self, uint64_t  height) { self.m_Height = height; }
	);
	layeredFile.def("is_layer_in_document", &Class::isLayerInDocument, py::arg("layer"), "Check if the requested layer is already in the document");

	// Read/write functionality
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	layeredFile.def_static("read", &Class::read, py::arg("path"));

	// wrap the write function to no longer be static as we dont have move semantics and it makes the signature
	// a bit awkward otherwise so that now you can just call LayeredFile.write("SomeFile.psd")
	layeredFile.def("write", [](Class& self, const std::filesystem::path& path, const bool force_overwrite = true)
	{
		self.write(std::move(self), path, force_overwrite);
	}, py::arg("path"), py::arg("force_overwrite") = true);
}


