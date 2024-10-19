# Define the project
# --------------------------------------------------------------------------
project(PhotoshopAPIPython)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

# Compile simdutf with position independent code as it otherwise wont compile on gcc ubuntu
set_property(TARGET simdutf PROPERTY POSITION_INDEPENDENT_CODE ON)

pybind11_add_module(psapi src/main.cpp)
target_include_directories(psapi PUBLIC src)
target_link_libraries(psapi PUBLIC PhotoshopAPI)