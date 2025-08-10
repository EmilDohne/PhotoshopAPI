cmake_minimum_required (VERSION 3.20)
set(VCPKG_LIBRARY_LINKAGE static)
set (PhotoshopAPI_VERSION "0.7.0")
project ("PhotoshopAPIBuild" VERSION 0.7.0 LANGUAGES CXX)

if (MSVC)
    add_compile_definitions(NOMINMAX)
endif()

# --------------------------------------------------------------------------
# Configurable options

option(PSAPI_BUILD_STATIC "Build a static library version of the PhotoshopAPI, currently must be on as we dont create a dynamic library target" ON)
option(PSAPI_USE_VCPKG "Build the extra dependencies using vckpg instead of system installed libraries" ON)
option(PSAPI_BUILD_TESTS "Build the tests associated with the PhotoshopAPI" ON)
option(PSAPI_BUILD_EXAMPLES "Build the examples associated with the PhotoshopAPI" ON)
option(PSAPI_BUILD_BENCHMARKS "Build the benchmarks associated with the PhotoshopAPI" ON)
option(PSAPI_BUILD_DOCS "Builds the documentation, requires some external installs which are documented in the README.md" OFF)
option(PSAPI_BUILD_PYTHON "Build the python bindings associated with the PhotoshopAPI" ON)


if (PSAPI_BUILD_PYTHON)
    # Link in the msvc runtime so that users dont need vcredist when using the python bindings
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()



# Build setup
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set (CMAKE_CXX_STANDARD 20)

# Add the cmake/ folder so the FindSphinx module is found
# --------------------------------------------------------------------------
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})


# Set up our toolchain using the local vcpkg, gets the 
# --------------------------------------------------------------------------

# Include the local vcpkg toolchain, if specified
if (PSAPI_USE_VCPKG)
    include("${PROJECT_SOURCE_DIR}/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

# Here we add support for OpenImageIO which is used for smart object image data extraction.
find_package(OpenImageIO CONFIG REQUIRED)

# Add libdeflate for zip compression and decompression
find_package(libdeflate CONFIG REQUIRED)

# Eigen is used to construct e.g. homographies for applying non-affine transforms
find_package(Eigen3 CONFIG REQUIRED)

# fmt library for formatting and logging.
find_package(fmt CONFIG REQUIRED)

# UUID for storing e.g. uuids of smart objects
find_package(stduuid CONFIG REQUIRED)



# Add thirdparty libraries
# --------------------------------------------------------------------------

# Add mio for memory-mapped IO files
add_subdirectory (thirdparty/mio)


# Add doctest
add_library(doctest INTERFACE)
target_include_directories(doctest SYSTEM INTERFACE thirdparty/doctest/doctest)

# Add simdutf for UTF conversion operations
set (SIMDUTF_TESTS OFF)
set (SIMDUTF_TOOLS OFF)
set (SIMDUTF_ICONV OFF)
add_subdirectory (thirdparty/simdutf)

# Add span from tcb for compatibility with older compilers for python bindings
add_library(tcb_span INTERFACE)
target_include_directories(tcb_span SYSTEM INTERFACE thirdparty/compatibility)

# CompressedImage for loading images into a compressed memory buffer as well as storing our channels
set(COMPRESSED_IMAGE_USE_VCPKG OFF)
add_subdirectory(thirdparty/compressed-image)

# Projects
# --------------------------------------------------------------------------
if(PSAPI_BUILD_STATIC)
    add_subdirectory (PhotoshopAPI)
endif()
if(PSAPI_BUILD_TESTS)
    add_subdirectory (PhotoshopTest)
    if (CMAKE_UNITY_BUILD)
        set_target_properties(PhotoshopTest PROPERTIES UNITY_BUILD OFF)
    endif()
endif()
if(PSAPI_BUILD_BENCHMARKS)
    add_subdirectory (PhotoshopBenchmark)
endif()
if(PSAPI_BUILD_EXAMPLES)
    add_subdirectory (PhotoshopExamples/AddLayerMasks)
    add_subdirectory (PhotoshopExamples/CreateGroups)
    add_subdirectory (PhotoshopExamples/CreateSimpleDocument)
    add_subdirectory (PhotoshopExamples/ExtendedSignature)
    add_subdirectory (PhotoshopExamples/ExtractImageData)
    add_subdirectory (PhotoshopExamples/ModifyLayerStructure)
    add_subdirectory (PhotoshopExamples/ProgressCallbacks)
    add_subdirectory (PhotoshopExamples/ReplaceImageData)
    add_subdirectory (PhotoshopExamples/SmartObjects)
endif()
if(PSAPI_BUILD_DOCS)
    if(NOT PSAPI_BUILD_PYTHON)
        message(WARNING "Building the documentation without the python bindings, this means the python bindings wont show up in your local copy")
    endif()
    add_subdirectory (docs/doxygen)
endif()
if(PSAPI_BUILD_PYTHON)
    add_subdirectory (thirdparty/pybind11)
    add_subdirectory (python)
endif()