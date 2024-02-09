# Define the project
# --------------------------------------------------------------------------
set(LIB PhotoshopAPI)
project(${LIB})

# Define MY_SOURCES to be a list of all the source files
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")

# Copy the header files from /include and /src to the binary dir
# --------------------------------------------------------------------------

# Set the source directory
set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
# Gather a list of all header files
file(GLOB_RECURSE HEADER_FILES ${SOURCE_DIR}/*.h)
# Set the include dir in our binary
set(BIN_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)

# Custom target for header file copying
add_custom_target(CopyHeaders
    COMMENT "Copying header files to include directory"
    VERBATIM
)

# Copy header files to the include directory with folder structure intact
foreach(HEADER_FILE ${HEADER_FILES})
    # Get the relative path of the header file
    file(RELATIVE_PATH RELATIVE_HEADER ${SOURCE_DIR} ${HEADER_FILE})

    # Create the corresponding include directory for the header file
    get_filename_component(INCLUDE_SUBDIR ${RELATIVE_HEADER} DIRECTORY)
    set(DESTINATION_DIR ${BIN_INCLUDE_DIR}/${INCLUDE_SUBDIR})
    file(MAKE_DIRECTORY ${DESTINATION_DIR})

    # Copy the header file to the include directory
    add_custom_command(TARGET CopyHeaders
        COMMAND ${CMAKE_COMMAND} -E copy ${HEADER_FILE} ${DESTINATION_DIR}
        COMMENT "Copying ${HEADER_FILE} to ${DESTINATION_DIR}"
    )
endforeach()

# Copy the /include/PhotoshopAPI.h separately
add_custom_command(TARGET CopyHeaders
    COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/include/PhotoshopAPI.h" ${BIN_INCLUDE_DIR}
    COMMENT "Copying ${PROJECT_SOURCE_DIR}/include/PhotoshopAPI.h to ${BIN_INCLUDE_DIR}"
)

# Create our static lib
# --------------------------------------------------------------------------
add_library (${LIB} STATIC ${MY_SOURCES})

# Add dependency to CopyHeaders
add_dependencies(${LIB} CopyHeaders)

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