
#include "shader_model.h"

struct DefaultLitGBufferData
{
    vec3 base_color;
    float ambient_occlusion;
    vec3 normal;
    float subsurface;
    vec3 emissive;
    vec3 tangent;
    float anisotropy;

    float metallic, specular, roughness;
};

void EncodeDefaultLitGBuffer(
    in DefaultLitGBufferData data,
    out vec4 out_gbuffer_a,
    out vec4 out_gbuffer_b,
    out vec4 out_gbuffer_c,
    out vec4 out_gbuffer_d,
    out vec4 out_gbuffer_e)
{
    out_gbuffer_a.rgb = data.base_color;
    out_gbuffer_a.a = data.ambient_occlusion;
    out_gbuffer_b.rgb = (data.normal * 0.5 + 0.5);
    out_gbuffer_b.a = data.subsurface;
    out_gbuffer_c.r = data.metallic;
    out_gbuffer_c.g = data.specular;
    out_gbuffer_c.b = data.roughness;
    out_gbuffer_c.a = (float(DEFAULT_LIT) / float(255));
    out_gbuffer_d.rgb = data.emissive;
    out_gbuffer_e.rgb = (data.tangent * 0.5 + 0.5);
    out_gbuffer_e.a = data.anisotropy;
}

DefaultLitGBufferData DecodeDefaultLitGBuffer(
    in vec4 in_gbuffer_a, 
    in vec4 in_gbuffer_b, 
    in vec4 in_gbuffer_c, 
    in vec4 in_gbuffer_d, 
    in vec4 in_gbuffer_e) 
{
    DefaultLitGBufferData data;
    data.base_color = in_gbuffer_a.rgb;
    data.ambient_occlusion = in_gbuffer_a.a;
    data.normal = (in_gbuffer_b.rgb * 2.0 - 1.0);
    data.subsurface = in_gbuffer_b.a;
    data.emissive = in_gbuffer_d.rgb;
    data.metallic = in_gbuffer_c.r;
    data.specular = in_gbuffer_c.g;
    data.roughness = in_gbuffer_c.b;
    data.tangent = (in_gbuffer_e.rgb * 2.0 - 1.0);
    data.anisotropy = in_gbuffer_e.a;

    return data;
}