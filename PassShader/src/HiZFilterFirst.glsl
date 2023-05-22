#version 460


#extension GL_EXT_control_flow_attributes: require
#extension GL_GOOGLE_include_directive : enable

layout(set = 0, binding = 0) uniform texture2D ori_depth_texture;
layout(set = 0, binding = 1, r32) uniform image2D first_level_hi_z;

layout(set = 0, binding = 2) uniform size_data 
{
    uvec2 mapping_scale;
    uvec2 first_level_size;
};

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

void main() 
{
    if(gl_GlobalInvocationID.x >= first_level_size.x || gl_GlobalInvocationID.y >= first_level_size.y)
    {
        return;
    }

    float nearest_z_value = 1.0;
    
    for(int i = 0; i < mapping_scale.x; ++i) 
    {

    }
}