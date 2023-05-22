#version 460

#extension GL_GOOGLE_include_directive : enable

#include "shader_def.h"
#include "structures.h"

layout(std140, set = 0, binding = 0) uniform _rendering_global_data 
#include "global_data_uniform.inl"

layout(set = 0, binding = 1) uniform sampler2D BRDF_LUT;
layout(set = 0, binding = 2) uniform samplerCube radiance_map;
layout(set = 0, binding = 3) uniform samplerCube irradiance_map;
layout(set = 0, binding = 4) uniform samplerCube skybox;
//layout(set = 0, binding = 5) uniform sampler2DArray directional_light_shaodw_map;


layout(input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput in_gbuffer_a;
layout(input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput in_gbuffer_b;
layout(input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput in_gbuffer_c;
layout(input_attachment_index = 3, set = 1, binding = 3) uniform subpassInput in_gbuffer_d;
layout(input_attachment_index = 4, set = 1, binding = 4) uniform subpassInput in_gbuffer_e;
layout(input_attachment_index = 5, set = 1, binding = 5) uniform subpassInput in_scene_depth;

layout(location = 0) in Interpolants 
{
    vec2 NDC;
} IN;

layout(location = 0) out vec4 out_frag_color;

vec3 NDC_2_W_inl(vec2 pixel_NDC, float detph)
{
    vec4 NDC_S = vec4(pixel_NDC, detph, 1.0);
    vec4 WS_H = mat_view_proj_inv * NDC_S;
    return WS_H.xyz / WS_H.www;
}

uint decode_shader_model_id(float v)
{
    return uint(round(v * float(255)));
}

#include "BuiltinModel/DefaultLitUtil.glsl"
#include "BuiltinModel/ReducedLitUtil.glsl"
#include "PBR/BRDF.glsl"

void main()
{

    vec4 gbuffer_a = subpassLoad(in_gbuffer_a).rgba;
    vec4 gbuffer_b = subpassLoad(in_gbuffer_b).rgba;
    vec4 gbuffer_c = subpassLoad(in_gbuffer_c).rgba;
    vec4 gbuffer_d = subpassLoad(in_gbuffer_d).rgba;
    vec4 gbuffer_e = subpassLoad(in_gbuffer_e).rgba;
    float scene_depth = subpassLoad(in_scene_depth).r;

    uint shader_model_id = decode_shader_model_id(gbuffer_c.a);

    vec3 world_position = NDC_2_W_inl(IN.NDC, scene_depth);


    if(shader_model_id == SKYBOX) 
    {
        vec3 skybox_uvw = normalize(world_position - camera_position);
        out_frag_color = vec4(textureLod(skybox, skybox_uvw, 0.0).rgb, 1.0);
        //out_frag_color = vec4(normalize(abs(world_position)), 1.0);
    }
    else if(shader_model_id == UNLIT) 
    {
        out_frag_color = vec4(gbuffer_d.rgb, 1.0);
    } 
    else if(shader_model_id == DEFAULT_LIT) 
    {
        DefaultLitGBufferData data = DecodeDefaultLitGBuffer(gbuffer_a, gbuffer_b, gbuffer_c, gbuffer_d, gbuffer_e);
        vec3 B = cross(data.tangent, data.normal);
        vec3 F0 = get_F0(data.base_color, data.specular, data.metallic);
        vec3 V = normalize(camera_position - world_position);
        vec3 R = reflect(-V, data.normal);

        vec3 default_lit_BRDF = BRDF_HQ(
            directional_light_direction, 
            V, 
            data.normal, 
            data.tangent, 
            B, 
            F0,
            data.base_color,
            data.metallic,
            data.roughness,
            data.subsurface,
            data.anisotropy);
        
        vec3 L_in = directional_light_color * dot(directional_light_direction, data.normal);
        vec3 L_reflect = L_in * default_lit_BRDF;
        
        float radiance_lod = 5.0 * data.roughness;
        vec3 radiance_L_in = textureLod(radiance_map, R, radiance_lod).xyz;
        vec3 L_irradiance = textureLod(irradiance_map, data.normal, 0.0).xyz;
        float dot_nv = clamp(dot(data.normal, V), 0.0 ,1.0);
        vec2 BRDF_IBL = textureLod(BRDF_LUT, vec2(dot_nv,  1.0 - data.roughness), 0.0).rg;
        vec3 F_IBL = F_schlick_roughness(F0, dot_nv, data.roughness);
        vec3 L_IBL = radiance_L_in * (F_IBL * BRDF_IBL.x + BRDF_IBL.y) + 
                     (vec3(1.0, 1.0, 1.0) - F_IBL) * (1.0 - data.metallic) * L_irradiance * data.base_color;

        out_frag_color = vec4(L_reflect + L_IBL + data.emissive, 1.0);
    }
    else if(shader_model_id == REDUCED_LIT)
    {
        ReducedLitGBufferData data = DecodeReducedLitGBuffer(gbuffer_a, gbuffer_b, gbuffer_c, gbuffer_d);
        vec3 F0 = get_F0(data.base_color, data.specular, data.metallic);
        vec3 V = normalize(camera_position - world_position);
        vec3 R = reflect(-V, data.normal);

        vec3 default_lit_BRDF = BRDF_MQ(
            directional_light_direction, 
            V, 
            data.normal, 
            F0,
            data.base_color,
            data.metallic,
            data.roughness);
        
        vec3 L_in = directional_light_color * dot(directional_light_direction, data.normal);
        vec3 L_reflect = L_in * default_lit_BRDF;
        
        float radiance_lod = 5.0 * data.roughness;
        vec3 radiance_L_in = textureLod(radiance_map, R, radiance_lod).xyz;
        vec3 L_irradiance = textureLod(irradiance_map, data.normal, 0.0).xyz;
        float dot_nv = clamp(dot(data.normal, V), 0.0 ,1.0);
        vec2 BRDF_IBL = textureLod(BRDF_LUT, vec2(dot_nv,  1.0 - data.roughness), 0.0).rg;
        vec3 F_IBL = F_schlick_roughness(F0, dot_nv, data.roughness);
        vec3 L_IBL = radiance_L_in * (F_IBL * BRDF_IBL.x + BRDF_IBL.y) + 
                     (vec3(1.0, 1.0, 1.0) - F_IBL) * (1.0 - data.metallic) * L_irradiance * data.base_color;

        out_frag_color = vec4(L_reflect + L_IBL + data.emissive, 1.0);
    }
    else
    {
        out_frag_color = vec4(1.0, 0.0, 1.0, 1.0);
    }
}
