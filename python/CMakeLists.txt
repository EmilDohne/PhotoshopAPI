# Define the project
# --------------------------------------------------------------------------
project(PhotoshopAPIPython)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

pybind11_add_module(psapi NO_EXTRAS src/main.cpp)
target_link_libraries(psapi PUBLIC PhotoshopAPI)