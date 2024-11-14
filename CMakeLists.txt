cmake_minimum_required (VERSION 3.20)
set(VCPKG_LIBRARY_LINKAGE "static")
project ("PhotoshopAPIBuild" VERSION 0.5.1 LANGUAGES CXX)

if (MSVC)
    add_compile_definitions(NOMINMAX)
endif()

# --------------------------------------------------------------------------
# Configurable options

option(PSAPI_BUILD_STATIC "Build a static library version of the PhotoshopAPI" ON)
option(PSAPI_BUILD_OIIO "Include OpenImageIO in the builds. This is required for extracting image data from smart objects" ON)
option(PSAPI_BUILD_TESTS "Build the tests associated with the PhotoshopAPI" ON)
option(PSAPI_BUILD_EXAMPLES "Build the examples associated with the PhotoshopAPI" ON)
option(PSAPI_BUILD_BENCHMARKS "Build the benchmarks associated with the PhotoshopAPI" ON)
option(PSAPI_BUILD_DOCS "Builds the documentation, requires some external installs which are documented in the README.md" OFF)
option(PSAPI_BUILD_PYTHON "Build the python bindings associated with the PhotoshopAPI" OFF)


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


# Set up our toolchain
# --------------------------------------------------------------------------

# Here we add support for OpenImageIO which is used for smart object image data extraction.
# This isn't strictly required. OpenImageIO may be passed externally rather than through the 
# vcpkg submodule. Users can provide an external toolchain file with DCMAKE_TOOLCHAIN_FILE
# which will essentially disable the building of OIIO locally. If DCMAKE_TOOLCHAIN_FILE is passed
# and PSAPI_BUILD_OIIO is true we first check whether the toolchain file defines OpenImageIO as a
# target and otherwise build it locally.
if (PSAPI_BUILD_OIIO)
    find_package(OpenImageIO CONFIG QUIET)

    if (TARGET OpenImageIO::OpenImageIO AND OpenImageIO_VERSION VERSION_GREATER "2.5")
        # OpenImageIO is found and meets version requirements
    elseif (DEFINED CMAKE_TOOLCHAIN_FILE)
        # Toolchain file specified, attempt to find OpenImageIO within it
        if (NOT TARGET OpenImageIO::OpenImageIO OR OpenImageIO_VERSION VERSION_LESS "2.5")
            message(STATUS "OpenImageIO version > 2.5 not found in toolchain. Attempting to build locally...")
        
            # Add vcpkg submodule to build OpenImageIO if not available
            include("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake")
        
            # Try finding OpenImageIO after including vcpkg
            find_package(OpenImageIO CONFIG REQUIRED)
        else()
            message(STATUS "OpenImageIO found in toolchain file.")
            get_target_property(OIIO_LOCATION OpenImageIO::OpenImageIO IMPORTED_LOCATION)
            if (OIIO_LOCATION)
                message(STATUS "OpenImageIO found at ${OIIO_LOCATION}")
            else()
                message(WARNING "Could not retrieve OpenImageIO location.")
            endif()
        endif()
    else()
        # No toolchain file provided, default to building OpenImageIO locally via vcpkg
        include("${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake")
        find_package(OpenImageIO CONFIG REQUIRED)
    endif()
endif()


# Add thirdparty libraries
# --------------------------------------------------------------------------


# Add libdeflate for zip compression and decompression
find_package(libdeflate CONFIG REQUIRED)

find_package(Eigen3 CONFIG REQUIRED)


# Add mio for memory-mapped IO files
add_subdirectory (thirdparty/mio)

# Add c-blosc2
set(DEACTIVATE_ZLIB ON)
set(DEACTIVATE_ZSTD ON)
set(BUILD_TESTS OFF)
set(BUILD_FUZZERS OFF)
set(BUILD_BENCHMARKS OFF)
set(BUILD_EXAMPLES OFF)
add_subdirectory (thirdparty/c-blosc2 EXCLUDE_FROM_ALL)

# Add target for blosc2 headers
add_library(blosc2_include INTERFACE)
target_include_directories(blosc2_include SYSTEM INTERFACE thirdparty/c-blosc2/include)

# Add doctest
add_library(doctest INTERFACE)
target_include_directories(doctest SYSTEM INTERFACE thirdparty/doctest/doctest)

# Add simdutf for UTF conversion operations
set (SIMDUTF_TESTS OFF)
set (SIMDUTF_TOOLS OFF)
set (SIMDUTF_ICONV OFF)
add_subdirectory (thirdparty/simdutf)

# fmt library for formatting and logging.
find_package(fmt CONFIG REQUIRED)

# UUID for storing e.g. uuids of smart objects
find_package(stduuid CONFIG REQUIRED)

# JSON module for parsing of Descriptors
add_subdirectory(thirdparty/json)

# Add span from tcb for compatibility with older compilers for python bindings
add_library(tcb_span INTERFACE)
target_include_directories(tcb_span SYSTEM INTERFACE thirdparty/compatibility)

# Projects
# --------------------------------------------------------------------------
if(PSAPI_BUILD_STATIC)
    add_subdirectory (PhotoshopAPI)
endif()
if(PSAPI_BUILD_TESTS)
    add_subdirectory (PhotoshopTest)
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
    add_subdirectory (PhotoshopExamples/Tmp)
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