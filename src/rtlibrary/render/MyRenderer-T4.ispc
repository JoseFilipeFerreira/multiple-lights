
#include "MyRenderer-T4.ih"
#include "MyMaterial.ih"
#include "ospray/SDK/common/World.ih"
#include "ospray/SDK/math/sampling.ih"

#include <stdio.h>


/******
*
*  Does classic deterministic direct illumination
*
************/

vec3f direct_illumination (uniform MyRenderer *uniform self,
            World *uniform model,
            varying RNGState * uniform rng,
						const DifferentialGeometry &dg,
						const vec3f Kd,
						const float Raytime) {

  vec3f color = make_vec3f(0.f);
  
  //calculate shading for all lights
  for (uniform int i = 0; self->lights && i < self->numLights; i++) {
  	const uniform Light *uniform l = self->lights[i];

    //const vec2f s = make_vec2f(0.0f); // sample center of area lights
    // stochastically sample the area lights
    const vec2f s = make_vec2f(frandom(rng),frandom(rng)); 
    
    const Light_SampleRes light = l->sample(l, dg, s);

    if (reduce_max(light.weight) > 0.f) {   // any potential contribution?
      float cosNL = dot(light.dir, dg.Ns);  // cosine between lighting dir and normal
      if (cosNL < 0.0f) continue;  // light on the other side

      const vec3f diffuse = Kd * cosNL;
      const vec3f light_contrib = diffuse * light.weight;

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
      }
    }  // light weight > 0
  } // for all lights



  return (color);
}



/******
*
*  Does classic deterministic secondary ilumination
*
************/

vec3f secondary_PT (uniform MyRenderer *uniform self,
            World *uniform model,
            varying RNGState * uniform rng,
						const DifferentialGeometry &dg,
						Ray &ray,
						const vec3f Kd, const vec3f Ks, const float ns,
            int depth) {

	vec3f color = make_vec3f (0.f);

  float SPECULAR_PROB=0.5f;

  /*float normalizing_constant = (length(Kd)+length(Ks));
  normalizing_constant = (normalizing_constant <= 1.e-5 ? 1.f : normalizing_constant);
  SPECULAR_PROB=length(Ks) / normalizing_constant;*/

  bool do_specular;
  float r = frandom(rng);
  do_specular = (r < SPECULAR_PROB ? true : false);

  do_specular = true;
  SPECULAR_PROB = 1.f;

	if (do_specular)  {     // follow specular reflection path
	  if (reduce_max(Ks) > 0.f) {   // any potential contribution from specular ray ?
		  Ray specRay;

		  // compute reflection direction
		  vec3f wi = 0.f - ray.dir;
		  vec3f Rs = 2.f * dot (dg.Ns, wi) * dg.Ns - wi;

      // sampling the BRDF glossy lobe
      // get a random direction distributed over a cosine lobe 
      // to the power of N
      // this direction is local, i.e. the lobe is centered around the Z axis
      const vec2f s = make_vec2f(frandom(rng), frandom(rng));
      const vec3f local_glossy_dir = powerCosineSampleHemisphere(ns, s);
      // rotate the local direction
      // to a lobe centered around the ideal specular reflection direction
      const vec3f glossy_dir = frame(Rs) * local_glossy_dir;

      // set the glossy reflection ray
      setRay(specRay, dg.P, glossy_dir, dg.epsilon, inf, ray.time);

		  // set reflection ray
		  // setRay(specRay, dg.P, Rs, dg.epsilon, inf, ray.time);
		  vec3f specularC = recursive_PT (self, model, rng, specRay, depth);
      specularC = specularC / SPECULAR_PROB;
		  color = color + Ks * specularC * dot(dg.Ns, Rs) ;
	  }  // spec ray
  }
  else {    // follow diffuse reflection path
  	if (reduce_max(Kd) > 0.f) {   // any potential contribution from diffuse ray ?
		  Ray diffRay;

      // sampling the hemisphere 
      // get a random direction distributed over the hemisphere 
      // this direction is local, i.e. is centered around the Z axis
      const vec2f s = make_vec2f(frandom(rng), frandom(rng)); 
      const vec3f local_diff_dir = cosineSampleHemisphere(s);

      const vec3f diff_dir = frame(dg.Ns) * local_diff_dir; 
      setRay(diffRay, dg.P, diff_dir, dg.epsilon, inf, ray.time);

      vec3f diffC = recursive_PT (self, model, rng, diffRay, depth);
      diffC = diffC / (1.f - SPECULAR_PROB);
      color = color + Kd * diffC * pi ;
    }
  }

	return (color);
}

/*
 *  recursive path tracing
 */
vec3f recursive_PT (uniform MyRenderer *uniform self,
                          World *uniform model,
                          varying RNGState * uniform rng,
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

    //ambient_contribution = Kd * ambColor;
    //color = color + ambient_contribution;

    // direct illumination
    color = color + direct_illumination (self, model, rng, dg, Kd, ray.time);

    
    // secondary ray : constant MAX DEPTH
    if ((depth+1) < self->super.maxDepth) {
      vec3f secondary = secondary_PT (self, model, rng, dg, ray, Kd, Ks, ns, depth+1);
      color = color + secondary;
    } 

    // secondary ray : Russian roulette
    /*
    const float continue_p = 0.9f;
    if (frandom(rng) <= continue_p) {
      vec3f secondary = secondary_PT (self, model, rng, dg, ray, Kd, Ks, ns, depth+1);
    	secondary = secondary / continue_p;
      color = color + secondary;
    }*/


    return (color);
}



/*
 *  Tutorial 4: renderSample
 *  Entry point for recursive path tracing
 */
void MyRenderer_renderSample_RT (Renderer *uniform _self,
                                        FrameBuffer *uniform fb,
                                        World *uniform model,
                                        void *uniform perFrameData,
                                        varying ScreenSample &sample)
{
  uniform MyRenderer *uniform self = (uniform MyRenderer *uniform) _self;

  varying RNGState state_RNG;
  varying RNGState * uniform rng = &state_RNG;
  int seed;
  while (rdrand(&seed) == false) ; 
  seed_rng(rng, seed);

  // get the primary ray
  Ray ray = sample.ray;
  
  // call recursive path tracing
  vec3f color = recursive_PT (self, model, rng, ray, 0);
  
  sample.rgb = color;
  sample.alpha = 1.f;
  sample.z = ray.t;
}

