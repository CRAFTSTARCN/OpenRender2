#include "OpenRender2/Engine/Core/TickTimer.h"

void TickTimer::CalculateTime()
{
	auto Now = std::chrono::steady_clock::now();
	DeltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(Now - Last).count();
	Last = Now;
}

float TickTimer::GetDeltaTime()
{
	return DeltaTime;
}
