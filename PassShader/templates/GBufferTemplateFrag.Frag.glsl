#version 460

#extension GL_GOOGLE_include_directive : enable

#include "structures.h"

layout(std140, set = 0, binding = 0) uniform __rendering_global_data 
#include "global_data_uniform.inl"


[[MATERIAL_DATA]]

layout(location=0) in Interpolants 
{
    vec3 world_position;
    vec3 world_normal;
    vec3 world_tangent;
    vec2 uv;
} IN;

layout(location = 0) out vec4 gbuffer_a;
layout(location = 1) out vec4 gbuffer_b;
layout(location = 2) out vec4 gbuffer_c;
layout(location = 3) out vec4 gbuffer_d;
layout(location = 4) out vec4 gbuffer_e;


[[INCLUDE_FILES]]

[[FRAGMENT_GBUFFER_FUNCTION]]
// void fragment_function(
//     out vec4 gbuffer_a,
//     out vec4 gbuffer_b,
//     out vec4 gbuffer_c,
//     out vec4 gbuffer_d,
//     out vec4 gbuffer_e)

void main() 
{
    fragment_function(
        gbuffer_a,
        gbuffer_b,
        gbuffer_c,
        gbuffer_d,
        gbuffer_e);
}