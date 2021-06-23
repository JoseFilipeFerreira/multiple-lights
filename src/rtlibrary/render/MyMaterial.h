// Copyright 2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// ospray
#include "ospray/SDK/render/Material.h"
#include "ospray/SDK/render/pathtracer/materials/OBJ.h"
//#include "ospray/SDK/texture/Texture2D.h"

namespace ospray {
namespace rtlibrary {

struct OSPRAY_SDK_INTERFACE MyMaterial : public ospray::pathtracer::OBJMaterial
{
  MyMaterial();
  void commit() override;

 private:
  vec3f kd;
  vec3f ks;
  float ns;
  float d;
  //Ref<Texture2D> map_Kd;
};

} // namespace rtlibrary
} // namespace ospray
