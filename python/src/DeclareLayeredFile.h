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
		ProgressCallback callback{};
		auto inputFile = File(filePath);
		auto psDocumentPtr = std::make_unique<PhotoshopFile>();
		psDocumentPtr->read(inputFile, callback);
		if (psDocumentPtr->m_Header.m_Depth == Enum::BitDepth::BD_8)
		{
			LayeredFile<bpp8_t> layeredFile = { std::move(psDocumentPtr), filePath };
			return std::move(layeredFile);
		}
		else if (psDocumentPtr->m_Header.m_Depth == Enum::BitDepth::BD_16)
		{
			LayeredFile<bpp16_t> layeredFile = { std::move(psDocumentPtr), filePath };
			return std::move(layeredFile);
		}
		else if (psDocumentPtr->m_Header.m_Depth == Enum::BitDepth::BD_32)
		{
			LayeredFile<bpp32_t> layeredFile = { std::move(psDocumentPtr), filePath };
			return std::move(layeredFile);
		}
		else
		{
			PSAPI_LOG_ERROR("LayeredFileWrapper", "Unable to extract the LayeredFile specialization from the fileheader");
			return {};
		}
	}
};


// Declare the wrapper class for the LayeredFile instance
void declare_layered_file_wrapper(py::module& m)
{
	py::class_<LayeredFileWrapper> layeredFileWrapper(m, "LayeredFile");

	layeredFileWrapper.doc() = R"pbdoc(

		A wrapper class for the different LayeredFile subtypes that we can call read() on to
		return the appropriate LayeredFile instance.

		.. warning::
        
			The psapi.LayeredFile class' only job is to simplify the read of a LayeredFile_*bit from 
			disk with automatic type deduction. It does not however hold any of the data itself.

	)pbdoc";

	layeredFileWrapper.def_static("read", &LayeredFileWrapper::read, py::arg("path"), R"pbdoc(

		Read a layeredfile into the appropriate type based on the actual bit-depth of the document

        :param path: The path to the Photoshop file
        :type path: str

        :rtype: :class:`psapi.LayeredFile_8bit` | :class:`psapi.LayeredFile_16bit` | :class:`psapi.LayeredFile_32bit`

	)pbdoc");
}

// Generate a LayeredFile python class from our struct adjusting some
// of the methods 
template <typename T>
void declare_layered_file(py::module& m, const std::string& extension) {
	using Class = LayeredFile<T>;
	std::string className = "LayeredFile" + extension;
	py::class_<Class> layeredFile(m, className.c_str(), py::dynamic_attr());

	layeredFile.doc() = R"pbdoc(

		This class defines a layered file structure, where each file contains a hierarchy of layers. Layers can be grouped and organized within this structure.

		Attributes
		-------------
		icc : numpy.ndarray
			Property for setting and retrieving the ICC profile attached to the file. This does not do any color conversions
			but simply tells photoshop how to interpret the data. The assignment is overloaded such that you need to pass
			a path to the ICC file you want to load and loading will be done internally.

		compression : psapi.enum.Compression
			Write-only property which sets the compression of all the layers in the LayeredFile

		num_channels : int
			Read-only property to retrieve the number of channels from the file (excludes mask channels)

		bit_depth : psapi.enum.BitDepth
			Read-only property to retrieve the bit-depth

		layers : list[Layer_*bit]
			Read-only property to retrieve a list of all the layers in the root of the file
		
		flat_layers: list[Layer_*bit]
			Read-only property to retrieve a flat list of all the layers in the file, convenience function 
			for iterating them all at once. Do not attempt to modify the layer structure itself while iterating
			over this flattened layer list as this wil lead to undefined behaviour

		dpi : int
			The document DPI settings

		width : int
			The width of the document, must not exceed 30,000 for PSD or 300,000 for PSB

		height : int
			The height of the document, must not exceed 30,000 for PSD or 300,000 for PSB

	)pbdoc";

	layeredFile.def(py::init<>());
	layeredFile.def(py::init<Enum::ColorMode, uint64_t, uint64_t>(), py::arg("color_mode"), py::arg("width"), py::arg("height"));

	// Layer Manipulation
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	
	// We must wrap this as it otherwise returns a nullptr which we cannot have
	layeredFile.def("find_layer",[](const Class& self, const std::string& path)
		{
			auto layer = self.find_layer(path);
			if (layer)
			{
				return layer;
			}
			throw py::value_error("Path '" + path + "' is not valid in the layered_file");
		}, py::arg("path"), R"pbdoc(

		Find a layer based on the given path

        :param path: The path to the requested layer
        :type path: str

        :return: The requested layer

        :raises:
            ValueError: If the path is not a valid path to a layer

	)pbdoc");

	layeredFile.def("__getitem__", [](Class& self, const std::string name)
		{
			for (auto& layer : self.layers())
			{
				// Get the layer name and recursively check the path
				if (layer->name() == name)
				{
					return layer;
				}
			}
			throw py::key_error("Unable to find layer '" + name + "' in the LayeredFile");
		}, py::arg("name"), R"pbdoc(

		Get the specified layer from the root of the layered file. Unlike :func:`find_layer` this does not accept a path but rather a 
		single layer located in the root layer. This is to make chaining of paths more pythonic since group layers also implement a __getitem__ function

        .. code-block:: python

            layered_file: LayeredFile_*bit = # Our layered file instance
            nested_img_layer = layered_file["Group"]["Image"]

        :param name: The name of the layer to search for
        :type name: str

        :raises:
            KeyError: If the requested layer is not found

        :return: The requested layer instance

	)pbdoc");


	layeredFile.def("add_layer", &Class::add_layer, py::arg("layer"));
	layeredFile.def("move_layer", py::overload_cast<std::shared_ptr<Layer<T>>, std::shared_ptr<Layer<T>>>(&Class::move_layer), 
		py::arg("child"), 
		py::arg("parent") = py::none().cast<std::shared_ptr<Layer<T>>>(), R"pbdoc(
		
		Move the child layer to the provided parent layer, if none is provided we move to scene root instead
	)pbdoc");
	layeredFile.def("move_layer", py::overload_cast<const std::string, const std::string>(&Class::move_layer),
		py::arg("child"), 
		py::arg("parent") = "");
	layeredFile.def("remove_layer", py::overload_cast<std::shared_ptr<Layer<T>>>(&Class::remove_layer), py::arg("layer"), R"pbdoc(
		
		Remove the specified layer from root of the layered_file, if you instead wish to remove from a group call remove_layer on a GroupLayer_*bit instance instead

	)pbdoc");
	layeredFile.def("remove_layer", py::overload_cast<const std::string>(&Class::remove_layer), py::arg("layer"));
	
	// Properties
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	layeredFile.def_property("icc",
		[](const Class& self) {
			auto data = self.icc_profile().data();
			uint8_t* ptr = data.data();
			std::vector<size_t> shape = { self.icc_profile().data_size() };
			return py::array_t<uint8_t>(shape, ptr);
		},
		[](Class& self, std::variant<std::filesystem::path, py::array_t<uint8_t>> value) {
			if (std::holds_alternative<py::array_t<uint8_t>>(value)) {
				const auto& arr = std::get<py::array_t<uint8_t>>(value);
				py::buffer_info buf = arr.request();
				std::vector<uint8_t> data(
					static_cast<uint8_t*>(buf.ptr),
					static_cast<uint8_t*>(buf.ptr) + buf.size
				);
				self.icc_profile(ICCProfile(std::move(data)));
			}
			else if (std::holds_alternative<std::filesystem::path>(value)) {
				const auto& path = std::get<std::filesystem::path>(value);
				self.icc_profile(ICCProfile(path));
			}
			else {
				throw py::type_error("ICC profile must be either a numpy.ndarray or a filesystem path (str or os.PathLike)");
			}
		});
	layeredFile.def_property("compression", [](const Class& self) {throw py::type_error("compression property has no getter"); }, &Class::set_compression);
	layeredFile.def_property_readonly("num_channels", &Class::num_channels);
	layeredFile.def_property_readonly("layers", [](Class& self) { return self.layers(); });
	layeredFile.def_property_readonly("flat_layers", [](Class& self) { return self.flat_layers(); });
	layeredFile.def_property_readonly("bit_depth", [](const Class& self) { return self.bitdepth(); });
	layeredFile.def_property("dpi",
			[](const Class& self) { return self.dpi(); },
			[](Class& self, float dpi) { self.dpi(dpi); }
	);
	layeredFile.def_property("width",
			[](const Class& self) { return self.width(); },
			[](Class& self, uint64_t width) { self.width(width); }
	);
	layeredFile.def_property("height",
			[](const Class& self) { return self.height(); },
			[](Class& self, uint64_t height) { self.height(height); }
	);
	layeredFile.def("is_layer_in_document", &Class::is_layer_in_file, py::arg("layer"), R"pbdoc(

		Check if the layer already exists in the LayeredFile at any level of nesting, this check is done internally on add_layer().

	)pbdoc");

	// Read/write functionality
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	layeredFile.def_static("read", py::overload_cast<const std::filesystem::path&>(&Class::read), py::arg("path"), R"pbdoc(

		Read and create a LayeredFile from disk. If the bit depth isnt known ahead of time use LayeredFile.read() instead which will return the appropriate type

	)pbdoc");

	// wrap the write function to no longer be static as we dont have move semantics and it makes the signature
	// a bit awkward otherwise so that now you can just call LayeredFile.write("SomeFile.psd")
	layeredFile.def("write", [](Class& self, const std::filesystem::path& path, const bool force_overwrite = true)
	{
		self.write(std::move(self), path, force_overwrite);
	}, py::arg("path"), py::arg("force_overwrite") = true, R"pbdoc(

		Write the LayeredFile_*bit instance to disk invalidating the data, after this point trying to use the instance is undefined behaviour. 

        :param path: 
            The path of the output file, must have a .psd or .psb extension. Conversion between these two types 
            is taken care of internally
        :type path: 
            os.PathLike
        
        :param force_overwrite: 
            Defaults to True, whether to forcefully overwrite the file if it exists. if False the write-op fails
            and emits an error message
        :type force_overwrite: bool

	)pbdoc");
}


