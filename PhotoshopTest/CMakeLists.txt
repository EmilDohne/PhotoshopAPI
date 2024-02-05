# Initialize our Project
set (EXE PhotoshopTest)
project(${EXE})

# Define MY_SOURCES to be a list of all the source files
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_executable(${EXE} ${MY_SOURCES})

# Unfortunately, Wall doesnt compile with blosc2 due to its use of getenv
if(MSVC)
 	target_compile_options(${EXE} PRIVATE /MP)
endif()

# Include and link the PhotoshopAPI
target_include_directories(${EXE} PRIVATE "${CMAKE_SOURCE_DIR}/PhotoshopAPI/include")
target_link_libraries(${EXE} PRIVATE PhotoshopAPI)

# include doctest v2.4.11 library
target_include_directories(${EXE} PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/doctest/")

# Include and link blosc2
target_include_directories(${EXE} PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/c-blosc2/include")
target_link_directories(${EXE} PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/c-blosc2")
target_link_libraries(${EXE} PRIVATE blosc2_static)


#include and link zlib-ng 
target_include_directories(${EXE} PUBLIC "${CMAKE_BINARY_DIR}/thirdparty/zlib-ng")
target_include_directories(${EXE} PUBLIC "${CMAKE_SOURCE_DIR}/thirdparty/zlib-ng")
target_link_directories(${EXE} PUBLIC "${CMAKE_BINARY_DIR}/thirdparty/zlib-ng")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_link_libraries(${EXE} PUBLIC zlibstatic-ng)
else()
    target_link_libraries(${EXE} PUBLIC zlibstatic-ngd)
endif()


# Copy the documents/ folder to the build dir
add_custom_command(TARGET ${EXE} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${PROJECT_SOURCE_DIR}/documents/ $<TARGET_FILE_DIR:${EXE}>/documents/)