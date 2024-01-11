set(LIB PhotoshopAPI)
project(${LIB})

# Define MY_SOURCES to be a list of all the source files
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_library (${LIB} STATIC ${MY_SOURCES})

# Set some additional includes 
target_include_directories(${LIB} PUBLIC
	"${PROJECT_SOURCE_DIR}/src/"
	"${PROJECT_SOURCE_DIR}/src/Util/"
)

# Include blosc2 headers
target_include_directories(${LIB} PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/c-blosc2/include")

#include zlib-ng headers
target_include_directories(${LIB} PUBLIC "${CMAKE_BINARY_DIR}/thirdparty/zlib-ng")
target_include_directories(${LIB} PUBLIC "${CMAKE_SOURCE_DIR}/thirdparty/zlib-ng")


if(MSVC)
	target_compile_options(${LIB} PRIVATE /MP /DNOMINMAX)
endif()