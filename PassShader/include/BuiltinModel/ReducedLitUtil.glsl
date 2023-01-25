#include "shader_model.h"

struct ReducedLitGBufferData 
{
    vec3 base_color;
    float ambient_occlusion;
    vec3 normal;
    vec3 emmisive;

    float metallic, specular, roughness;
};

void EncodeReducedLitGBuffer(
    in ReducedLitGBufferData data,
    out vec4 out_gbuffer_a,
    out vec4 out_gbuffer_b,
    out vec4 out_gbuffer_c,
    out vec4 out_gbuffer_d)
{
    out_gbuffer_a.rgb = data.base_color;
    out_gbuffer_a.a = data.ambient_occlusion;
    out_gbuffer_b.rgb = (data.normal * 0.5 + 0.5);
    out_gbuffer_c.r = data.metallic;
    out_gbuffer_c.g = data.specular;
    out_gbuffer_c.b = data.roughness;
    out_gbuffer_c.a = (float(DEFAULT_LIT) / float(255));
    out_gbuffer_d.rgb = data.emmisive;
}

ReducedLitGBufferData DecodeReducedLitGBuffer(
    in vec4 in_gbuffer_a,
    in vec4 in_gbuffer_b,
    in vec4 in_gbuffer_c,
    in vec4 in_gbuffer_d) 
{
    ReducedLitGBufferData  data;
    data.base_color = in_gbuffer_a.rgb;
    data.ambient_occlusion = in_gbuffer_a.a;
    data.normal = (in_gbuffer_b.rgb * 2.0 - 1.0);
    data.emmisive = in_gbuffer_d.rgb;
    data.metallic = in_gbuffer_c.r;
    data.specular = in_gbuffer_c.g;
    data.roughness = in_gbuffer_c.b;

    return data;
}