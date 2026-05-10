#include "screen_shake.h"
#include <algorithm>

namespace
{
float shakeTimer = 0.0f;
float shakeDuration = 0.0f;
float shakeStrength = 0.0f;
Vector2 currentOffset = Vector2{0.0f, 0.0f};

float RandomUnit()
{
	return (float)GetRandomValue(-1000, 1000) / 1000.0f;
}
} // namespace

void ScreenShake::Update(float deltaTime)
{
	if (shakeTimer <= 0.0f)
	{
		currentOffset = Vector2{0.0f, 0.0f};
		return;
	}

	shakeTimer = std::max(shakeTimer - deltaTime, 0.0f);

	const float life = shakeDuration > 0.0f ? shakeTimer / shakeDuration : 0.0f;
	const float easedStrength = shakeStrength * life * life;
	currentOffset = Vector2{RandomUnit() * easedStrength, RandomUnit() * easedStrength};

	if (shakeTimer <= 0.0f)
	{
		currentOffset = Vector2{0.0f, 0.0f};
	}
}

void ScreenShake::Shake(float strength, float duration)
{
	if (strength <= 0.0f || duration <= 0.0f)
	{
		return;
	}

	if (shakeTimer <= 0.0f)
	{
		shakeStrength = strength;
		shakeDuration = duration;
		shakeTimer = duration;
		return;
	}

	shakeStrength = std::max(shakeStrength, strength);
	shakeDuration = std::max(shakeDuration, duration);
	shakeTimer = std::max(shakeTimer, duration);
}

void ScreenShake::Stop()
{
	shakeTimer = 0.0f;
	shakeDuration = 0.0f;
	shakeStrength = 0.0f;
	currentOffset = Vector2{0.0f, 0.0f};
}

Vector2 ScreenShake::GetOffset() { return currentOffset; }
