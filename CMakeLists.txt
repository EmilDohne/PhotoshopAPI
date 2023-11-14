# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.15)
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

# Include sub-projects.
add_subdirectory ("${CMAKE_SOURCE_DIR}/PhotoshopAPI")
add_subdirectory ("${CMAKE_SOURCE_DIR}/PhotoshopTest")

# Add thirdparty libraries
add_subdirectory ("${CMAKE_SOURCE_DIR}/thirdparty/c-blosc2")

# Set the test project (PhotoshopTest) as the default target
add_custom_target(default_target ALL
    DEPENDS PhotoshopTest
    COMMENT "Building the default target: PhotoshopTest"
)