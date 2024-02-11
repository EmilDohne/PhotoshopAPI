
# Define MY_SOURCES to be a list of all the source files
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(PhotoshopTest ${MY_SOURCES})
if(MSVC)
    target_compile_options(PhotoshopTest PRIVATE /MP)
endif()

target_link_libraries(PhotoshopTest PRIVATE PhotoshopAPI doctest)
