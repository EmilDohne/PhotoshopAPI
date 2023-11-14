set (EXE PhotoshopTest)
project(${EXE})

add_executable(${EXE} ${PROJECT_SOURCE_DIR}/src/main.cpp)

# Unfortunately, these dont compile with blosc2 due to its use of getenv
# if(MSVC)
# 	target_compile_options(${LIB} PRIVATE "/MP")
# 	target_compile_options(${LIB} PRIVATE /W4 /WX)
# else()
# 	target_compile_options(${LIB} PRIVATE -Wall -Wextra -Wpedantic -Werror)
# endif()

# Include and link the PhotoshopAPI
include_directories(${CMAKE_SOURCE_DIR}/PhotoshopAPI/src)
target_link_libraries(${EXE} PRIVATE PhotoshopAPI)

# Include and link blosc2
include_directories("${CMAKE_SOURCE_DIR}/thirdparty/c-blosc2/include")
target_link_libraries(${EXE} PRIVATE blosc2_static)


# Copy the documents/ folder to the build dir
add_custom_command(TARGET ${EXE} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${PROJECT_SOURCE_DIR}/documents/ $<TARGET_FILE_DIR:${EXE}>/documents/)

