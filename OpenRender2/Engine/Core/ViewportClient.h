#pragma once
#include "OpenRenderRuntime/Core/RHI/RenderWindowProxy.h"

class ViewportClient
{

	RenderWindowProxy* RenderWindow = nullptr;

	uint32_t WindowWidth{0}, WindowHeight{0};

	/*
	 * For UI 
	 */
	uint32_t RenderingWidth{0}, RenderingHeight{0};
	uint32_t RenderingOffsetX{0}, RenderingOffsetY{0};

	//TODO: resize function
	
public:

	
	ViewportClient();

	void Init(RenderWindowProxy* InWindowProxy);
	bool ClientShutdown();

	uint32_t GetWidth() const;
	uint32_t GetHeight() const;

	uint32_t GetRenderingWidth() const;
	uint32_t GetRenderingHeight() const;
	
};
