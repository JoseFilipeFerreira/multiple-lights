#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "openvkl::openvkl" for configuration "Release"
set_property(TARGET openvkl::openvkl APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(openvkl::openvkl PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "ospcommon::ospcommon"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libopenvkl.so.0.9.0"
  IMPORTED_SONAME_RELEASE "libopenvkl.so.0"
  )

list(APPEND _IMPORT_CHECK_TARGETS openvkl::openvkl )
list(APPEND _IMPORT_CHECK_FILES_FOR_openvkl::openvkl "${_IMPORT_PREFIX}/lib/libopenvkl.so.0.9.0" )

# Import target "openvkl::openvkl_module_ispc_driver_4" for configuration "Release"
set_property(TARGET openvkl::openvkl_module_ispc_driver_4 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(openvkl::openvkl_module_ispc_driver_4 PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "embree"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libopenvkl_module_ispc_driver_4.so.0.9.0"
  IMPORTED_SONAME_RELEASE "libopenvkl_module_ispc_driver_4.so.0"
  )

list(APPEND _IMPORT_CHECK_TARGETS openvkl::openvkl_module_ispc_driver_4 )
list(APPEND _IMPORT_CHECK_FILES_FOR_openvkl::openvkl_module_ispc_driver_4 "${_IMPORT_PREFIX}/lib/libopenvkl_module_ispc_driver_4.so.0.9.0" )

# Import target "openvkl::openvkl_module_ispc_driver_8" for configuration "Release"
set_property(TARGET openvkl::openvkl_module_ispc_driver_8 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(openvkl::openvkl_module_ispc_driver_8 PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "embree"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libopenvkl_module_ispc_driver_8.so.0.9.0"
  IMPORTED_SONAME_RELEASE "libopenvkl_module_ispc_driver_8.so.0"
  )

list(APPEND _IMPORT_CHECK_TARGETS openvkl::openvkl_module_ispc_driver_8 )
list(APPEND _IMPORT_CHECK_FILES_FOR_openvkl::openvkl_module_ispc_driver_8 "${_IMPORT_PREFIX}/lib/libopenvkl_module_ispc_driver_8.so.0.9.0" )

# Import target "openvkl::openvkl_module_ispc_driver_16" for configuration "Release"
set_property(TARGET openvkl::openvkl_module_ispc_driver_16 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(openvkl::openvkl_module_ispc_driver_16 PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "embree"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libopenvkl_module_ispc_driver_16.so.0.9.0"
  IMPORTED_SONAME_RELEASE "libopenvkl_module_ispc_driver_16.so.0"
  )

list(APPEND _IMPORT_CHECK_TARGETS openvkl::openvkl_module_ispc_driver_16 )
list(APPEND _IMPORT_CHECK_FILES_FOR_openvkl::openvkl_module_ispc_driver_16 "${_IMPORT_PREFIX}/lib/libopenvkl_module_ispc_driver_16.so.0.9.0" )

# Import target "openvkl::openvkl_module_ispc_driver" for configuration "Release"
set_property(TARGET openvkl::openvkl_module_ispc_driver APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(openvkl::openvkl_module_ispc_driver PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "openvkl::openvkl_module_ispc_driver_4;openvkl::openvkl_module_ispc_driver_8;openvkl::openvkl_module_ispc_driver_16"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libopenvkl_module_ispc_driver.so.0.9.0"
  IMPORTED_SONAME_RELEASE "libopenvkl_module_ispc_driver.so.0"
  )

list(APPEND _IMPORT_CHECK_TARGETS openvkl::openvkl_module_ispc_driver )
list(APPEND _IMPORT_CHECK_FILES_FOR_openvkl::openvkl_module_ispc_driver "${_IMPORT_PREFIX}/lib/libopenvkl_module_ispc_driver.so.0.9.0" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
