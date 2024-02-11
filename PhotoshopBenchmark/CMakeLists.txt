
# Define MY_SOURCES to be a list of all the source files
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(PhotoshopBenchmark ${MY_SOURCES})
target_link_libraries(PhotoshopBenchmark PRIVATE PhotoshopAPI)
