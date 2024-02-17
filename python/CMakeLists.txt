# Define the project
# --------------------------------------------------------------------------
project(PhotoshopAPIPython)

pybind11_add_module(psapi src/LayeredFile.cpp)
target_link_libraries(psapi PUBLIC PhotoshopAPI)