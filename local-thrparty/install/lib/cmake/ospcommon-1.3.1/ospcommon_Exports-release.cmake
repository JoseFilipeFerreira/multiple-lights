#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ospcommon::ospcommon" for configuration "Release"
set_property(TARGET ospcommon::ospcommon APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ospcommon::ospcommon PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libospcommon.so.1.3.1"
  IMPORTED_SONAME_RELEASE "libospcommon.so.1"
  )

list(APPEND _IMPORT_CHECK_TARGETS ospcommon::ospcommon )
list(APPEND _IMPORT_CHECK_FILES_FOR_ospcommon::ospcommon "${_IMPORT_PREFIX}/lib/libospcommon.so.1.3.1" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
