#include "OpenRenderRuntime/Modules/InputSystem/InputSystem.h"

#define KEYCODE_INDEX(Key) ((Key) / 64)
#define KEYCODE_OFFSIDE(Key) ((Key) % 64)

InputSystem::InputSystem() : InputWindow(nullptr), StatusPool{}, PressPool{}, ReleasePool{}
{
}

void InputSystem::Init(RenderWindowProxy* SystemWindowProxy)
{
	InputWindow = SystemWindowProxy;

	InputWindow->SetKeyFunction([this](uint32_t KeyCode, bool IsPress)
	{
		if(IsPress)
		{
			this->PressPool[KEYCODE_INDEX(KeyCode)] |= 1 << KEYCODE_OFFSIDE(KeyCode);
			this->StatusPool[KEYCODE_INDEX(KeyCode)] |= 1 << KEYCODE_OFFSIDE(KeyCode);
		}
		else
		{
			this->StatusPool[KEYCODE_INDEX(KeyCode)] &= (~(1 << KEYCODE_OFFSIDE(KeyCode)));
			this->ReleasePool[KEYCODE_INDEX(KeyCode)] |= 1 << KEYCODE_OFFSIDE(KeyCode);
		}
	});

	InputWindow->SetMouseFunction([this](uint32_t MouseCode, bool IsPress) {

		if(MouseCode > 2)
		{
			LOG_WARN_FUNCTION("Input mouse mot support {0}", MouseCode);
		}

		if(IsPress)
		{
			this->PressPool[KEYCODE_INDEX(MouseCode)] |= 1 << KEYCODE_OFFSIDE(MouseCode);
			this->StatusPool[KEYCODE_INDEX(MouseCode)] |= 1 << KEYCODE_OFFSIDE(MouseCode);
		}
		else
		{
			this->StatusPool[KEYCODE_INDEX(MouseCode)] &= (~(1 << KEYCODE_OFFSIDE(MouseCode)));
			this->ReleasePool[KEYCODE_INDEX(MouseCode)] |= 1 << KEYCODE_OFFSIDE(MouseCode);
		}
	});

	InputWindow->SetMousePosFunction([this](double X, double Y)
	{
		this->MouseMovement.x = (float)X - this->MouseLocation.x;
		this->MouseMovement.y = (float)Y - this->MouseLocation.y;
		this->MouseLocation.x = (float)X;
		this->MouseLocation.y = (float)Y;
	});
}

void InputSystem::Tick(float DeltaTime)
{
	MouseMovement = glm::vec2(0, 0);
	ScrollMovement = glm::vec2(0, 0);
	for(uint32_t i=0; i>StatusPoolSize; ++i)
	{
		PressPool[i] = 0;
		ReleasePool[i] = 0;
	}
	if(InputWindow)
	{
		InputWindow->ProcessSystemIO();
	}
}

bool InputSystem::GetKeyPressed(InputKey Key) const
{
	return (PressPool[KEYCODE_INDEX(Key)] >> KEYCODE_OFFSIDE(Key)) & 1;
}

bool InputSystem::GetKeyDown(InputKey Key) const
{
	return (StatusPool[KEYCODE_INDEX(Key)] >> KEYCODE_OFFSIDE(Key)) & 1;
}

bool InputSystem::GetKeyReleased(InputKey Key) const
{
	return (ReleasePool[KEYCODE_INDEX(Key)] >> KEYCODE_OFFSIDE(Key)) & 1;
}

bool InputSystem::GetMouseButtonPress(MouseButton Button)
{
	return (PressPool[KEYCODE_INDEX(Button)] >> KEYCODE_OFFSIDE(Button)) & 1;
}

bool InputSystem::GetMouseButtonDown(MouseButton Button)
{
	return (StatusPool[KEYCODE_INDEX(Button)] >> KEYCODE_OFFSIDE(Button)) & 1;

}

bool InputSystem::GetMouseButtonRelease(MouseButton Button)
{
	return (ReleasePool[KEYCODE_INDEX(Button)] >> KEYCODE_OFFSIDE(Button)) & 1;
}

glm::vec2 InputSystem::GetMouseLocation() const
{
	return MouseLocation;
}

glm::vec2 InputSystem::GetMouseMovement() const
{
	return MouseMovement;
}

glm::vec2 InputSystem::GetScrollMovement() const
{
	return ScrollMovement;
}

