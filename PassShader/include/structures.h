#ifndef _STRUCTURE_H_
#define _STRUCTURE_H_

struct VertexStructure
{
    float x,y,z;
    float Nx, Ny, Nz;
    float Tx,Ty,Tz;
    float u,v;
};

struct Meshlet 
{
    uint VertexAndPrimCount;   //16bit encoded
	uint VertexBegin; 
	uint PrimIndexBegin;
	float CenterX, CenterY, CenterZ;
	float ExtendX, ExtendY, ExtendZ;
    uint Cone;
    float ApexOffset;
};

struct PointLight 
{
    vec3 color;
    float radius;
    vec3 position;
    float position_padding;
};

struct InstanceData 
{
    mat4 mat_model;
    mat4 mat_model_normal;
};

struct Frustum 
{
    vec4 planes[6];
};

#endif