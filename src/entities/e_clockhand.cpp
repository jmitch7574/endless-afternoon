#include "entity.h"
#include <cmath>
#include "custom_draws.h"
#include "utils.h"
#include <scene.h>
#include <arena_manager.h>
#include <iostream>


ClockHand::ClockHand(raylib::Vector2 pivot, float angleDeg, float length, float thickness, Color color)
	: Entity(pivot), angleDeg(angleDeg), length(length), thickness(thickness), color(color)
{
}
ClockHand::~ClockHand(void) {}

void ClockHand::Update() 
{
  if (inBigDeadlySpin) 
  {
    bigDeadlySpinTime += GetFrameTime();

    if (bigDeadlySpinTime >= 9)
    {
      inBigDeadlySpin = false;
      bigDeadlySpinTime = 0;
    }
  }

  
  if (isAdvancing) 
  {
    advanceTime += GetFrameTime();
    angleDeg += GetFrameTime() * 15.0f;

    if (advanceTime >= 2)
    {
      isAdvancing = false;
    }
  }
}
void ClockHand::Draw()
{
  CustomDraws::DrawArrow(position, GetAngle(), 450, 10, 35, 35, activated ? WHITE : Color(80, 80, 80, 255));
}

void ClockHand::Advance() 
{
  if (isAdvancing) return;
  isAdvancing = true;
  advanceTime = 0;
}

void ClockHand::BeginBigDeadlySpin() 
{
  bigDeadlySpinTime = 0;
  inBigDeadlySpin = true;
}

Vector2 ClockHand::GetLargeExtendedPoint() 
{ 
  return Vector2Add(position, Vector2Scale(Utils::AngleToVector2(GetAngle()), 10000));
}

float ClockHand::GetAngle()
{
  return angleDeg + bigDeadlySpinTime * bigDeadlySpinCoefficient;
}
