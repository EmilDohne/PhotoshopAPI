set (EXE PhotoshopTest)
project(${EXE})

add_executable(${EXE} ${PROJECT_SOURCE_DIR}/src/main.cpp)
# Include and link the PhotoshopAPI
include_directories(${CMAKE_SOURCE_DIR}/PhotoshopAPI/src)
target_link_libraries(${EXE} PRIVATE PhotoshopAPI)

# Copy the documents/ folder to the build dir
add_custom_command(TARGET ${EXE} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${PROJECT_SOURCE_DIR}/documents/ $<TARGET_FILE_DIR:${EXE}>/documents/)



if(MSVC)
	target_compile_options(${EXE} PRIVATE "/MP")
	target_compile_options(${EXE} PRIVATE /W4 /WX)
else()
	target_compile_options(${EXE} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()