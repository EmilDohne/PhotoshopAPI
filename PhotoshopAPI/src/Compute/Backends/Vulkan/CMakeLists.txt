# CMake Target for compiling our shaders into spirv

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

file (GLOB SHADERS
	${SHADER_SOURCE_DIR}/*.comp
)

# Create the binary dir for the shaders to go into
add_custom_command(
	COMMAND
	${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
	OUTPUT ${SHADER_BINARY_DIR}
	COMMENT "Creating ${SHADER_BINARY_DIR}"
)

# Compile each shader
foreach(source IN LISTS SHADERS)
  get_filename_component(FILENAME ${source} NAME)
  add_custom_command(
    COMMAND
      ${glslc_executable}
      -MD -MF ${SHADER_BINARY_DIR}/${FILENAME}.d
      -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
      ${source}
    OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
    DEPENDS ${source} ${SHADER_BINARY_DIR}
    COMMENT "Compiling ${FILENAME}"
  )
  list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
endforeach()