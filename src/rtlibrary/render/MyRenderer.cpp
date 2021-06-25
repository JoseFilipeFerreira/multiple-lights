// Copyright 2009-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "MyRenderer.h"
// ispc exports
#include "MyRenderer_ispc.h"

#include "ospray/SDK/lights/Light.h"
#include "ospray/SDK/lights/PointLight.h"
#include "ospray/SDK/lights/QuadLight.h"


namespace ospray {
namespace rtlibrary {

MyRenderer::MyRenderer()
{
  ispcEquivalent = ispc::MyRenderer_create(this);
};

std::string MyRenderer::toString() const
{
  return "ospray::MyRenderer";
}

void MyRenderer::endFrame (FrameBuffer *fb, void *perFrameData) {};

// sets the lights for the cornell box (VI2)
void defineLightsConference (MyRenderer *self){
	int l;
	// get the light sources to make them available for ispc context
	self->lightArray.clear();

	for (l=0 ; l<4; l++) {

	  	QuadLight *qLight=  new QuadLight(); 
		qLight->setParam("color", vec3f(1.f,1.f,1.f));
  	  	qLight->setParam("intensity", 50.f);
		switch (l) {
			case 0:
  				qLight->setParam("position", vec3f(120., 650., -500.));
				break;
			case 1:
  				qLight->setParam("position", vec3f(1100., 650., -500.));
				break;
			case 2:
  				qLight->setParam("position", vec3f(120., 650., 300.));
				break;
			case 3:
  				qLight->setParam("position", vec3f(1100., 650., 300.));
				break;

		}
  		qLight->setParam("edge1", vec3f(40., 0., 0.));
  		qLight->setParam("edge2", vec3f(0., 0., 40.));
  		qLight->commit();
  		Light *light= (Light *) qLight; 
		self->lightArray.push_back(light->getIE());
	}
}

#define ONE_LIGHT 0
#define FOUR_LIGHTS 1
#define MANY_LIGHTS 2

// sets the lights for the cornell box (VI2)
void defineLightsCornell (MyRenderer *self){
	
	// get the light sources to make them available for ispc context
	self->lightArray.clear();

// #ifdef ONE_LIGHT
// 	const int chosen1 = MANY_LIGHTS;
// #endif
	const int chosen = MANY_LIGHTS;

	if(chosen == FOUR_LIGHTS)
	{
		for (int l = 0 ; l < 4 ; ++l) {

		  	QuadLight *qLight=  new QuadLight(); 
  		  	qLight->setParam("color", vec3f(1.f,1.f,1.f));
  		  	qLight->setParam("intensity", 10.f);
			switch (l) {
				case 0:
  					qLight->setParam("position", vec3f(120., 548., 120.));
					break;
				case 1:
  					qLight->setParam("position", vec3f(400., 548., 120.));
					break;
				case 2:
  					qLight->setParam("position", vec3f(120., 548., 400.));
					break;
				case 3:
  					qLight->setParam("position", vec3f(400., 548., 400.));
					break;

			}
  			qLight->setParam("edge1", vec3f(40., 0., 0.));
  			qLight->setParam("edge2", vec3f(0., 0., 40.));
  			qLight->commit();
  			Light *light= (Light *) qLight; 
			self->lightArray.push_back(light->getIE());
		}
	}
	else if(chosen == ONE_LIGHT)
	{
		QuadLight *qLight=  new QuadLight(); 
		qLight->setParam("color", vec3f(1.f,1.f,1.f));
		qLight->setParam("intensity", 10.f);
		qLight->setParam("position", vec3f(110., 548., 110.));
		qLight->setParam("edge1", vec3f(20., 0., 0.));
		qLight->setParam("edge2", vec3f(0., 0., 20.));
		qLight->commit();
		Light *light= (Light *) qLight; 
		self->lightArray.push_back(light->getIE());
	}
	else if(chosen == MANY_LIGHTS)
	{
		const int WIDTH = 4;
		const float LIGHT_SIZE = 10.;
		
		for (int i = 0 ; i < WIDTH ; ++i)
		{
			for (int j = 0 ; j < WIDTH ; ++j)
			{
				const float stride = (540. - 110.) / WIDTH;
				QuadLight *qLight=  new QuadLight(); 
				qLight->setParam("color", vec3f(1.f,1.f,1.f));
				qLight->setParam("intensity", 10.f);
				qLight->setParam("position", vec3f(110. + i*stride, 548., 110. + j*stride)); 
				qLight->setParam("edge1", vec3f(LIGHT_SIZE, 0., 0.));
				qLight->setParam("edge2", vec3f(0., 0., LIGHT_SIZE));
				qLight->commit();
				Light *light= (Light *) qLight; 
				self->lightArray.push_back(light->getIE());
			}
		}
	}
}

//!  \brief  light source creation
void MyRenderer::commit()
{
	vec3f ambColor = vec3f(0.2f);

	Renderer::commit();

	// define the lights according to the scene
	defineLightsCornell(this);
	//defineLightsConference(this);

	// now set lightPtr as the pointer to the lights buffer that will be set via ispc::VI2Renderer_set
	void **lightPtr = lightArray.empty() ? nullptr : &lightArray[0];

	ispc::MyRenderer_set(getIE(), lightPtr, lightArray.size(), (ispc::vec3f&)ambColor);
}


} // namespace rtlibrary
} // namespace ospray
