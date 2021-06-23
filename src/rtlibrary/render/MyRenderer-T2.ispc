
#include "MyRenderer-T2.ih"
#include "MyMaterial.ih"
#include "ospray/SDK/common/World.ih"


/******
*
*  Does classic deterministic direct illumination
*
************/

vec3f direct_illumination (uniform MyRenderer *uniform self,
            World *uniform model,
						const DifferentialGeometry &dg,
						const vec3f Kd,
						const float Raytime) {

  vec3f color = make_vec3f(0.f);
  
  //calculate shading for all lights
  for (uniform int i = 0; self->lights && i < self->numLights; i++) {
  	const uniform Light *uniform l = self->lights[i];

    const vec2f s = make_vec2f(0.0f); // sample center of area lights
    
    const Light_SampleRes light = l->sample(l, dg, s);

    if (reduce_max(light.weight) > 0.f) {   // any potential contribution?
      float cosNL = dot(light.dir, dg.Ns);  // cosine between lighting dir and normal
      if (cosNL < 0.0f) continue;  // light on the other side

      const vec3f diffuse = Kd * cosNL;
      const vec3f light_contrib = diffuse * light.weight;

      /*
       * Shoot shadow ray 
       */
      /*  Shooting shadow rays is commented out
      const float max_contrib = reduce_max(light_contrib);
      if (max_contrib > 0.f) {
        Ray shadowRay;
		    vec3f P = dg.P;
        if (dot(dg.Ng, light.dir) < 0.0f)
            P = P - (2.f * dg.epsilon) * dg.Ng;

        setRay(shadowRay, P, light.dir, 0.f, light.dist, Raytime);

	    	traceRay(model,shadowRay);
    		if (shadowRay.geomID < 0) {    // no intersection, light source is visible
		      color = color + light_contrib;
  		  }
      } */    // end shoot shadow ray
      // ushadowed light contribution: comment if shooting shadow rays
      color = color + light_contrib;
    }  // light weight > 0
  } // for all lights

  return (color);
}



/******
*
*  Does classic deterministic secondary ilumination
*
************/

vec3f secondary_classic (uniform MyRenderer *uniform self,
            World *uniform model,
						const DifferentialGeometry &dg,
						Ray &ray,
						const vec3f Ks, int depth) {

	vec3f color = make_vec3f (0.f);

	// Specular reflection
	if (reduce_max(Ks) > 0.f) {   // any potential contribution from specular ray ?
		Ray specRay;

		// compute reflection direction
		vec3f wi = 0.f - ray.dir;
		vec3f Rs = 2.f * dot (dg.Ns, wi) * dg.Ns - wi;
		// set reflection ray
		setRay(specRay, dg.P, Rs, dg.epsilon, inf, ray.time);
		vec3f specularC = recursive_classicRT (self, model, specRay, depth);
		color = color + Ks * specularC * dot(dg.Ns, Rs) ;
	}  // spec ray

	return (color);
}

/*
 *  recursive classical RT
 */
vec3f recursive_classicRT (uniform MyRenderer *uniform self,
                          World *uniform model,
                          Ray ray,  int depth)
{
    vec3f color, Kd, Ks;
    float d, ns;
    DifferentialGeometry dg;
    vec3f ambColor = self->ambColor;
    
    color = make_vec3f(0.f);
    
    // trace the ray
    traceRay(model, ray);

    // check whether this ray intersects a light source 
    // at a distance less than ray.t
    // if it does (l_eval.radiance != 0.f) 
    // then it is the light source that is intersected, 
    // rather than ray.geomID

    dg.P = ray.org;
    const uniform vec3f ZeroVec3 = make_vec3f(0.f);
    //calculate intersection for all lights
    for (uniform int i = 0; self->lights && i < self->numLights; i++) {
	    const uniform Light *uniform l = self->lights[i];

	    const Light_EvalRes l_eval = l->eval(l, dg, ray.dir, 0.f, ray.t);
	    if (ne(l_eval.radiance, ZeroVec3)) {  
        // returned radiane is not zero: INTERSECT
		    return(l_eval.radiance);
	    }
    }

    
    if (ray.geomID < 0) {  // No intersection
      color = make_vec3f(self->super.bgColor.x,self->super.bgColor.y,self->super.bgColor.z);
      return (color);
    }
    
    // from here on we know there was an intersection
    // get the intersection data onto dg

    // the declaration of DifferentialGeomtry datatype can be found in
    // "ospray/SDK/common/DifferentialGeometry.ih"
    
    // the prototype of postIntersect datatype can be found in
    // "ospray/SDK/common/World.ih"
    
    postIntersect (model, &self->super, dg, ray, DG_NG | DG_NS | DG_NORMALIZE | DG_FACEFORWARD | DG_TANGENTS | DG_COLOR | DG_TEXCOORD);
    
    // MyMaterial is defined in MyMaterial.[cpp|h|ispc|ih]
    // It currently supports the following data:
    //  vec3f kd,ks;
    // float ns, d;
    MyMaterial *materials = (MyMaterial *) dg.material;
    
    if (materials) {
      // get the intersection point material
      foreach_unique (mat in materials) {
        Kd = (mat ? mat->kd : make_vec3f (1.0f));
        Ks = (mat ? mat->ks : make_vec3f (0.f));
        ns = (mat ? mat->ns : 1.f);
        d = (mat ? mat->d : 1.f);
      }
    } else {
        Kd = make_vec3f (0.8f,0.5f,0.2f);
        Ks = make_vec3f (0.f);
        ns = 1.f;
        d = 1.f;
    }

    // compute the ambient light contribution
    vec3f ambient_contribution;

    ambient_contribution = Kd * ambColor;
    color = color + ambient_contribution;

    // direct illumination
    //color = color + direct_illumination (self, model, dg, Kd, ray.time);

    // secondary (specular) ray
    /*
    if ((depth+1) < self->super.maxDepth) {
      vec3f secondary = secondary_classic (self, model, dg, ray, Ks, depth+1);
      color = color + secondary;
    }*/


    return (color);
}



/*
 *  Tutorial 2: renderSample
 *  Entry point for recursive Whitted RT
 */
void MyRenderer_renderSample_RT (Renderer *uniform _self,
                                        FrameBuffer *uniform fb,
                                        World *uniform model,
                                        void *uniform perFrameData,
                                        varying ScreenSample &sample)
{
  uniform MyRenderer *uniform self = (uniform MyRenderer *uniform) _self;

  // get the primary ray
  Ray ray = sample.ray;
  
  // call recursive classicRT
  vec3f color = recursive_classicRT (self, model, ray, 0);
  
  sample.rgb = color;
  sample.alpha = 1.f;
  sample.z = ray.t;
}

