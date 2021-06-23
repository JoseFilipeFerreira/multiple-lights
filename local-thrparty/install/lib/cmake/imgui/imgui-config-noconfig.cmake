#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "imgui::imgui" for configuration ""
set_property(TARGET imgui::imgui APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(imgui::imgui PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libimgui.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS imgui::imgui )
list(APPEND _IMPORT_CHECK_FILES_FOR_imgui::imgui "${_IMPORT_PREFIX}/lib/libimgui.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
