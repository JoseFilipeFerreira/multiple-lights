// Copyright 2009-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "MyMaterial.h"
// ispc
#include "MyMaterial_ispc.h"

namespace ospray {
namespace rtlibrary {

MyMaterial::MyMaterial()
{
  ispcEquivalent = ispc::MyMaterial_create(this);
}

void MyMaterial::commit()
{
  // ospray::pathtracer::OBJMaterial::commit();

  kd = getParam<vec3f>("kd", vec3f(.8f));
  d = getParam<float>("d", 1.f);
  ks = getParam<vec3f>("ks", vec3f(0.0f));
  ns = getParam<float>("ns", 1.f);
  ispc::MyMaterial_set(
         getIE(), (const ispc::vec3f &)kd, (const ispc::vec3f &)ks, d, ns);
  //map_Kd = (Texture2D *)getParamObject("map_kd");
  // ispc::MyMaterial_set(
  //     getIE(), (const ispc::vec3f &)Kd, d, map_Kd ? map_Kd->getIE() : nullptr);
}

} // namespace rtlibrary
} // namespace ospray
