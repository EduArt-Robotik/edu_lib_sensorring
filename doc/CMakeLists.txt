#=====================================
# Generate doxygen documentation
#=====================================

# Locate doxygen with optional graphviz tool to generate diagrams
find_package(Doxygen REQUIRED)
find_package(Doxygen OPTIONAL_COMPONENTS dot)

set(DOXYGEN_CONFIG_IN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/doxygen.config.in")
set(DOXYGEN_CONFIG_FILE    "${CMAKE_CURRENT_BINARY_DIR}/doxygen.config")

# Find all image files in the images directory
file(GLOB DOXYGEN_IMAGES "${CMAKE_CURRENT_SOURCE_DIR}/images/*")

# Convert the full paths to relative paths for Doxygen
set(DOXYGEN_IMAGES_REL "")
foreach(img ${DOXYGEN_IMAGES})
    # Assuming your Doxygen config is in the source dir
    file(RELATIVE_PATH rel_path "${CMAKE_CURRENT_SOURCE_DIR}" "${img}")
    list(APPEND DOXYGEN_IMAGES_REL "${rel_path}")
endforeach()

# Join into a single string with spaces
string(JOIN " " DOXYGEN_IMAGES_LIST ${DOXYGEN_IMAGES_REL})

configure_file(${DOXYGEN_CONFIG_IN_FILE} ${DOXYGEN_CONFIG_FILE} @ONLY)

add_custom_target(
  doc ALL
  COMMAND
    ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIG_FILE}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Generating documentation..."
  VERBATIM
)

install(
  DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/html
  DESTINATION /share/doc/sensorring COMPONENT Documentation
)
