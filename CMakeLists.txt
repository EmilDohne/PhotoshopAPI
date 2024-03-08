cmake_minimum_required (VERSION 3.20)
project ("PhotoshopAPIBuild" VERSION 0.4.0 LANGUAGES CXX)

if (MSVC)
    # MSVC is stupid
    add_compile_definitions(NOMINMAX)
endif()

# --------------------------------------------------------------------------
# Configurable options

option(PSAPI_BUILD_STATIC "Build a static library version of the PhotoshopAPI" ON)
option(PSAPI_BUILD_TESTS "Build the tests associated with the PhotoshopAPI" ON)
option(PSAPI_BUILD_EXAMPLES "Build the examples associated with the PhotoshopAPI" ON)
option(PSAPI_BUILD_BENCHMARKS "Build the benchmarks associated with the PhotoshopAPI" OFF)
option(PSAPI_BUILD_DOCS "Builds the documentation, requires some external installs which are documented in the README.md" OFF)
option(PSAPI_BUILD_PYTHON "Build the python bindings associated with the PhotoshopAPI" OFF)

# Build setup
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set (CMAKE_CXX_STANDARD 20)

# Add the cmake/ folder so the FindSphinx module is found
# --------------------------------------------------------------------------
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
# Link in the msvc runtime so that users dont have to have it dynamically loaded
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Add thirdparty libraries
# --------------------------------------------------------------------------

# Add zlib-ng
set(ZLIB_ENABLE_TESTS OFF)
set(ZLIBNG_ENABLE_TESTS OFF)
set(WITH_GZFILEOP OFF)
add_subdirectory (thirdparty/zlib-ng EXCLUDE_FROM_ALL)

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
target_include_directories(blosc2_include INTERFACE thirdparty/c-blosc2/include)

# Add doctest
add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE thirdparty/doctest/doctest)

# Add span from tcb for compatibility with older compilers for python bindings
add_library(tcb_span INTERFACE)
target_include_directories(tcb_span INTERFACE thirdparty/compatibility)

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