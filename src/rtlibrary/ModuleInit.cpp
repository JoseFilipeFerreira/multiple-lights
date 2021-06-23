
#include "render/MyRenderer.h"
#include "render/MyMaterial.h"
#include "ospray/ospray.h"
#include "ospray/version.h"

extern "C" OSPError OSPRAY_DLLEXPORT ospray_module_init_rtlibrary(int16_t versionMajor,
                                                                  int16_t versionMinor,
                                                                  int16_t /*versionPatch*/) {
    using namespace ospray;

    auto status = moduleVersionCheck(versionMajor, versionMinor);

    if (status == OSP_NO_ERROR) {
        // Run the ISPC module's initialization function as well to register local
        // types
        status = ospLoadModule("ispc");
    }

    if (status == OSP_NO_ERROR) {
        
        std::cout << "Add render class" << std::endl;

        Renderer::registerType<ospray::rtlibrary::MyRenderer>("myrenderer");
        Material::registerType<ospray::rtlibrary::MyMaterial>("pathtracer", "MyMaterial");
    }

    return status;
}
