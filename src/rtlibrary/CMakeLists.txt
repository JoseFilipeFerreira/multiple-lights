# project(rtlibrary)

# add_library(ospray_module_rtlibrary SHARED ModuleInit.cpp)

# ispc_include_directories(${CMAKE_CURRENT_BINARY_DIR})

# ispc_target_add_sources(ospray_module_rtlibrary
  
# )

# target_link_libraries(ospray_module_rtlibrary PUBLIC     
#     ospray::ospray_sdk
#     ospray::ospray_module_ispc
# )


ispc_include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(ospray_module_rtlibrary SHARED)

ispc_target_add_sources(ospray_module_rtlibrary

  ModuleInit.cpp

  render/MyRenderer.h
  render/MyRenderer.cpp
  render/MyRenderer.ih
  render/MyRenderer.ispc
  render/MyMaterial.h
  render/MyMaterial.cpp
  render/MyMaterial.ih
  render/MyMaterial.ispc
  render/MyRenderer-T4.ih
  render/MyRenderer-T4.ispc
)

target_link_libraries(ospray_module_rtlibrary
  PRIVATE
    ospray::ospray_sdk
    ospray::ospray_module_ispc
)

target_include_directories(ospray_module_rtlibrary
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(ospray_module_rtlibrary
  PROPERTIES VERSION 1 SOVERSION 0)

install(TARGETS ospray_module_rtlibrary
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} NAMELINK_SKIP
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(TARGETS ospray_module_rtlibrary
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} NAMELINK_ONLY
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
    
