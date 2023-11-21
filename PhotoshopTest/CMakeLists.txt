# Initialize our Project
set (EXE PhotoshopTest)
project(${EXE})

add_executable(${EXE} ${PROJECT_SOURCE_DIR}/src/main.cpp)

# Unfortunately, Wall doesnt compile with blosc2 due to its use of getenv
if(MSVC)
 	target_compile_options(${EXE} PRIVATE "/MP")
# 	target_compile_options(${EXE} PRIVATE /W4 /WX)
# else()
# 	target_compile_options(${EXE} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Include and link the PhotoshopAPI
target_include_directories(${EXE} PRIVATE "${CMAKE_SOURCE_DIR}/PhotoshopAPI/src")
target_link_libraries(${EXE} PRIVATE PhotoshopAPI)


# Include and link blosc2
target_include_directories(${EXE} PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/c-blosc2/include")
target_link_directories(${EXE} PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/c-blosc2")
target_link_libraries(${EXE} PRIVATE blosc2_static)

#include and link zlib-ng headers
target_include_directories(${EXE} PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/c-blosc2/internal-complibs/zlib-ng-2.0.7")
target_link_libraries(${EXE} PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/c-blosc2/internal-complibs/zlib-ng-2.0.7/Release/zlibstatic-ng.lib")


# Copy the documents/ folder to the build dir
add_custom_command(TARGET ${EXE} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${PROJECT_SOURCE_DIR}/documents/ $<TARGET_FILE_DIR:${EXE}>/documents/)

