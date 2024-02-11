project(PhotoshopAPI)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_library(PhotoshopAPI STATIC ${MY_SOURCES})
target_include_directories(PhotoshopAPI PUBLIC include src src/Util)
target_link_libraries(PhotoshopAPI PUBLIC zlibstatic Blosc2::blosc2_static)

if(MSVC)
	target_compile_options(PhotoshopAPI PRIVATE /MP /DNOMINMAX)
endif()
