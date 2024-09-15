# Define the project
# --------------------------------------------------------------------------
project(PhotoshopAPIPython)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

# Link in the msvc runtime so that users dont need vcredist when using the python bindings
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Compile simdutf with position independent code as it otherwise wont compile on gcc ubuntu
set_property(TARGET simdutf PROPERTY POSITION_INDEPENDENT_CODE ON)

pybind11_add_module(psapi src/main.cpp)
target_link_libraries(psapi PUBLIC PhotoshopAPI)