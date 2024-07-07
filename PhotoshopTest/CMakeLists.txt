file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

# Enable ctest
enable_testing()

add_executable(PhotoshopTest ${MY_SOURCES})
if(MSVC)
    target_compile_options(PhotoshopTest PRIVATE /MP)
    add_compile_options(/fsanitize=address)
else()
    add_compile_options(-fsanitize=address,leak,undefined)
endif()
target_link_libraries(PhotoshopTest PRIVATE PhotoshopAPI doctest)

include(CTest)
add_test(TestPhotoshopAPI PhotoshopTest)

# Copy the documents/ folder to the build dir to run the tests
add_custom_command(TARGET PhotoshopTest POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_CURRENT_SOURCE_DIR}/documents/ $<TARGET_FILE_DIR:PhotoshopTest>/documents)
add_custom_command(TARGET PhotoshopTest POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E echo 
                    "Finished copying test files to output directory $<TARGET_FILE_DIR:PhotoshopTest>/documents")