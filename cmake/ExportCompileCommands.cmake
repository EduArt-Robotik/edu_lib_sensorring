# ExportCompileCommands.cmake
#
# Maintain a symlink <source-root>/compile_commands.json pointing at the
# active build tree's compile_commands.json so that clangd auto-discovers
# the currently configured build without a hard-coded CompilationDatabase
# path in .clangd. The link is refreshed on every configure.
#
# Include this module AFTER CMAKE_EXPORT_COMPILE_COMMANDS has been set and
# BEFORE project() is strictly not required, but CMAKE_BINARY_DIR and
# CMAKE_SOURCE_DIR must already be known (they are, at the top of the
# top-level CMakeLists.txt).

if(NOT CMAKE_EXPORT_COMPILE_COMMANDS)
    return()
endif()

set(_cdb_src "${CMAKE_BINARY_DIR}/compile_commands.json")
set(_cdb_dst "${CMAKE_SOURCE_DIR}/compile_commands.json")

file(REMOVE "${_cdb_dst}")
file(CREATE_LINK "${_cdb_src}" "${_cdb_dst}" SYMBOLIC RESULT _cdb_link_result)
if(NOT _cdb_link_result STREQUAL "0")
    message(WARNING
        "Could not symlink ${_cdb_dst} -> ${_cdb_src}: ${_cdb_link_result}. "
        "clangd may not pick up the active build's compile_commands.json."
    )
endif()

unset(_cdb_src)
unset(_cdb_dst)
unset(_cdb_link_result)
