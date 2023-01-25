#pragma once
#include "OpenRenderRuntime/Modules/InputSystem/InputKey.h"
#include "OpenRenderRuntime/Core/RHI/RenderWindowProxy.h"
#include "OpenRenderRuntime/Core/Basic/RenderMath.h"

/*
 * Basic input system
 */
class InputSystem
{

	static constexpr uint32_t StatusPoolSize = (InputKey::InputKey_MAX + 65) / 64;
	
	RenderWindowProxy* InputWindow;

	glm::vec2 MouseLocation;
	glm::vec2 MouseMovement;
	glm::vec2 ScrollMovement;

	uint64_t StatusPool[StatusPoolSize];
	uint64_t PressPool[StatusPoolSize];
	uint64_t ReleasePool[StatusPoolSize];

	
public:

	InputSystem();
	void Init(RenderWindowProxy* SystemWindowProxy);
	
	void Tick(float DeltaTime);

	bool GetKeyPressed(InputKey Key) const;
	bool GetKeyDown(InputKey Key) const;
	bool GetKeyReleased(InputKey Key) const;

	bool GetMouseButtonPress(MouseButton Button);
	bool GetMouseButtonDown(MouseButton Button);
	bool GetMouseButtonRelease(MouseButton Button);

	
	glm::vec2 GetMouseLocation() const;
	glm::vec2 GetMouseMovement() const;
	glm::vec2 GetScrollMovement() const;
};
