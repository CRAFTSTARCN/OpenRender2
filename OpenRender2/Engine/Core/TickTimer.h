#pragma once
#include <chrono>

class TickTimer
{
	std::chrono::time_point<std::chrono::steady_clock> Last;
	float DeltaTime = 0.0f;
public:

	void CalculateTime();

	float GetDeltaTime();
};
