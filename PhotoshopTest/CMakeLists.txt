# Initialize our Project
set (EXE PhotoshopTest)
project(${EXE})

add_executable(${EXE} 
    "${PROJECT_SOURCE_DIR}/src/main.cpp"
    "${PROJECT_SOURCE_DIR}/src/TestCompression/Utility.cpp"
    "${PROJECT_SOURCE_DIR}/src/TestCompression/TestRLE.cpp"
    "${PROJECT_SOURCE_DIR}/src/TestCompression/TestZIPPrediction.cpp"
    "${PROJECT_SOURCE_DIR}/src/TestGroups/TestLayerDetection.cpp"
)

target_include_directories(${EXE} PRIVATE "${CMAKE_SOURCE_DIR}/PhotoshopTest/src/TestCompression")


# Unfortunately, Wall doesnt compile with blosc2 due to its use of getenv
if(MSVC)
 	target_compile_options(${EXE} PRIVATE /MP /Ox)
# 	target_compile_options(${EXE} PRIVATE /W4 /WX)
# else()
# 	target_compile_options(${EXE} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Include and link the PhotoshopAPI
target_include_directories(${EXE} PRIVATE "${CMAKE_SOURCE_DIR}/PhotoshopAPI/src")
target_link_libraries(${EXE} PRIVATE PhotoshopAPI)

# include doctest v2.4.11 library
target_include_directories(${EXE} PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/doctest/")

# Include and link blosc2
target_include_directories(${EXE} PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/c-blosc2/include")
target_link_directories(${EXE} PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/c-blosc2")
target_link_libraries(${EXE} PRIVATE blosc2_static)

#include zlib-ng headers
target_include_directories(${EXE} PUBLIC "${CMAKE_BINARY_DIR}/thirdparty/zlib-ng")
target_include_directories(${EXE} PUBLIC "${CMAKE_SOURCE_DIR}/thirdparty/zlib-ng")
target_link_directories(${EXE} PUBLIC "${CMAKE_SOURCE_DIR}/thirdparty/zlib-ng/Release")
target_link_libraries(${EXE} PUBLIC zlibstatic-ng)


# Copy the documents/ folder to the build dir
add_custom_command(TARGET ${EXE} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${PROJECT_SOURCE_DIR}/documents/ $<TARGET_FILE_DIR:${EXE}>/documents/)

