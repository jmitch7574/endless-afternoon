#include "custom_draws.h"
#include "entities/clockhand.h"
#include "utils.h"
#include <cmath>

ClockHand::ClockHand(raylib::Vector2 pivot, float angleDeg, float length, float thickness, Color color)
	: Entity(pivot), angleDeg(angleDeg), length(length), thickness(thickness), color(color)
{
	targetAngleDeg = angleDeg;
	advanceStartAngleDeg = angleDeg;
	advanceTargetAngleDeg = angleDeg;
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
		const float progress = fminf(advanceTime / advanceDuration, 1.0f);
		const float easedProgress = progress * progress * (3.0f - 2.0f * progress);
		angleDeg = advanceStartAngleDeg + (advanceTargetAngleDeg - advanceStartAngleDeg) * easedProgress;

		if (progress >= 1.0f)
		{
			angleDeg = advanceTargetAngleDeg;
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
	targetAngleDeg += 90.0f;
	advanceStartAngleDeg = angleDeg;
	advanceTargetAngleDeg = targetAngleDeg;
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

float ClockHand::GetAngle() { return fmod(angleDeg + bigDeadlySpinTime * bigDeadlySpinCoefficient, 360.0f); }
