cmake_minimum_required (VERSION 3.20)
project ("PhotoshopAPIBuild" VERSION 0.1.0 LANGUAGES CXX)

# --------------------------------------------------------------------------
# Configurable options

option(PSAPI_BUILD_STATIC "Build a static library version of the PhotoshopAPI" ON)
option(PSAPI_BUILD_TESTS "Build the tests associated with the PhotoshopAPI" ON)
option(PSAPI_BUILD_EXAMPLES "Build the examples associated with the PhotoshopAPI" ON)
option(PSAPI_BUILD_BENCHMARKS "Build the benchmarks associated with the PhotoshopAPI" ON)
option(PSAPI_BUILD_DOCS "Builds the documentation, requires some external installs which are documented in the README.md" ON)

# Build setup
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set (CMAKE_CXX_STANDARD 20)

# Add the cmake/ folder so the FindSphinx module is found
# --------------------------------------------------------------------------
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})


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

# Add doctest
add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE thirdparty/doctest/doctest)

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
    add_subdirectory (docs/doxygen)
endif()