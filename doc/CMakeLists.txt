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
    file(RELATIVE_PATH rel_path "${CMAKE_CURRENT_SOURCE_DIR}" "${img}")
    list(APPEND DOXYGEN_IMAGES_REL "${rel_path}")
endforeach()

# Join into a single string with spaces
string(JOIN " " DOXYGEN_IMAGES_LIST ${DOXYGEN_IMAGES_REL})

#=====================================
# Fetch doxygen-awesome-css
#=====================================
include(FetchContent)
FetchContent_Declare(
    doxygen-awesome-css
    URL https://github.com/jothepro/doxygen-awesome-css/archive/refs/heads/main.zip
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
)
FetchContent_MakeAvailable(doxygen-awesome-css)

# Save the location the files were cloned into
# This allows us to get the path to doxygen-awesome.css
FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

#=====================================
# Generate doxygen configuration
#=====================================
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
  DESTINATION share/doc/sensorring COMPONENT Documentation
)
