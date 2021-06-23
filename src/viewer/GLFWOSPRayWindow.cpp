// Copyright 2018-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "GLFWOSPRayWindow.h"
#include <ospcommon/math/vec.h>
#include "imgui_impl_glfw_gl3.h"

#include "ospcommon/utility/multidim_index_sequence.h"
// ospray_testing
// #include "ospray_testing.h"
// imgui
#include "imgui.h"
// std
#include <iostream>
#include <stdexcept>

// stl
#include <random>

// on Windows often only GL 1.1 headers are present
#ifndef GL_CLAMP_TO_BORDER
#define GL_CLAMP_TO_BORDER 0x812D
#endif
#ifndef GL_FRAMEBUFFER_SRGB
#define GL_FRAMEBUFFER_SRGB 0x8DB9
#endif
#ifndef GL_RGBA32F
#define GL_RGBA32F 0x8814
#endif
#ifndef GL_RGB32F
#define GL_RGB32F 0x8815
#endif

static bool g_quitNextFrame = false;

static const std::vector<std::string> g_renderers = {"pathtracer", "myrenderer", "debug"};

static const std::vector<std::string> g_debugRendererTypes =
    {"eyeLight", "primID", "geomID", "instID", "Ng", "Ns", "backfacing_Ng", "backfacing_Ns", "dPds", "dPdt", "volume"};

bool rendererUI_callback(void*, int index, const char** out_text) {
    *out_text = g_renderers[index].c_str();
    return true;
}

bool debugTypeUI_callback(void*, int index, const char** out_text) {
    *out_text = g_debugRendererTypes[index].c_str();
    return true;
}

// GLFWOSPRayWindow definitions ///////////////////////////////////////////////

void error_callback(int error, const char* desc) {
    std::cerr << "error " << error << ": " << desc << std::endl;
}

GLFWOSPRayWindow* GLFWOSPRayWindow::activeWindow = nullptr;

GLFWOSPRayWindow::GLFWOSPRayWindow(const vec2i& windowSize, cpp::Group MeshGroup, bool modelAvailable)
    : MeshGroup(MeshGroup), modelAvailable(modelAvailable) {
    if (activeWindow != nullptr) {
        throw std::runtime_error("Cannot create more than one GLFWOSPRayWindow!");
    }

    activeWindow = this;

    glfwSetErrorCallback(error_callback);

    // initialize GLFW
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW!");
    }

    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
    // create GLFW window
    glfwWindow = glfwCreateWindow(windowSize.x, windowSize.y, "Computer Graphics 2021", nullptr, nullptr);

    if (!glfwWindow) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window!");
    }

    // make the window's context current
    glfwMakeContextCurrent(glfwWindow);

    //
//    glfwSetWindowSize(glfwWindow,windowSize.x,windowSize.y);
//    glViewport(0, 0, windowSize.x, windowSize.y);

    ImGui_ImplGlfwGL3_Init(glfwWindow, true);

    // set initial OpenGL state
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    // create OpenGL frame buffer texture
    glGenTextures(1, &framebufferTexture);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // set GLFW callbacks
    glfwSetFramebufferSizeCallback(glfwWindow, [](GLFWwindow*, int newWidth, int newHeight) {
        activeWindow->reshape(vec2i{newWidth, newHeight});
    });

    glfwSetCursorPosCallback(glfwWindow, [](GLFWwindow*, double x, double y) {
        ImGuiIO& io = ImGui::GetIO();
        if (!activeWindow->showUi || !io.WantCaptureMouse) {
            activeWindow->motion(vec2f{float(x), float(y)});
        }
    });

    glfwSetKeyCallback(glfwWindow, [](GLFWwindow*, int key, int, int action, int) {
        if (action == GLFW_PRESS) {
            switch (key) {
                case GLFW_KEY_G:
                    activeWindow->showUi = !(activeWindow->showUi);
                    break;
                case GLFW_KEY_Q:
                    g_quitNextFrame = true;
                    break;
            }
        }
    });

    glfwSetMouseButtonCallback(glfwWindow, [](GLFWwindow*, int button, int action, int /*mods*/) {
        auto& w = *activeWindow;
        if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
            auto mouse      = activeWindow->previousMouse;
            auto windowSize = activeWindow->windowSize;
            const vec2f pos(mouse.x / static_cast<float>(windowSize.x),
                            1.f - mouse.y / static_cast<float>(windowSize.y));

            auto res = w.framebuffer.pick(w.renderer, w.camera, w.world, pos);

            if (res.hasHit) {
                std::cout << "Picked geometry [inst: " << res.instance << ", model: " << res.model
                          << ", prim: " << res.primID << "]" << std::endl;
            }
        }
    });

    // OSPRay setup //

    refreshScene(true);

    // trigger window reshape events with current window size
    glfwGetFramebufferSize(glfwWindow, &this->windowSize.x, &this->windowSize.y);
    reshape(this->windowSize);

    commitOutstandingHandles();
}

GLFWOSPRayWindow::~GLFWOSPRayWindow() {
    ImGui_ImplGlfwGL3_Shutdown();
    // cleanly terminate GLFW
    glfwTerminate();
}

GLFWOSPRayWindow* GLFWOSPRayWindow::getActiveWindow() {
    return activeWindow;
}

void GLFWOSPRayWindow::mainLoop() {
    // continue until the user closes the window
    startNewOSPRayFrame();

    while (!glfwWindowShouldClose(glfwWindow) && !g_quitNextFrame) {
        ImGui_ImplGlfwGL3_NewFrame();

        display();

        // poll and process events
        glfwPollEvents();
    }

    waitOnOSPRayFrame();
}

void GLFWOSPRayWindow::reshape(const vec2i& newWindowSize) {
    windowSize = newWindowSize;

    std::cout << windowSize << std::endl;

    // create new frame buffer
    auto buffers = OSP_FB_COLOR | OSP_FB_DEPTH | OSP_FB_ACCUM | OSP_FB_ALBEDO | OSP_FB_NORMAL;
    framebuffer  = cpp::FrameBuffer(windowSize, OSP_FB_RGBA32F, buffers);

    refreshFrameOperations();

    // reset OpenGL viewport and orthographic projection
    glViewport(0, 0, windowSize.x, windowSize.y);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, windowSize.x, 0.0, windowSize.y, -1.0, 1.0);

    // update camera
    arcballCamera->updateWindowSize(windowSize);

    camera.setParam("aspect", windowSize.x / float(windowSize.y));
    camera.commit();
}

void GLFWOSPRayWindow::updateCamera() {
    camera.setParam("aspect", windowSize.x / float(windowSize.y));
    camera.setParam("position", arcballCamera->eyePos());
    camera.setParam("direction", arcballCamera->lookDir());
    camera.setParam("up", arcballCamera->upDir());
}

void GLFWOSPRayWindow::motion(const vec2f& position) {
    const vec2f mouse(position.x, position.y);
    if (previousMouse != vec2f(-1)) {
        const bool leftDown   = glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        const bool rightDown  = glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
        const bool middleDown = glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
        const vec2f prev      = previousMouse;

        bool cameraChanged = leftDown || rightDown || middleDown;

        if (leftDown) {
            const vec2f mouseFrom(clamp(prev.x * 2.f / windowSize.x - 1.f, -1.f, 1.f),
                                  clamp(prev.y * 2.f / windowSize.y - 1.f, -1.f, 1.f));
            const vec2f mouseTo(clamp(mouse.x * 2.f / windowSize.x - 1.f, -1.f, 1.f),
                                clamp(mouse.y * 2.f / windowSize.y - 1.f, -1.f, 1.f));
            arcballCamera->rotate(mouseFrom, mouseTo);
        } else if (rightDown) {
            arcballCamera->zoom(mouse.y - prev.y);
        } else if (middleDown) {
            arcballCamera->pan(vec2f(mouse.x - prev.x, prev.y - mouse.y));
        }

        if (cameraChanged) {
            if (cancelFrameOnInteraction) {
                currentFrame.cancel();
                waitOnOSPRayFrame();
            }
            updateCamera();
            addObjectToCommit(camera.handle());
        }
    }

    previousMouse = mouse;
}

void GLFWOSPRayWindow::display() {
    if (showUi) {
        buildUI();
    }

    if (displayCallback)
        displayCallback(this);

    updateTitleBar();

    // Turn on SRGB conversion for OSPRay frame
    glEnable(GL_FRAMEBUFFER_SRGB);

    static bool firstFrame = true;
    if (firstFrame || currentFrame.isReady()) {
        waitOnOSPRayFrame();

        latestFPS = 1.f / currentFrame.duration();

        auto* fb = framebuffer.map(showAlbedo ? OSP_FB_ALBEDO : OSP_FB_COLOR);

        glBindTexture(GL_TEXTURE_2D, framebufferTexture);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     showAlbedo ? GL_RGB32F : GL_RGBA32F,
                     windowSize.x,
                     windowSize.y,
                     0,
                     showAlbedo ? GL_RGB : GL_RGBA,
                     GL_FLOAT,
                     fb);

        framebuffer.unmap(fb);

        commitOutstandingHandles();

        startNewOSPRayFrame();
        firstFrame = false;
    }

    // clear current OpenGL color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // render textured quad with OSPRay frame buffer contents
    glBegin(GL_QUADS);

    glTexCoord2f(0.f, 0.f);
    glVertex2f(0.f, 0.f);

    glTexCoord2f(0.f, 1.f);
    glVertex2f(0.f, windowSize.y);

    glTexCoord2f(1.f, 1.f);
    glVertex2f(windowSize.x, windowSize.y);

    glTexCoord2f(1.f, 0.f);
    glVertex2f(windowSize.x, 0.f);

    glEnd();

    // Disable SRGB conversion for UI
    glDisable(GL_FRAMEBUFFER_SRGB);

    ImGui::Render();

    // swap buffers
    glfwSwapBuffers(glfwWindow);
}

void GLFWOSPRayWindow::startNewOSPRayFrame() {
    if (updateFrameOpsNextFrame) {
        refreshFrameOperations();
        updateFrameOpsNextFrame = false;
    }
    currentFrame = framebuffer.renderFrame(renderer, camera, world);
}

void GLFWOSPRayWindow::waitOnOSPRayFrame() {
    currentFrame.wait();
}

void GLFWOSPRayWindow::addObjectToCommit(OSPObject obj) {
    objectsToCommit.push_back(obj);
}

void GLFWOSPRayWindow::updateTitleBar() {
    std::stringstream windowTitle;
    windowTitle << "OSPRay: " << std::setprecision(3) << latestFPS << " fps";
    if (latestFPS < 2.f) {
        float progress = currentFrame.progress();
        windowTitle << " | ";
        int barWidth = 20;
        std::string progBar;
        progBar.resize(barWidth + 2);
        auto start = progBar.begin() + 1;
        auto end   = start + progress * barWidth;
        std::fill(start, end, '=');
        std::fill(end, progBar.end(), '_');
        *end            = '>';
        progBar.front() = '[';
        progBar.back()  = ']';
        windowTitle << progBar;
    }

    glfwSetWindowTitle(glfwWindow, windowTitle.str().c_str());
}

void GLFWOSPRayWindow::buildUI() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::Begin("press 'g' to hide/show UI", nullptr, flags);

    if (ImGui::Checkbox("Add ground plane", &addPlane)) {
        refreshScene(false);
    }

    static int whichRenderer     = 0;
    static int whichDebuggerType = 0;
    if (ImGui::Combo("renderer##whichRenderer", &whichRenderer, rendererUI_callback, nullptr, g_renderers.size())) {
        rendererTypeStr = g_renderers[whichRenderer];

        if (rendererType == OSPRayRendererType::DEBUGGER)
            whichDebuggerType = 0;  // reset UI if switching away from debug renderer
        if (rendererTypeStr == "pathtracer")
            rendererType = OSPRayRendererType::PATHTRACER;
        if (rendererTypeStr == "myrenderer")
            rendererType = OSPRayRendererType::MYRENDERER;
        else if (rendererTypeStr == "debug")
            rendererType = OSPRayRendererType::DEBUGGER;
        else
            rendererType = OSPRayRendererType::OTHER;

        refreshScene();
    }

    if (rendererType == OSPRayRendererType::DEBUGGER) {
        if (ImGui::Combo("debug type##whichDebugType",
                         &whichDebuggerType,
                         debugTypeUI_callback,
                         nullptr,
                         g_debugRendererTypes.size())) {
            renderer.setParam("method", g_debugRendererTypes[whichDebuggerType]);
            addObjectToCommit(renderer.handle());
        }
    }

    if (ImGui::Checkbox("Add ambient light", &ambientLight)) {
        refreshScene(false);
    }

    ImGui::Separator();

    ImGui::Checkbox("cancel frame on interaction", &cancelFrameOnInteraction);
    ImGui::Checkbox("show albedo", &showAlbedo);

    ImGui::Separator();

    static int spp = 1;
    if (ImGui::SliderInt("pixelSamples", &spp, 1, 64)) {
        renderer.setParam("pixelSamples", spp);
        addObjectToCommit(renderer.handle());
    }

    if (ImGui::ColorEdit3("backgroundColor", bgColor)) {
        renderer.setParam("backgroundColor", bgColor);
        addObjectToCommit(renderer.handle());
    }

    static bool useTestTex = false;
    if (ImGui::Checkbox("backplate texture", &useTestTex)) {
        if (useTestTex) {
            renderer.setParam("map_backplate", backplateTex);
        } else {
            renderer.removeParam("map_backplate");
        }
        addObjectToCommit(renderer.handle());
    }

    if (rendererType != OSPRayRendererType::DEBUGGER) {
        if (ImGui::Checkbox("renderSunSky", &renderSunSky)) {
            if (renderSunSky) {
                sunSky.setParam("direction", sunDirection);
                world.setParam("light", cpp::Data(sunSky));
                addObjectToCommit(sunSky.handle());
            } else {
                cpp::Light light("ambient");
                light.setParam("visible", ambientLight);
                light.commit();
                world.setParam("light", cpp::Data(light));
            }
            addObjectToCommit(world.handle());
        }
        if (renderSunSky) {
            if (ImGui::DragFloat3("sunDirection", sunDirection, 0.01f, -1.f, 1.f)) {
                sunSky.setParam("direction", sunDirection);
                addObjectToCommit(sunSky.handle());
            }
            if (ImGui::DragFloat("turbidity", &turbidity, 0.1f, 1.f, 10.f)) {
                sunSky.setParam("turbidity", turbidity);
                addObjectToCommit(sunSky.handle());
            }
        }
        static int maxDepth = 20;
        if (ImGui::SliderInt("maxPathLength", &maxDepth, 1, 64)) {
            renderer.setParam("maxPathLength", maxDepth);
            addObjectToCommit(renderer.handle());
        }

        static int rouletteDepth = 1;
        if (ImGui::SliderInt("roulettePathLength", &rouletteDepth, 1, 64)) {
            renderer.setParam("roulettePathLength", rouletteDepth);
            addObjectToCommit(renderer.handle());
        }

        static float minContribution = 0.001f;
        if (ImGui::SliderFloat("minContribution", &minContribution, 0.f, 1.f)) {
            renderer.setParam("minContribution", minContribution);
            addObjectToCommit(renderer.handle());
        }
    }

    if (uiCallback) {
        ImGui::Separator();
        uiCallback();
    }

    ImGui::End();
}

void GLFWOSPRayWindow::commitOutstandingHandles() {
    auto handles = objectsToCommit.consume();
    if (!handles.empty()) {
        for (auto& h : handles)
            ospCommit(h);
        framebuffer.resetAccumulation();
    }
}

void GLFWOSPRayWindow::refreshScene(bool resetCamera) {
    if (currentFrame)
        waitOnOSPRayFrame();

    if (!modelAvailable) {
        world = cpp::World();

        cpp::Geometry boxGeometry("box");

        vec3i dimensions(4);

        ospcommon::index_sequence_3D numBoxes(dimensions);

        std::vector<box3f> boxes;
        std::vector<vec4f> color;

        auto dim = reduce_max(dimensions);

        for (auto i : numBoxes) {
            // auto i_f = static_cast<vec3f>(i);

            auto lower = (i * 5.f) - vec3f(5.f * dimensions.x / 2, 0.f, 5.f * dimensions.z / 2);
            auto upper = lower + (0.75f * 5.f);
            boxes.emplace_back(lower, upper);

            auto box_color = (0.8f * i / dim) + 0.2f;
            color.emplace_back(box_color.x, box_color.y, box_color.z, 1.f);
        }

        boxGeometry.setParam("box", cpp::Data(boxes));
        boxGeometry.commit();

        cpp::GeometricModel model(boxGeometry);

        model.setParam("color", cpp::Data(color));

        if (rendererTypeStr == "pathtracer" || rendererTypeStr == "myrenderer") {
            // cpp::Material material("pathtracer", "obj");
            // material.commit();
            // model.setParam("material", material);
        }

        model.commit();

        MeshGroup.setParam("geometry", cpp::Data(model));
        MeshGroup.commit();
    }
    // put the group into an instance (give the group a world transform)
    ospray::cpp::Instance instance(MeshGroup);
    instance.commit();

    std::vector<cpp::Instance> inst;
    inst.push_back(instance);

    if (addPlane && !modelAvailable) {
        auto bounds = MeshGroup.getBounds();

        auto planeExtent = 1.0f * length(bounds.center() - bounds.lower);

        cpp::Geometry planeGeometry("mesh");

        std::vector<vec3f> v_position;
        std::vector<vec3f> v_normal;
        std::vector<vec4f> v_color;
        std::vector<vec4ui> indices;

        unsigned int startingIndex = 0;

        const vec3f up   = vec3f{0.f, 1.f, 0.f};
        const vec4f gray = vec4f{0.9f, 0.9f, 0.9f, 0.75f};

        v_position.emplace_back(-planeExtent, -1.f, -planeExtent);
        v_position.emplace_back(planeExtent, -1.f, -planeExtent);
        v_position.emplace_back(planeExtent, -1.f, planeExtent);
        v_position.emplace_back(-planeExtent, -1.f, planeExtent);

        v_normal.push_back(up);
        v_normal.push_back(up);
        v_normal.push_back(up);
        v_normal.push_back(up);

        v_color.push_back(gray);
        v_color.push_back(gray);
        v_color.push_back(gray);
        v_color.push_back(gray);

        indices.emplace_back(startingIndex, startingIndex + 1, startingIndex + 2, startingIndex + 3);

        // stripes on ground plane
        const float stripeWidth  = 0.025f;
        const float paddedExtent = planeExtent + stripeWidth;
        const size_t numStripes  = 10;

        const vec4f stripeColor = vec4f{1.0f, 0.1f, 0.1f, 1.f};

        for (size_t i = 0; i < numStripes; i++) {
            // the center coordinate of the stripe, either in the x or z
            // direction
            const float coord = -planeExtent + float(i) / float(numStripes - 1) * 2.f * planeExtent;

            // offset the stripes by an epsilon above the ground plane
            const float yLevel = -1.f + 1e-3f;

            // x-direction stripes
            startingIndex = v_position.size();

            v_position.emplace_back(-paddedExtent, yLevel, coord - stripeWidth);
            v_position.emplace_back(paddedExtent, yLevel, coord - stripeWidth);
            v_position.emplace_back(paddedExtent, yLevel, coord + stripeWidth);
            v_position.emplace_back(-paddedExtent, yLevel, coord + stripeWidth);

            v_normal.push_back(up);
            v_normal.push_back(up);
            v_normal.push_back(up);
            v_normal.push_back(up);

            v_color.push_back(stripeColor);
            v_color.push_back(stripeColor);
            v_color.push_back(stripeColor);
            v_color.push_back(stripeColor);

            indices.emplace_back(startingIndex, startingIndex + 1, startingIndex + 2, startingIndex + 3);

            // z-direction stripes
            startingIndex = v_position.size();

            v_position.emplace_back(coord - stripeWidth, yLevel, -paddedExtent);
            v_position.emplace_back(coord + stripeWidth, yLevel, -paddedExtent);
            v_position.emplace_back(coord + stripeWidth, yLevel, paddedExtent);
            v_position.emplace_back(coord - stripeWidth, yLevel, paddedExtent);

            v_normal.push_back(up);
            v_normal.push_back(up);
            v_normal.push_back(up);
            v_normal.push_back(up);

            v_color.push_back(stripeColor);
            v_color.push_back(stripeColor);
            v_color.push_back(stripeColor);
            v_color.push_back(stripeColor);

            indices.emplace_back(startingIndex, startingIndex + 1, startingIndex + 2, startingIndex + 3);
        }

        planeGeometry.setParam("vertex.position", cpp::Data(v_position));
        planeGeometry.setParam("vertex.normal", cpp::Data(v_normal));
        planeGeometry.setParam("vertex.color", cpp::Data(v_color));
        planeGeometry.setParam("index", cpp::Data(indices));

        planeGeometry.commit();

        cpp::GeometricModel plane(planeGeometry);

        if (rendererTypeStr == "pathtracer" || rendererTypeStr == "myrenderer") {
            // cpp::Material material("pathtracer", "obj");
            // material.commit();
            // plane.setParam("material", material);
        }

        plane.commit();

        cpp::Group planeGroup;
        planeGroup.setParam("geometry", cpp::Data(plane));
        planeGroup.commit();

        cpp::Instance planeInst(planeGroup);
        planeInst.commit();

        inst.push_back(planeInst);
    }

    world.setParam("instance", cpp::Data(inst));

    // put the instance in the world
    // ospray::cpp::World world;
    // world.setParam("instance", ospray::cpp::Data(instance));

    // create and setup light for Ambient Occlusion
    ospray::cpp::Light light("ambient");
    light.setParam("visible", ambientLight);
    light.commit();

    world.setParam("light", ospray::cpp::Data(light));

    world.commit();

    resetCamera = true;

    renderer = cpp::Renderer(rendererTypeStr);

    // complete setup of renderer
    // renderer.setParam("aoSamples", 1);
    // renderer.setParam("backgroundColor", 1.0f); // white, transparent

    // retains a set background color on renderer change
    renderer.setParam("backgroundColor", bgColor);
    renderer.commit();
    addObjectToCommit(renderer.handle());

    // set up backplate texture
    std::vector<vec4f> backplate;
    backplate.push_back(vec4f(0.8f, 0.2f, 0.2f, 1.0f));
    backplate.push_back(vec4f(0.2f, 0.8f, 0.2f, 1.0f));
    backplate.push_back(vec4f(0.2f, 0.2f, 0.8f, 1.0f));
    backplate.push_back(vec4f(0.4f, 0.2f, 0.4f, 1.0f));

    OSPTextureFormat texFmt = OSP_TEXTURE_RGBA32F;
    cpp::Data texData(vec2ul(2, 2), backplate.data());
    backplateTex.setParam("data", texData);
    backplateTex.setParam("format", OSP_INT, &texFmt);
    addObjectToCommit(backplateTex.handle());

    if (resetCamera) {
        // create the arcball camera model
        // arcballCamera.reset(new ArcballCamera(world.getBounds(), windowSize));

        if (!cameraSet) {
            auto d = length(world.getBounds().size());
            arcballCamera.reset(
                new ArcballCamera(world.getBounds().center() + vec3f(0, 0, d), world.getBounds().center(), windowSize));
        } else {
            arcballCamera.reset(new ArcballCamera(cameraPos, cameraLookAt, windowSize));
        }

        // init camera
        updateCamera();
        camera.commit();
    }
}

void GLFWOSPRayWindow::refreshFrameOperations() {
    framebuffer.commit();
}
