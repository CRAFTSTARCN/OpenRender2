#include "shader_model.h"

#ifndef UNLIT_UTIL_GLSL
#define UNLIT_UTIL_GLSL

void EncodeUnlitGBuffer(in vec3 emissive, out vec4 out_gbuffer_c, out vec4 out_gbuffer_d) 
{
    out_gbuffer_c.a = (float(UNLIT) / float(255));
    out_gbuffer_d.rgb = emissive;
}
#endif