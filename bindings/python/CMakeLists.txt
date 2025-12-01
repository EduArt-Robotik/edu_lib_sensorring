# Setup
set(EDU_PYTHON_PACKAGE_NAME "eduart")
set(EDU_PYTHON_MODULE_NAME  ${CMAKE_PROJECT_NAME})
set(EDU_PYTHON_OUTPUT_DIR   ${CMAKE_CURRENT_BINARY_DIR})

# Conversion doxygen to docstrings
if(${SWIG_VERSION} VERSION_GREATER_EQUAL 4.0.0)
 list(APPEND CMAKE_SWIG_FLAGS "-doxygen")
endif()

# Find Python 3
if(IS_WINDOWS)
  # Use the stable ABI development module on Windows
  find_package(Python3 REQUIRED COMPONENTS Interpreter Development.SABIModule NumPy)
else()
  find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module NumPy)
endif()

list(APPEND CMAKE_SWIG_FLAGS "-DPY3")

set_property(SOURCE ${CMAKE_PROJECT_NAME}.i PROPERTY CPLUSPLUS ON)
set_property(SOURCE ${CMAKE_PROJECT_NAME}.i PROPERTY SWIG_MODULE_NAME ${EDU_PYTHON_MODULE_NAME})

# Generate bindings
swig_add_library(
  ${CMAKE_PROJECT_NAME}_python
    TYPE MODULE
    LANGUAGE python
    OUTPUT_DIR  ${EDU_PYTHON_OUTPUT_DIR}/package/${EDU_PYTHON_PACKAGE_NAME}
    OUTFILE_DIR ${EDU_PYTHON_OUTPUT_DIR}/cpp
    SOURCES     ${CMAKE_PROJECT_NAME}.i
)
add_library(${CMAKE_PROJECT_NAME}::python ALIAS ${CMAKE_PROJECT_NAME}_python)

target_include_directories(
  ${CMAKE_PROJECT_NAME}_python
    PRIVATE
      ${Python3_INCLUDE_DIRS}
)

target_link_libraries(
  ${CMAKE_PROJECT_NAME}_python 
    PRIVATE 
      Python3::NumPy
      $<$<BOOL:${IS_WINDOWS}>:Python3::SABIModule>
      ${CMAKE_PROJECT_NAME}
)

if(IS_WINDOWS)
  # Ensure Python and NumPy downwards compatibility on Windows
  # Minimum versions: Python >= 3.9 and NumPy >= 2.0.0
  target_compile_definitions(
    ${CMAKE_PROJECT_NAME}_python
      PRIVATE
        Py_LIMITED_API=0x03090000
        NPY_TARGET_VERSION=NPY_2_00_API_VERSION
  )
endif()

set_target_properties(
  ${CMAKE_PROJECT_NAME}_python 
    PROPERTIES
      SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON
      POSITION_INDEPENDENT_CODE ON
)

file(GENERATE OUTPUT ${EDU_PYTHON_OUTPUT_DIR}/package/${EDU_PYTHON_PACKAGE_NAME}/__init__.py CONTENT "__version__ = \"${PROJECT_VERSION}\"\n")


# Install
install(
  DIRECTORY
    ${EDU_PYTHON_OUTPUT_DIR}/package/${EDU_PYTHON_PACKAGE_NAME}
  DESTINATION ${SENSORRING_INSTALL_PYTHON_DIR} COMPONENT Python3
)

install(
  TARGETS
    ${CMAKE_PROJECT_NAME}_python
  RUNTIME DESTINATION ${SENSORRING_INSTALL_PYTHON_DIR}/${EDU_PYTHON_PACKAGE_NAME} COMPONENT Python3
  ARCHIVE DESTINATION ${SENSORRING_INSTALL_PYTHON_DIR}/${EDU_PYTHON_PACKAGE_NAME} COMPONENT Python3
  LIBRARY DESTINATION ${SENSORRING_INSTALL_PYTHON_DIR}/${EDU_PYTHON_PACKAGE_NAME} COMPONENT Python3
)