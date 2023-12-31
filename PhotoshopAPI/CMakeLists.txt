set(LIB PhotoshopAPI)
project(${LIB})

# Add additional source directories here if needed
add_library (${LIB} STATIC 
	"${PROJECT_SOURCE_DIR}/src/PhotoshopFile/ColorModeData.cpp"
	"${PROJECT_SOURCE_DIR}/src/PhotoshopFile/FileHeader.cpp"
	"${PROJECT_SOURCE_DIR}/src/PhotoshopFile/ImageResources.cpp"
	"${PROJECT_SOURCE_DIR}/src/PhotoshopFile/LayerAndMaskInformation.cpp"
	"${PROJECT_SOURCE_DIR}/src/PhotoshopFile/PhotoshopFile.cpp"
	"${PROJECT_SOURCE_DIR}/src/Util/Struct/File.cpp"
	"${PROJECT_SOURCE_DIR}/src/Util/Struct/PascalString.cpp"
	"${PROJECT_SOURCE_DIR}/src/Util/Struct/ResourceBlock.cpp"
	"${PROJECT_SOURCE_DIR}/src/Util/Struct/Signature.cpp"
	"${PROJECT_SOURCE_DIR}/src/Util/Struct/TaggedBlock.cpp"
)


target_include_directories(${LIB} PUBLIC
	"${PROJECT_SOURCE_DIR}/src/"
	"${PROJECT_SOURCE_DIR}/src/Util/"
)

# Include blosc2 headers
target_include_directories(${LIB} PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/c-blosc2/include")

#include zlib-ng headers
target_include_directories(${LIB} PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/c-blosc2/internal-complibs/zlib-ng-2.0.7")

# Unfortunately, Wall doesnt compile with blosc2 due to its use of getenv
if(MSVC)
	target_compile_options(${LIB} PRIVATE "/MP")
# 	target_compile_options(${LIB} PRIVATE /W4 /WX)
# else()
# 	target_compile_options(${LIB} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()