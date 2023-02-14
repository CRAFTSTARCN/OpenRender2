#include "ViewportClient.h"

#include <cassert>

ViewportClient::ViewportClient() = default;

void ViewportClient::Init(RenderWindowProxy* InWindowProxy)
{
	assert(InWindowProxy);
	RenderWindow = InWindowProxy;

	WindowSize Size = RenderWindow->GetWindowSize();

	WindowWidth = Size.Width;
	WindowHeight = Size.Height;

	RenderingWidth = Size.Width;
	RenderingHeight = Size.Height;

	/*
	 * Temp implement(no ui)
	 */

	RenderWindow->SetResizeFunction([this](uint32_t Width, uint32_t Height)
	{
		this->WindowWidth = Width;
		this->WindowHeight = Height;

		this->RenderingWidth = Width;
		this->RenderingHeight = Height;
	});
}

bool ViewportClient::ClientShutdown()
{
	return RenderWindow->WindowShouldClose();
}

uint32_t ViewportClient::GetWidth() const
{
	return WindowWidth;
}

uint32_t ViewportClient::GetHeight() const
{
	return WindowHeight;
}

uint32_t ViewportClient::GetRenderingWidth() const
{
	return RenderingWidth;
}

uint32_t ViewportClient::GetRenderingHeight() const
{
	return RenderingHeight;
}
