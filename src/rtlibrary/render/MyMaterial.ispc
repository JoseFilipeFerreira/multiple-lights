// Copyright 2009-2019 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "MyMaterial.ih"

export void *uniform MyMaterial_create(void *uniform cppE)
{
  MyMaterial *uniform mat = uniform new uniform MyMaterial;

  mat->super.cppEquivalent = cppE;

  return mat;
}

export void MyMaterial_set(void *uniform _mat,
    const uniform vec3f &kd,
    const uniform vec3f &ks,
    uniform float d,
    uniform float ns)
    //void *uniform _map_Kd)
{
  MyMaterial *uniform mat = (MyMaterial * uniform) _mat;

  mat->kd = kd;
  mat->ks = ks;
  //mat->map_Kd = (uniform Texture2D * uniform) _map_Kd;
  mat->d = d;
  mat->ns = ns;
}
