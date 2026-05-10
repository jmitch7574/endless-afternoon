#include "arena_manager.h"
#include "custom_draws.h"
#include "entities/clockhand.h"
#include "utils.h"
#include <cmath>

namespace
{
constexpr float BIG_DEADLY_SPIN_SPEED_DEGREES_PER_SECOND = 40.0f;
}

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
		const float rotationThisFrame =
			fminf(bigDeadlySpinDegreesRemaining, BIG_DEADLY_SPIN_SPEED_DEGREES_PER_SECOND * GetFrameTime());
		angleDeg = Utils::NormalizeAngle(angleDeg + rotationThisFrame * (float)bigDeadlySpinDirection);
		bigDeadlySpinDegreesRemaining -= rotationThisFrame;

		if (bigDeadlySpinDegreesRemaining <= 0.0f)
		{
			targetAngleDeg = angleDeg;
			inBigDeadlySpin = false;
			bigDeadlySpinDegreesRemaining = 0.0f;
			StartQueuedAdvanceIfReady();
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
			angleDeg = Utils::NormalizeAngle(advanceTargetAngleDeg);
			targetAngleDeg = angleDeg;
			isAdvancing = false;
			StartQueuedAdvanceIfReady();
		}
	}
}

void ClockHand::Draw()
{
	const float drawLength = fmaxf(length, ArenaManager::DistanceToOctagonEdge(GetAngle()));
	const float fletchLength = fmaxf(35.0f, thickness * 3.0f);
	CustomDraws::DrawArrow(position, GetAngle(), drawLength, thickness, fletchLength, 35, (activated || isAdvancing || inBigDeadlySpin) ? color : Color(80, 80, 80, 255));
}

void ClockHand::Advance()
{
	if (IsMoving())
	{
		queuedAdvances++;
		return;
	}

	StartAdvance();
}

void ClockHand::StartAdvance()
{
	targetAngleDeg = angleDeg + 90.0f;
	advanceStartAngleDeg = angleDeg;
	advanceTargetAngleDeg = targetAngleDeg;
	isAdvancing = true;
	advanceTime = 0;
}

void ClockHand::StartQueuedAdvanceIfReady()
{
	if (IsMoving() || queuedAdvances <= 0)
	{
		return;
	}

	queuedAdvances--;
	StartAdvance();
}

void ClockHand::CompleteAdvanceInstantly()
{
	if (!isAdvancing)
	{
		return;
	}

	angleDeg = Utils::NormalizeAngle(advanceTargetAngleDeg);
	targetAngleDeg = angleDeg;
	isAdvancing = false;
	advanceTime = 0.0f;
}

void ClockHand::BeginBigDeadlySpin(int spinDirection, float spinDegrees)
{
	const int direction = spinDirection == 0 ? (GetRandomValue(0, 99) < 50 ? 1 : -1) : (spinDirection < 0 ? -1 : 1);
	const float totalSpinDegrees = fmaxf(fabsf(spinDegrees), 1.0f);

	CompleteAdvanceInstantly();
	while (queuedAdvances > 0)
	{
		angleDeg = Utils::NormalizeAngle(angleDeg + 90.0f);
		targetAngleDeg = angleDeg;
		queuedAdvances--;
	}

	bigDeadlySpinDegreesRemaining = totalSpinDegrees;
	bigDeadlySpinDirection = direction;
	inBigDeadlySpin = true;
}

Vector2 ClockHand::GetLargeExtendedPoint()
{
	return Vector2Add(position, Vector2Scale(Utils::AngleToVector2(GetAngle()), 10000));
}

bool ClockHand::IsMoving() const { return isAdvancing || inBigDeadlySpin; }

float ClockHand::GetAngle() { return Utils::NormalizeAngle(angleDeg); }
