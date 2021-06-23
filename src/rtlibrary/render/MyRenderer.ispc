// Copyright 2009-2019 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "MyRenderer.ih"

#include "ospray/SDK/common/World.ih"

/*
 *  Test renderSample
 *  This returns a color (RGB) for each sample based on the sampleID
 */
void MyRenderer_renderSample_testFrame (Renderer *uniform _self,
                                        FrameBuffer *uniform fb,
                                        World *uniform model,
                                        void *uniform perFrameData,
                                        varying ScreenSample &sample)
{
  sample.rgb.x = ((sample.sampleID.x)%256)/255.f;
  sample.rgb.y = ((sample.sampleID.y)%256)/255.f;
  sample.rgb.z = ((sample.sampleID.x+sample.sampleID.y+sample.sampleID.z)%256)/255.f;
  sample.alpha = 1.f;
  sample.z = 1.f;
}

/*
 *  render depth
 *  Sets the sample (pixel) color (RGB) equal to the primary ray depth
 */
void MyRenderer_renderSample_depth (Renderer *uniform _self,
                                    FrameBuffer *uniform fb,
                                    World *uniform model,
                                    void *uniform perFrameData,
                                    varying ScreenSample &sample)
{
    uniform MyRenderer *uniform self = (uniform MyRenderer *uniform) _self;
    
    // trace the primary ray which is stored in sample
    traceRay(model, sample.ray);
    sample.z = sample.ray.t;    // t is the ray's length until the intersection
    sample.alpha = 1.f;
    
    //float depth_normalizing_constant = 1.e3;  // cornell_VI2
    //float depth_normalizing_constant = 5.f;   // cornell_OSP
    //float depth_normalizing_constant = 5.e3;    // conference
    float depth_normalizing_constant = 1.e4;    // sponza

    sample.rgb = make_vec3f(sample.z/depth_normalizing_constant);
}


// Exports (called from C++) //////////////////////////////////////////////////

export void *uniform MyRenderer_create(void *uniform cppE)
{
  uniform MyRenderer *uniform self = uniform new uniform MyRenderer;
  Renderer_Constructor(&self->super, cppE);
  
  int renderer_select = 2;  // use this to select which renderSample to use
  switch (renderer_select) {
    case 0:  // patterned pixels
        self->super.renderSample = MyRenderer_renderSample_testFrame;
        break;
    case 1:  // depth renderer
        self->super.renderSample = MyRenderer_renderSample_depth;
        break;
    case 2:  // tutorial 2,3,4 renderer
        self->super.renderSample = MyRenderer_renderSample_RT;
        break;
    default:
        self->super.renderSample = MyRenderer_renderSample_testFrame;
        break;
  }

  // exported function used ro set MyRenderer parameters
  // currently setting lights and ambColor
  uniform vec3f ambient = make_vec3f(0.f,0.f,0.f);
  MyRenderer_set (self, NULL, 0, ambient);

  return self;
}

export void MyRenderer_set (void *uniform _self,
                  void **uniform lights,
                  const uniform uint32 numLights,
                  uniform vec3f &ambColor) {
  MyRenderer *uniform self = (MyRenderer *uniform)_self;
  self->lights = (const uniform Light *uniform *uniform) lights;
  self->numLights = numLights;
  self->ambColor = ambColor;
}
