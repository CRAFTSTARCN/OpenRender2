#pragma once
#include "OpenRenderRuntime/Core/Basic/BoundingVolumes.h"
#include "OpenRenderRuntime/Core/RHI/RHIBuffer.h"
#include "OpenRenderRuntime/Core/RHI/RHIDescriptor.h"

class RenderMesh
{
    
public:

   inline static AutoIncreaseIdAllocator Registry {0};
   constexpr static size_t BAD_MESH_ID = SIZE_MAX;
   
   size_t Id = 0;
   AABB OriginalAABB {};

   uint32_t MeshletCount = 0;
   
   RHIBuffer* VertexBuffer = nullptr;
   RHIBuffer* MeshletBuffer = nullptr;
   RHIBuffer* VertexIndexBuffer =nullptr;
   RHIBuffer* PrimIndexBuffer = nullptr;

   RHIDescriptorSet* Descriptor = nullptr;
};
