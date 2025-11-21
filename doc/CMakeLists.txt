#=====================================
# Generate doxygen documentation
#=====================================

# Locate doxygen with optional graphviz tool to generate diagrams
find_package(Doxygen OPTIONAL_COMPONENTS dot)

message("${CMAKE_CURRENT_BUILD_DIR}")

if(DOXYGEN_FOUND)
  set(DOXYGEN_CONFIG_IN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/doxygen.config.in")
  set(DOXYGEN_CONFIG_FILE    "${CMAKE_CURRENT_BINARY_DIR}/doxygen.config")

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

else()
  message(FATAL_ERROR "Doxygen needs to be installed to generate the documentation.")
endif()