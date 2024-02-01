# This is largely adopted by the following tutorial: https://devblogs.microsoft.com/cppblog/clear-functional-c-documentation-with-sphinx-breathe-doxygen-cmake/

find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED)

# Find all the public headers
# --------------------------------------------------------------------------
get_target_property(PHOTOSHOP_API_PUBLIC_HEADER_DIR PhotoshopAPI INTERFACE_INCLUDE_DIRECTORIES)
file(GLOB_RECURSE PHOTOSHOP_API_PUBLIC_HEADERS ${CAT_CUTIFIER_PUBLIC_HEADER_DIR}/*.h)

set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR}/PhotoshopAPI)
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

#Replace variables inside @@ with the current values
# --------------------------------------------------------------------------
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

#Doxygen won't create this for us
file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR}) 
# Only regenerate Doxygen when the Doxyfile or public headers change
add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                   DEPENDS ${PHOTOSHOP_API_PUBLIC_HEADERS}
                   COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                   MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
                   COMMENT "Generating docs"
                   VERBATIM)

add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})


# Generate Sphinx data
# --------------------------------------------------------------------------
set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/docs/sphinx)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

# Find all .rst files in the source directory and its subdirectories
file(GLOB_RECURSE RST_FILES CONFIGURE_DEPENDS ${SPHINX_SOURCE}/*.rst)

# Only regenerate Sphinx when:
# - Doxygen has rerun
# - Our doc *.rst files have been updated
# - The Sphinx config has been updated

add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
                  COMMAND ${SPHINX_EXECUTABLE} -b html
                  # Tell Breathe where to find the Doxygen output
                  -Dbreathe_projects.PhotoshopAPI=${DOXYGEN_OUTPUT_DIR}
                  ${SPHINX_SOURCE} ${SPHINX_BUILD}
                  DEPENDS 
                  ${RST_FILES}
                  ${DOXYGEN_INDEX_FILE}
                  MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                  COMMENT "Generating documentation with Sphinx")

add_custom_target(Sphinx ALL DEPENDS ${SPHINX_INDEX_FILE})