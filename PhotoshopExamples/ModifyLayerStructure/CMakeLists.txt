add_executable(ModifyLayerStructure main.cpp)
target_link_libraries(ModifyLayerStructure PRIVATE PhotoshopAPI)


# Copy the LayeredFile.psd to our binary directory
add_custom_command(TARGET ModifyLayerStructure POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                       ${CMAKE_CURRENT_SOURCE_DIR}/LayeredFile.psd $<TARGET_FILE_DIR:ModifyLayerStructure>/LayeredFile.psd)