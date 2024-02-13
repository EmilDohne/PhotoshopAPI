add_executable(ExtractImageData main.cpp)
target_link_libraries(ExtractImageData PRIVATE PhotoshopAPI)


# Copy the LayeredFile.psd to our binary directory
add_custom_command(TARGET ExtractImageData POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                       ${CMAKE_CURRENT_SOURCE_DIR}/ImageData.psb $<TARGET_FILE_DIR:ExtractImageData>/ImageData.psb)