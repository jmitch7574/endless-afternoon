#include "arena_manager.h"
#include "custom_draws.h"
#include "entities/enemy.h"
#include "utils.h"
#include <algorithm>
#include <cmath>

void Enemy::DrawEnemyFace()
{
	Vector2 leftEyePos = Vector2Add(position, Vector2(-25, -20));
	Vector2 rightEyePos = Vector2Add(position, Vector2(25, -20));

	leftEyePos = Vector2Add(leftEyePos, eyeOffsets);
	rightEyePos = Vector2Add(rightEyePos, eyeOffsets);

	if (GetState() == EnemyState::Recover) {
		DrawEllipseV(leftEyePos, 15, 5, WHITE);
		DrawEllipseV(rightEyePos, 15, 5, WHITE);
	}
	else
	{
		DrawCircleSector(leftEyePos, 20, 0 + currentEyeRotation, 180 + currentEyeRotation, 1, WHITE);
		DrawCircleSector(rightEyePos, 20, 0 - currentEyeRotation, 180 - currentEyeRotation, 1, WHITE);
	}
}

void Enemy::DrawAttackClockHand(Vector2 clockCenter, float sweepAngle, bool isRightSwing, unsigned char alpha,
								float lengthScale)
{
	const float faceRadius = CELL_SIZE * 1.5f;
	const float innerRadius = faceRadius + CELL_SIZE * 0.05f;
	const float handLength = (isRightSwing ? CELL_SIZE * 2.0f : CELL_SIZE) * lengthScale;
	const float handThickness = isRightSwing ? CELL_SIZE * 0.14f : CELL_SIZE * 0.22f;
	const float fletchLength = (isRightSwing ? CELL_SIZE * 0.24f : CELL_SIZE * 0.28f) * lengthScale;
	const Vector2 sweepDirection = Utils::AngleToVector2(sweepAngle);
	const Vector2 handPivot = Vector2Add(clockCenter, Vector2Scale(sweepDirection, innerRadius));

	CustomDraws::DrawArrow(handPivot, sweepDirection, handLength, handThickness, fletchLength, 32.0f,
						   ClockHandOrange(alpha));
	DrawCircleV(handPivot, handThickness * 0.62f, Color{230, 148, 58, alpha});
}

Color Enemy::ClockHandOrange(unsigned char alpha) { return Color{196, 116, 36, alpha}; }

void Enemy::DrawPunchEffect()
{
	if (punchAnimationTime >= PUNCH_EFFECT_DURATION)
	{
		return;
	}

	const float progress = std::max(std::min(punchAnimationTime / PUNCH_EFFECT_DURATION, 1.0f), 0.0f);
	const float easedProgress = progress * progress * (3.0f - 2.0f * progress);
	const float targetAngle = Utils::Vector2ToAngle(punchDirection);
	const float sweepStart = targetAngle - punchHookSide * (CLOCK_HAND_SWEEP_DEGREES * 0.5f);
	const float sweepAngle = sweepStart + punchHookSide * CLOCK_HAND_SWEEP_DEGREES * easedProgress;
	const float fadeProgress = std::max((progress - 0.72f) / 0.28f, 0.0f);
	const unsigned char alpha = (unsigned char)(255.0f * (1.0f - fadeProgress));

	for (int i = CLOCK_HAND_TRAIL_SAMPLES; i > 0; i--)
	{
		const float trailProgress = std::max(progress - i * CLOCK_HAND_TRAIL_STEP, 0.0f);
		if (trailProgress <= 0.0f)
		{
			continue;
		}

		const float easedTrailProgress = trailProgress * trailProgress * (3.0f - 2.0f * trailProgress);
		const float trailAngle = sweepStart + punchHookSide * CLOCK_HAND_SWEEP_DEGREES * easedTrailProgress;
		const float trailAge = (float)i / (float)(CLOCK_HAND_TRAIL_SAMPLES + 1);
		const float trailStrength = (1.0f - trailAge) * (1.0f - trailAge) * (1.0f - fadeProgress);
		const unsigned char trailAlpha = (unsigned char)(115.0f * trailStrength);

		DrawAttackClockHand(position, trailAngle, currentBasicAttackIsRightSwing, trailAlpha);
	}

	DrawAttackClockHand(position, sweepAngle, currentBasicAttackIsRightSwing, alpha);
}

void Enemy::DrawBasicAttackTelegraph()
{
	const Vector2 targetWorld = ArenaManager::GridPositionToWorld(Vector2{(float)attackTargetX, (float)attackTargetY});
	const Rectangle targetRect = {targetWorld.x - CELL_SIZE / 2.0f, targetWorld.y - CELL_SIZE / 2.0f, CELL_SIZE,
								  CELL_SIZE};
	const float swingSide = currentBasicAttackIsRightSwing ? 1.0f : -1.0f;
	const float targetAngle = Utils::Vector2ToAngle(GetPunchDirectionToTarget());
	const float sweepStart = targetAngle - swingSide * (CLOCK_HAND_SWEEP_DEGREES * 0.5f);
	const float windUpProgress = std::max(std::min(1.0f - (stateTimer / baseAttackWindUpDuration), 1.0f), 0.0f);
	const float lengthScale = 0.2f + 0.8f * windUpProgress;

	DrawRectangleLinesEx(targetRect, 2.0f, ClockHandOrange(255));
	DrawAttackClockHand(position, sweepStart, currentBasicAttackIsRightSwing, 255, lengthScale);
}

void Enemy::DrawSecondaryAttackEffect()
{
	if (currentSecondaryAttack == 2 &&
		(currentState == EnemyState::SecondaryWindUp || currentState == EnemyState::SecondaryAttack))
	{
		DrawSpinningSecondaryAttackEffect();
		return;
	}

	if (currentState != EnemyState::SecondaryAttack)
	{
		return;
	}

	const float progress = std::max(std::min(1.0f - (stateTimer / secondaryAttackDuration), 1.0f), 0.0f);
	const float faceRadius = CELL_SIZE * 1.5f;
	const float pulseRadius = faceRadius + progress * secondaryAttackRange * CELL_SIZE;
	const float ringThickness = CELL_SIZE * 0.08f;
	const float fadeProgress = std::max((progress - 0.72f) / 0.28f, 0.0f);
	const unsigned char alpha = (unsigned char)(255.0f * (1.0f - fadeProgress));

	DrawRing(position, pulseRadius - ringThickness, pulseRadius + ringThickness, 0.0f, 360.0f, SECONDARY_PULSE_SEGMENTS,
			 ClockHandOrange(alpha));
}

void Enemy::DrawSpinningSecondaryAttackEffect()
{
	float angle = spinningSecondarySpinAngle;
	float lengthScale = 1.0f;
	unsigned char alpha = 255;

	if (currentState == EnemyState::SecondaryWindUp)
	{
		const float progress = std::max(std::min(1.0f - (stateTimer / secondaryWindUpDuration), 1.0f), 0.0f);
		lengthScale = 0.25f + 0.75f * progress;
		angle += spinningSecondarySpinDirection * 70.0f * progress;
		alpha = (unsigned char)(160.0f + 95.0f * progress);
	}
	else
	{
		for (int i = 18; i > 0; i--)
		{
			const float trailAge = (float)i / 19.0f;
			const float trailAngle = angle - spinningSecondarySpinDirection * i * 12.0f;
			const float blurStrength = (1.0f - trailAge) * (1.0f - trailAge);
			const unsigned char trailAlpha = (unsigned char)(145.0f * blurStrength);
			DrawAttackClockHand(position, trailAngle, false, trailAlpha);
			DrawAttackClockHand(position, trailAngle + 180.0f, true, trailAlpha);
		}
	}

	DrawAttackClockHand(position, angle, false, alpha, lengthScale);
	DrawAttackClockHand(position, angle + 180.0f, true, alpha, lengthScale);
}

void Enemy::Draw()
{
	Color bodyColor = ORANGE;
	float radius = CELL_SIZE * 1.5f;

	switch (currentState)
	{
	case EnemyState::Idle:
		bodyColor = Color{180, 115, 45, 255};
		break;
	case EnemyState::Advance:
		bodyColor = ORANGE;
		break;
	case EnemyState::WindUp:
		bodyColor = ORANGE;
		break;
	case EnemyState::Attack:
		bodyColor = ORANGE;
		break;
	case EnemyState::Recover:
		bodyColor = Fade(ORANGE, 0.45f);
		break;
	case EnemyState::SecondaryWindUp:
		bodyColor = ORANGE;
		radius += sinf((float)GetTime() * 18.0f) * 7.0f;
		break;
	case EnemyState::SecondaryAttack:
		bodyColor = ORANGE;
		break;
	case EnemyState::SecondaryRecover:
		bodyColor = Fade(ORANGE, 0.45f);
		break;
	case EnemyState::SpecialWindUp:
		bodyColor = ORANGE;
		radius += sinf((float)GetTime() * 14.0f) * 10.0f;
		break;
	case EnemyState::SpecialAttack:
		bodyColor = ORANGE;
		break;
	case EnemyState::SpecialRecover:
		bodyColor = Fade(ORANGE, 0.45f);
		break;
	}

	DrawCircleV(position, radius + CELL_SIZE * 0.13f, WHITE);
	DrawCircleV(position, radius, bodyColor);
	DrawEnemyFace();

	if (currentState == EnemyState::WindUp)
	{
		DrawBasicAttackTelegraph();
	}

	DrawPunchEffect();
	DrawSecondaryAttackEffect();
}

Rectangle Enemy::GetBBoxWorld()
{
	return Rectangle(position.x - CELL_SIZE * 1.5f, position.y - CELL_SIZE * 1.5f, CELL_SIZE * 3, CELL_SIZE * 3);
}
