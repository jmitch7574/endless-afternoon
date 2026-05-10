#pragma once

#include "raylib.h"

class ScreenShake
{
  public:
	static void Update(float deltaTime);
	static void Shake(float strength, float duration);
	static void Stop();
	static Vector2 GetOffset();
};
