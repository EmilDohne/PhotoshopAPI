add_executable(ExtendedSignature main.cpp)
target_link_libraries(ExtendedSignature PRIVATE PhotoshopAPI)


# Copy the LayeredFile.psd to our binary directory
add_custom_command(TARGET ExtendedSignature POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                       ${CMAKE_CURRENT_SOURCE_DIR}/LayeredFile.psd $<TARGET_FILE_DIR:ExtendedSignature>/LayeredFile.psd)