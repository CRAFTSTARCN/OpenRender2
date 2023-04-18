#pragma once
#include <cstdint>

#define MAX_INSTANCE_DRAW 1023u
#define WORKGROUP_SIZE 32u

struct InstanceData
{
	float mat_model[4][4];
	float mat_model_normal[4][4];
};

struct MeshDrawCallDataProxty
{
	uint32_t _meshlet_count;
	uint32_t _darw_instance_count;
	uint32_t _enable_culling;
	uint32_t _pre_instances_padding;

	InstanceData _instances[MAX_INSTANCE_DRAW];
};
