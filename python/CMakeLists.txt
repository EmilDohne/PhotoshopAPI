cmake_minimum_required(VERSION 3.15...3.27)

# Define the project
# --------------------------------------------------------------------------
project(PhotoshopAPIPython)

# Compile simdutf with position independent code as it otherwise wont compile on gcc ubuntu
set_property(TARGET simdutf PROPERTY POSITION_INDEPENDENT_CODE ON)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(psapi src/main.cpp)

target_include_directories(psapi PUBLIC src)
target_link_libraries(psapi PRIVATE pybind11::headers PhotoshopAPI)

# Copy the psapi-test/ folder to the build dir to run the tests
add_custom_command(TARGET psapi POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${CMAKE_CURRENT_SOURCE_DIR}/psapi-test/" $<TARGET_FILE_DIR:psapi>)


install(TARGETS psapi LIBRARY DESTINATION psapi)