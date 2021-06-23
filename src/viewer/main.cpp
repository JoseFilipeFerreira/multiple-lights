#include "GLFWOSPRayWindow.h"
#include "scene/sceneloader.h"

using namespace ospray;
using ospcommon::make_unique;

#include "ospcommon/utility/multidim_index_sequence.h"

#include <iostream>
#include <stdexcept>
#include <filesystem>

#include <json/json.h>

inline void initializeOSPRay(int argc, const char** argv, bool errorsFatal = true) {
    // initialize OSPRay; OSPRay parses (and removes) its commandline parameters,
    // e.g. "--osp:debug"

    ospLoadModule("rtlibrary");

    OSPError initError = ospInit(&argc, argv);

    if (initError != OSP_NO_ERROR)
        throw std::runtime_error("OSPRay not initialized correctly!");

    OSPDevice device = ospGetCurrentDevice();
    if (!device)
        throw std::runtime_error("OSPRay device could not be fetched!");

    // set an error callback to catch any OSPRay errors and exit the application
    if (errorsFatal) {
        ospDeviceSetErrorFunc(device, [](OSPError error, const char* errorDetails) {
            std::cerr << "OSPRay error: " << errorDetails << std::endl;
            exit(error);
        });
    } else {
        ospDeviceSetErrorFunc(device, [](OSPError, const char* errorDetails) {
            std::cerr << "OSPRay error: " << errorDetails << std::endl;
        });
    }

    ospDeviceSetStatusFunc(device, [](const char* msg) { std::cout << msg; });

    bool warnAsErrors = true;
    auto logLevel     = OSP_LOG_WARNING;

    ospDeviceSetParam(device, "warnAsError", OSP_BOOL, &warnAsErrors);
    ospDeviceSetParam(device, "logLevel", OSP_INT, &logLevel);

    ospDeviceCommit(device);
    ospDeviceRelease(device);
}

int main(int argc, char* argv[]) {
    initializeOSPRay(argc, (const char**)argv);
    std::string modelname;
    std::unique_ptr<GLFWOSPRayWindow> glfwOSPRayWindow = nullptr;

    bool hasEye{false}, hasLook{false};
    ospcommon::math::vec3f eye, look;
    ospcommon::math::vec2i windowSize(1024,768);

    for (int arg = 0; arg < argc; arg++) {
        if (std::string(argv[arg]) == "-m") {
            std::cout << "Load Model" << std::endl;
            modelname = std::string(argv[arg + 1]);
            arg++;
            continue;
        }
        if (std::string(argv[arg]) == "-v") {
            char* token = std::strtok(argv[arg + 1], ",");
            int i       = 0;
            while (token != NULL) {
                eye[i++] = atof(token);
                token    = std::strtok(NULL, ",");
            }

            if (i != 3)
                throw std::runtime_error("Error parsing view");

            std::cout << "View from : " << eye << std::endl;
            hasEye = true;
            arg++;
            continue;
        }
        if (std::string(argv[arg]) == "-l") {
            char* token = std::strtok(argv[arg + 1], ",");
            int i       = 0;

            while (token != NULL) {
                look[i++] = atof(token);
                token     = std::strtok(NULL, ",");
            }

            if (i != 3)
                throw std::runtime_error("Error parsing view");

            std::cout << "Look at : " << look << std::endl;
            hasLook = true;
            arg++;
            continue;
        }
        if (std::string(argv[arg]) == "-w") {
            char* token = std::strtok(argv[arg + 1], ",");
            int i       = 0;

            while (token != NULL) {
                //look[i++] = atof(token);
                windowSize[i++] = atof(token);
                token     = std::strtok(NULL, ",");
            }

            if (i != 2)
                throw std::runtime_error("Error parsing window size");

            std::cout << "Window size : " << windowSize << std::endl;
            //hasLook = true;
            arg++;
            continue;
        }
    }

    cpp::Group MeshGroup;
    bool ModelLoaded = false;

    if (modelname != "") {
        MeshGroup   = model::loadobj(modelname);
        ModelLoaded = true;
    }

    glfwOSPRayWindow = make_unique<GLFWOSPRayWindow>(windowSize, MeshGroup, ModelLoaded);


    if (hasEye && hasLook) {
        glfwOSPRayWindow->cameraSet    = true;
        glfwOSPRayWindow->cameraPos    = eye;
        glfwOSPRayWindow->cameraLookAt = look;
        glfwOSPRayWindow->refreshScene(true);
    }

    glfwOSPRayWindow->rendererTypeStr = "myrenderer";
    glfwOSPRayWindow->rendererType    = OSPRayRendererType::MYRENDERER;
//    glfwOSPRayWindow->buildUI();
    glfwOSPRayWindow->mainLoop();
    // glfwOSPRayWindow.reset();

    return 0;
}
