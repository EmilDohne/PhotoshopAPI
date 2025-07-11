cmake_minimum_required(VERSION 3.15...3.27)

# Define the project
# --------------------------------------------------------------------------
project(PhotoshopAPIPython)

# Compile simdutf with position independent code as it otherwise wont compile on gcc ubuntu
set_property(TARGET simdutf PROPERTY POSITION_INDEPENDENT_CODE ON)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(photoshopapi_py src/main.cpp)

set_target_properties(photoshopapi_py PROPERTIES OUTPUT_NAME photoshopapi)
target_include_directories(photoshopapi_py PUBLIC src)
target_link_libraries(photoshopapi_py PRIVATE pybind11::headers PhotoshopAPI)

install(TARGETS photoshopapi_py
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})