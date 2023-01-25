#version 460

#extension GL_EXT_control_flow_attributes: require
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_mesh_shader : require

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(max_vertices = 4, max_primitives = 2) out;
layout(triangles) out;

layout(location = 0) out Interpolants 
{
    vec2 NDC;
} OUT[];

void main()
{
    SetMeshOutputsEXT(4, 2);

    gl_MeshVerticesEXT[0].gl_Position = vec4(1.0, 1.0, 0.5, 1.0);
    OUT[0].NDC = vec2(1.0, 1.0);
    gl_MeshVerticesEXT[1].gl_Position = vec4(-1.0, 1.0, 0.5, 1.0);
    OUT[1].NDC = vec2(-1.0, 1.0);
    gl_MeshVerticesEXT[2].gl_Position = vec4(-1.0, -1.0, 0.5, 1.0);
    OUT[2].NDC = vec2(-1.0, -1.0);
    gl_MeshVerticesEXT[3].gl_Position = vec4(1.0, -1.0, 0.5, 1.0);
    OUT[3].NDC = vec2(1.0, -1.0);

    gl_PrimitiveTriangleIndicesEXT[0] = uvec3(0, 1, 2);
    gl_PrimitiveTriangleIndicesEXT[1] = uvec3(0, 2, 3);
}