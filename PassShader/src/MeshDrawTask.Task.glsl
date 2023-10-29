#version 450 

#extension GL_GOOGLE_include_directive : enable

#extension GL_EXT_control_flow_attributes: require
#define UNROLL_LOOP [[unroll]]

#extension GL_EXT_mesh_shader : require

#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
#extension GL_KHR_shader_subgroup_basic : require
#extension GL_KHR_shader_subgroup_ballot : require
#extension GL_KHR_shader_subgroup_vote : require

#include "structures.h"

#include "shader_def.h"

layout(local_size_x = WORKGROUP_SIZE) in;

layout(std140, set = 0, binding = 0) uniform _rendering_global_data 
#include "global_data_uniform.inl"



//Task shader doesn't need vertex data

layout(std430, set = 1, binding = 1) readonly buffer _meshlet_buffer
{
    Meshlet _meshlets[];
};

layout(std430, set = 3, binding = 0) readonly buffer _drawcall_data 
{
    uint _meshlet_count;
    uint _darw_instance_count;
    uint _enable_culling;
    uint _backface_culling;

    InstanceData _instances[MAX_INSTANCE_DRAW];
};

struct Task 
{
    uint base_id;
    uint8_t offset_id[MESHLET_SPAWN_PER_TASK];
    uint8_t instance_id[MESHLET_SPAWN_PER_TASK];
};

taskPayloadSharedEXT Task OUT;

#include "meshlet_util.glsl"

//Current no tesslation
void main()
{
    
    uint base_id = gl_WorkGroupID.x * WORKGROUP_SIZE;
    uint local_id = gl_LocalInvocationID.x;
    uint instance_id = gl_GlobalInvocationID.x / _meshlet_count;

    uint meshlet_index = gl_GlobalInvocationID.x % _meshlet_count;

    vec3 c = vec3(_meshlets[meshlet_index].CenterX,
                  _meshlets[meshlet_index].CenterY,
                  _meshlets[meshlet_index].CenterZ);

    vec3 e = vec3(_meshlets[meshlet_index].ExtendX,
                  _meshlets[meshlet_index].ExtendY,
                  _meshlets[meshlet_index].ExtendZ);

    vec3 center_world;
    bool render = (instance_id < _darw_instance_count) &&
                  early_culling(c, e, _instances[instance_id].mat_model, camera_frustum, center_world);
    render = render && ((_backface_culling == 0) || 
                       backface_culling(camera_position, center_world, _instances[instance_id].mat_model, _meshlets[meshlet_index].Cone, _meshlets[meshlet_index].ApexOffset));
    
    uvec4 vote = subgroupBallot(render);
    uint meshlet_num = subgroupBallotBitCount(vote);
    uint offset_index = subgroupBallotExclusiveBitCount(vote);


    if(render) 
    {
        OUT.offset_id[offset_index] = uint8_t(local_id);
        OUT.instance_id[offset_index] = uint8_t(instance_id);
    }

    if(local_id == 0) 
    {
        OUT.base_id = base_id;
    }    
    barrier();
    EmitMeshTasksEXT(meshlet_num, 1, 1);
}