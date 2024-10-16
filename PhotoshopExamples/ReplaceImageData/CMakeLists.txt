add_executable(ReplaceImageData main.cpp)
target_link_libraries(ReplaceImageData PRIVATE PhotoshopAPI)


# Copy the LayeredFile.psd to our binary directory
add_custom_command(TARGET ReplaceImageData POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                       ${CMAKE_CURRENT_SOURCE_DIR}/ImageData.psb $<TARGET_FILE_DIR:ReplaceImageData>/ImageData.psb)