project(CG-Spring2012)

find_package(jsoncpp REQUIRED)

set(SRC_FILES 

    scene/sceneloader.cpp
    imgui_impl_glfw_gl3.cpp
    GLFWOSPRayWindow.cpp
    ArcballCamera.cpp
    main.cpp
)
set(HDR_FILES )

add_executable(CGViewer2021 ${SRC_FILES})

add_custom_command(
    TARGET CGViewer2021 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory              
        ${CMAKE_CURRENT_SOURCE_DIR}/models $<TARGET_FILE_DIR:CGViewer2021>/models
    COMMENT "Copying models" VERBATIM
    )
  
target_include_directories(CGViewer2021 PRIVATE ${OpenGL_INCLUDE_DIR})
target_compile_definitions(CGViewer2021 PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)

target_link_directories(CGViewer2021 PRIVATE ${OpenGL_LIBRARY_DIRS} <TARGET_FILE_DIR:lib>)
target_link_libraries(CGViewer2021 PUBLIC
ospcommon::ospcommon
ospray::ospray
jsoncpp_lib
ospray_module_rtlibrary
PRIVATE
OpenGL::GL imgui::imgui glfw 
)
