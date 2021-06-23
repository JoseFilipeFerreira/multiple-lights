// Copyright 2009-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

//#include "MyRenderer.ih"
#include "ospray/SDK/render/Material.h"
#include "ospray/SDK/render/Renderer.h"

namespace ospray {
namespace rtlibrary {

struct MyRenderer : public Renderer
{
    MyRenderer();
  //virtual ~MyRenderer() override = default;
  virtual std::string toString() const override;
  void endFrame (FrameBuffer *fb, void *perFrameData) override;
  virtual void commit() override;

  // the ispc equivalent of my light sources
  std::vector<void*> lightArray;  

};

} // namespace rtlibrary
} // namespace ospray
