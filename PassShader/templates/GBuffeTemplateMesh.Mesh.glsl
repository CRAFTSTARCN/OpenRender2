#version 460

#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_control_flow_attributes: require
#define UNROLL_LOOP [[unroll]]

#extension GL_EXT_mesh_shader : require

#extension GL_EXT_shader_explicit_arithmetic_types_int8  : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "shader_def.h"

#include "structures.h"


layout(local_size_x = WORKGROUP_SIZE) in;
layout(max_vertices = VERTEX_OUTPUT_SIZE, max_primitives = PRIM_OUTPUT_SIZE) out;
layout(triangles) out;

layout(std140, set = 0, binding = 0) uniform _rendering_global_data 
#include "global_data_uniform.inl"


layout(std430, set = 1, binding = 0) readonly buffer _vertex_buffer
{
    VertexStructure _vertices[];
};

layout(std430, set = 1, binding = 1) readonly buffer _meshlet_buffer
{
    Meshlet _meshlets[];
};

layout(std430, set = 1, binding = 2) readonly buffer _vertex_index_buffer
{
    uint _vertex_indices[];
};

layout(std430, set = 1, binding = 3) readonly buffer _primitive_buffer 
{
    uint8_t _primitive_indices[];
};

[[MATERIAL_DATA]]

layout(std430, set = 3, binding = 0) readonly buffer _drawcall_data 
{
    uint _meshlet_count;
    uint _darw_instance_count;
    uint _enable_culling;
    uint _pre_instances_padding_2;

    InstanceData _instances[MAX_INSTANCE_DRAW];
};

struct Task 
{
    uint base_id;
    uint8_t offset_id[MESHLET_SPAWN_PER_TASK];
    uint8_t instance_id[MESHLET_SPAWN_PER_TASK];

};

taskPayloadSharedEXT Task IN;

const uint MESHLET_VERTEX_ITERATIONS = ((VERTEX_OUTPUT_SIZE + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE);
const uint MESHLET_PRIMITIVE_ITERATIONS = ((PRIM_OUTPUT_SIZE + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE);

layout(location=0) out Interpolants 
{
    vec3 world_position;
    vec3 world_normal;
    vec3 world_tangent;
    vec2 uv;
} OUT[];

[[INCLUDE_FILES]]

[[MESH_VERTEX_PROCESSOR]]
//void process_vertex(in VertexStructure vertex, in InstanceData instance, uint vertex_id);

void main() 
{
    uint meshlet_id = IN.base_id + IN.offset_id[gl_WorkGroupID.x];
    uint meshlet_vert_count = _meshlets[meshlet_id].VertexAndPrimCount & 0xffff;
    uint meshlet_prim_count = _meshlets[meshlet_id].VertexAndPrimCount >> 16;
    uint meshlet_vert_start = _meshlets[meshlet_id].VertexBegin;
    uint meshlet_prim_start = _meshlets[meshlet_id].PrimIndexBegin;

    uint instance_id = IN.instance_id[gl_WorkGroupID.x];

    SetMeshOutputsEXT(meshlet_vert_count, meshlet_prim_count);

    //VERTEX 
    {
        UNROLL_LOOP
        for(uint i=0; i<MESHLET_VERTEX_ITERATIONS; ++i) 
        {
            uint vert_id = gl_LocalInvocationID.x + i * WORKGROUP_SIZE;
            uint vert_load = min(vert_id, meshlet_vert_count-1);
            uint vert_index = _vertex_indices[meshlet_vert_start + vert_id];
            process_vertex(_vertices[vert_id], _instances[instance_id], vert_id);
        }
    }

    //PRIMITIVE
    {
        UNROLL_LOOP
        for(uint i=0; i<MESHLET_PRIMITIVE_ITERATIONS; ++i)
        {
            uint prim_id = gl_LocalInvocationID.x + i * WORKGROUP_SIZE;
            uint prim_load = min(prim_id, meshlet_prim_count - 1);

            uvec3 indices = uvec3(
                _primitive_indices[meshlet_prim_start + prim_load * 3],
                _primitive_indices[meshlet_prim_start + prim_load * 3 + 1],
                _primitive_indices[meshlet_prim_start + prim_load * 3 + 2]);

            if(prim_id < meshlet_prim_count)
            {
                gl_PrimitiveTriangleIndicesEXT[prim_id] = indices;
                //gl_MeshPrimitivesEXT[prim_id].gl_PrimitiveID = int(prim_id); //Use inner meshlet primitive ID
            }
        }
    }
}