#pragma once
#include "OpenRenderRuntime/Core/RenderResource/RenderResource.h"

class IIBLPass
{
	
public:

	virtual ~IIBLPass() = default;
	
	virtual void OnUpdateIBLResource(const IBLResource& IBL) = 0;
};
