# Define the project
# --------------------------------------------------------------------------
project(PhotoshopAPI)
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_library(PhotoshopAPI STATIC ${MY_SOURCES})

set_property(TARGET PhotoshopAPI PROPERTY POSITION_INDEPENDENT_CODE 1)
set_property(TARGET zlibstatic PROPERTY POSITION_INDEPENDENT_CODE 1)
set_property(TARGET blosc2_static PROPERTY POSITION_INDEPENDENT_CODE 1)

target_include_directories(PhotoshopAPI PUBLIC include src src/Util)
target_link_libraries(PhotoshopAPI PUBLIC zlibstatic Blosc2::blosc2_static blosc2_include tcb_span)

if(MSVC)
	target_compile_options(PhotoshopAPI PRIVATE /MP /DNOMINMAX)
	target_compile_options(PhotoshopAPI PUBLIC /arch:AVX2)
else()
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
    if(COMPILER_SUPPORTS_AVX2)
        target_compile_options(PhotoshopAPI PUBLIC -mavx2)
    elseif(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
        target_compile_options(PhotoshopAPI PUBLIC -no-pie)
    endif()
endif()