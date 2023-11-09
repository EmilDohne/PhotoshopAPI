set (EXE PhotoshopTest)
project(${EXE})

add_executable(${EXE} ${CMAKE_SOURCE_DIR}/PhotoshopTest/src/main.cpp)
#include and link the PhotoshopAPI
include_directories(${CMAKE_SOURCE_DIR}/PhotoshopAPI/src)
target_link_libraries(${EXE} PRIVATE PhotoshopAPI)

if(MSVC)
 target_compile_options(${EXE} PRIVATE "/MP")
endif()
