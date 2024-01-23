# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.20)
set (CMAKE_CXX_STANDARD 20)

if(MSVC)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("PhotoshopAPIBuild")


# Specify our own version of zlib-ng rather than the supplied one for blosc2.
# Switch to the debug library if that is the build target
set(PREFER_EXTERNAL_ZLIB ON)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set (ZLIB_NG_LIBRARY "${CMAKE_BINARY_DIR}/thirdparty/zlib-ng/zlibstatic-ng.lib")
else()
    set (ZLIB_NG_LIBRARY "${CMAKE_BINARY_DIR}/thirdparty/zlib-ng/zlibstatic-ngd.lib")
endif()
set (ZLIB_NG_INCLUDE_DIR "${CMAKE_BINARY_DIR}/thirdparty/zlib-ng")

# Add thirdparty libraries
add_subdirectory ("${CMAKE_SOURCE_DIR}/thirdparty/zlib-ng" EXCLUDE_FROM_ALL)
add_subdirectory ("${CMAKE_SOURCE_DIR}/thirdparty/c-blosc2" EXCLUDE_FROM_ALL)

# Include PhotoshopAPI sub-projects.
add_subdirectory ("${CMAKE_SOURCE_DIR}/PhotoshopAPI")
add_subdirectory ("${CMAKE_SOURCE_DIR}/PhotoshopTest")

# Add our Example directories
add_subdirectory ("${CMAKE_SOURCE_DIR}/PhotoshopExamples/CreateSimpleDocument")
add_subdirectory ("${CMAKE_SOURCE_DIR}/PhotoshopExamples/CreateGroups")
add_subdirectory ("${CMAKE_SOURCE_DIR}/PhotoshopExamples/AddLayerMasks")
add_subdirectory ("${CMAKE_SOURCE_DIR}/PhotoshopExamples/ModifyLayerStructure")

# Set the test project (PhotoshopTest) as the default target
add_custom_target(default_target ALL
    DEPENDS PhotoshopTest
    COMMENT "Building the default target: PhotoshopTest"
)