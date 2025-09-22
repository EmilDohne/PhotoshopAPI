# Define the project
# --------------------------------------------------------------------------
project(PhotoshopAPI)
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_library(PhotoshopAPI STATIC ${MY_SOURCES})

set_property(TARGET PhotoshopAPI PROPERTY POSITION_INDEPENDENT_CODE 1)
set_property(TARGET blosc2_static PROPERTY POSITION_INDEPENDENT_CODE 1)

target_include_directories(PhotoshopAPI PUBLIC include src)
target_include_directories(PhotoshopAPI PRIVATE src/Util)
target_link_libraries(PhotoshopAPI PUBLIC 
    OpenImageIO::OpenImageIO
    Eigen3::Eigen
    mio::mio-headers 
    tcb_span 
    compressed_image
    $<IF:$<TARGET_EXISTS:libdeflate::libdeflate_static>,libdeflate::libdeflate_static,libdeflate::libdeflate_shared>
    simdutf
    fmt::fmt
    stduuid
)

if(MSVC)
	target_compile_options(PhotoshopAPI PRIVATE /MP /DNOMINMAX)
	target_compile_options(PhotoshopAPI PUBLIC /arch:AVX2 /Zc:__cplusplus /utf-8 /bigobj)
    # Bump up warning levels and enable the following extra exceptions:
    # w44062 is for if a switch misses some enum members.
    # w44464 is for #include containing .. in the path
    # w45264 is for using std:move when returning a temporary.
    target_compile_options(PhotoshopAPI PRIVATE /W4 /WX /wd4702 /w44062 /w44464 /w45264)
else()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(PhotoshopAPI PUBLIC -O3 -Wuninitialized -Wmaybe-uninitialized)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
        
        # AppleClang only partially supports the parallel standard library (std::for_each(std::execution::par))
        # and it is hidden by the compile/link flag -fexperimental-library. As PhotoshopAPI requires this, we expose it 
        # as a public flag
        target_compile_options(
		    PhotoshopAPI 
		    PUBLIC 
		    -fexperimental-library
		    -Wno-pass-failed # Silence warnings about 'failed to transform' when clang cannot vectorize a loop.
            -O3
		    -Wuninitialized 
            -Wmaybe-uninitialized
	    )
        target_link_options(PhotoshopAPI PUBLIC -fexperimental-library)
    else()
        target_compile_options(PhotoshopAPI PUBLIC -O3)
    endif()
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
    if(COMPILER_SUPPORTS_AVX2)
        target_compile_options(PhotoshopAPI PUBLIC -mavx2)
    endif()
endif()