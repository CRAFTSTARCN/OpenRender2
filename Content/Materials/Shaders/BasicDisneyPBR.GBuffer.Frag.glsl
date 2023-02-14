#version 450

#extension GL_GOOGLE_include_directive : enable


//#define _DEBUG_MESHLET

#include "structures.h"

layout(std140, set = 0, binding = 0) uniform _rendering_global_data 
#include "global_data_uniform.inl"

layout(std140, set = 2, binding = 0) uniform _material_uniform 
{
    vec4 Emissive;

    float Metallic;
    float Specular;
    float Roughness;
    float Subsurface;
    float Anisotropy;

    float _end_padding_0;
    float _end_padding_1;
    float _end_padding_2;
};

layout(set = 2, binding = 1) uniform sampler2D BaseColor;

layout(location=0) in Interpolants 
{
    vec3 world_position;
    vec3 world_normal;
    vec3 world_tangent;
    vec2 uv;

#ifdef _DEBUG_MESHLET
    vec3 debug_color;
#endif

} IN;

layout(location = 0) out vec4 gbuffer_a;
layout(location = 1) out vec4 gbuffer_b;
layout(location = 2) out vec4 gbuffer_c;
layout(location = 3) out vec4 gbuffer_d;
layout(location = 4) out vec4 gbuffer_e;


#include "BuiltinModel/DefaultLitUtil.glsl"
#include "BuiltinModel/UnlitUtil.glsl"

void fragment_function(
    out vec4 gbuffer_a,
    out vec4 gbuffer_b,
    out vec4 gbuffer_c,
    out vec4 gbuffer_d,
    out vec4 gbuffer_e)
{
#ifdef _DEBUG_MESHLET
    EncodeUnlitGBuffer(IN.debug_color, gbuffer_c, gbuffer_d); //texture(BaseColor, IN.uv).rgb;
#else
    
    DefaultLitGBufferData data;
    data.base_color = texture(BaseColor, IN.uv).rgb;
    data.ambient_occlusion = 1.0;
    data.normal = normalize(IN.world_normal);
    data.subsurface = Subsurface;
    data.emissive = Emissive.rgb;
    data.tangent = normalize(IN.world_tangent);
    data.anisotropy = Anisotropy;
    data.metallic = Metallic;
    data.specular = Specular;
    data.roughness = Roughness;
    EncodeDefaultLitGBuffer(
    data, 
    gbuffer_a,
    gbuffer_b,
    gbuffer_c,
    gbuffer_d,
    gbuffer_e);
#endif

}
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