# PhotoshopAPI Requires C++ 20
cmake_minimum_required (VERSION 3.20)
set (CMAKE_CXX_STANDARD 20)

if(MSVC)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Link the MSVC Runtime libraries into the exe
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# --------------------------------------------------------------------------
# CMake Build Setup for the PhotoshopAPI
# --------------------------------------------------------------------------
#
# A list of available options can be found below:
#
# PSAPI_BUILD_STATIC: default ON
#   Build a static library version of the PhotoshopAPI
# PSAPI_BUILD_TESTS: default ON
#   Build the tests associated with the PhotoshopAPI
# PSAPI_BUILD_EXAMPLES: default ON
#   Build the examples associated with the PhotoshopAPI
# PSAPI_BUILD_BENCHMARKS: default ON
#   Build the benchmarks associated with the PhotoshopAPI
# PSAPI_BUILD_DOCS: default ON
#   Builds the documentation, requires some external installs which are documented in the README.md
#
project ("PhotoshopAPIBuild")

# Initialize our options
option(PSAPI_BUILD_STATIC "Build a static library version of the PhotoshopAPI" ON)
option(PSAPI_BUILD_TESTS "Build the tests associated with the PhotoshopAPI" ON)
option(PSAPI_BUILD_EXAMPLES "Build the examples associated with the PhotoshopAPI" ON)
option(PSAPI_BUILD_BENCHMARKS "Build the benchmarks associated with the PhotoshopAPI" ON)
option(PSAPI_BUILD_DOCS "Builds the documentation, requires some external installs which are documented in the README.md" ON)


# Add the cmake/ folder so the FindSphinx module is found
# --------------------------------------------------------------------------
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Specify our own version of zlib-ng rather than the supplied one for blosc2.
# Switch to the debug library if that is the build target
# --------------------------------------------------------------------------
set(PREFER_EXTERNAL_ZLIB ON)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set (ZLIB_NG_LIBRARY "${CMAKE_BINARY_DIR}/thirdparty/zlib-ng/zlibstatic-ng.lib")
else()
    set (ZLIB_NG_LIBRARY "${CMAKE_BINARY_DIR}/thirdparty/zlib-ng/zlibstatic-ngd.lib")
endif()
set (ZLIB_NG_INCLUDE_DIR "${CMAKE_BINARY_DIR}/thirdparty/zlib-ng")


# Add thirdparty libraries
# --------------------------------------------------------------------------

# Modify zlib-ng to only build libs
set(ZLIB_ENABLE_TESTS OFF)
set(ZLIBNG_ENABLE_TESTS OFF)
set(WITH_GZFILEOP OFF)
add_subdirectory ("${CMAKE_SOURCE_DIR}/thirdparty/zlib-ng" EXCLUDE_FROM_ALL)

# Modify c-blosc2 to only build libs
set(BUILD_TESTS OFF)
set(BUILD_FUZZERS OFF)
set(BUILD_BENCHMARKS OFF)
set(BUILD_EXAMPLES OFF)
add_subdirectory ("${CMAKE_SOURCE_DIR}/thirdparty/c-blosc2" EXCLUDE_FROM_ALL)

# Include PhotoshopAPI sub-projects.
# --------------------------------------------------------------------------
if(PSAPI_BUILD_STATIC)
    add_subdirectory ("${CMAKE_SOURCE_DIR}/PhotoshopAPI")
endif()
if(PSAPI_BUILD_TESTS)
    add_subdirectory ("${CMAKE_SOURCE_DIR}/PhotoshopTest")
endif()
if(PSAPI_BUILD_BENCHMARKS)
    add_subdirectory ("${CMAKE_SOURCE_DIR}/PhotoshopBenchmark")
endif()

# Add our Example directories
# --------------------------------------------------------------------------
if(PSAPI_BUILD_EXAMPLES)
    add_subdirectory ("${CMAKE_SOURCE_DIR}/PhotoshopExamples/CreateSimpleDocument")
    add_subdirectory ("${CMAKE_SOURCE_DIR}/PhotoshopExamples/CreateGroups")
    add_subdirectory ("${CMAKE_SOURCE_DIR}/PhotoshopExamples/AddLayerMasks")
    add_subdirectory ("${CMAKE_SOURCE_DIR}/PhotoshopExamples/ModifyLayerStructure")
endif()

# Add our documentation dir
# --------------------------------------------------------------------------
if(PSAPI_BUILD_DOCS)
    add_subdirectory ("${CMAKE_SOURCE_DIR}/docs/doxygen")
endif()


# Set the test project (PhotoshopTest) as the default target
# --------------------------------------------------------------------------
add_custom_target(default_target ALL
    DEPENDS PhotoshopTest
    COMMENT "Building the default target: PhotoshopTest"
)