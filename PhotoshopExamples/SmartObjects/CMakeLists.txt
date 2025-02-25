add_executable(SmartObjects main.cpp)
target_link_libraries(SmartObjects PRIVATE PhotoshopAPI)

# Copy the SmartObject.psd and images to our binary directory
add_custom_command(TARGET SmartObjects POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                       ${CMAKE_CURRENT_SOURCE_DIR}/SmartObject.psd $<TARGET_FILE_DIR:SmartObjects>/SmartObject.psd)
add_custom_command(TARGET SmartObjects POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                       ${CMAKE_CURRENT_SOURCE_DIR}/uv_grid.jpg $<TARGET_FILE_DIR:SmartObjects>/uv_grid.jpg)