#include "arena_manager.h"
#include "custom_draws.h"
#include "entities/enemy.h"
#include "raymath.h"
#include "utils.h"
#include <algorithm>
#include <cmath>

namespace
{
constexpr float HANDSTACHE_THICKNESS = 8.0f;
constexpr float HANDSTACHE_FLETCH_LENGTH = 15.0f;
constexpr float HANDSTACHE_FLETCH_ANGLE = 15.0f;
constexpr float HANDSTACHE_CENTER_DOT_RADIUS = 5.5f;
constexpr float DETACHED_HAND_RADIUS = CELL_SIZE * 0.24f;
constexpr float HANDSTACHE_GRAB_PROGRESS = 0.48f;
constexpr float HELD_HAND_RADIUS_FROM_BODY = CELL_SIZE * 1.88f;
constexpr float PULLBACK_HOLD_PROGRESS = 0.25f;

float Clamp01(float value)
{
	return std::max(std::min(value, 1.0f), 0.0f);
}

float SmoothStep(float value)
{
	value = Clamp01(value);
	return value * value * (3.0f - 2.0f * value);
}

float LerpFloat(float start, float end, float amount) { return start + (end - start) * amount; }

float ShortestAngleDelta(float start, float end)
{
	float delta = Utils::NormalizeAngle(end - start);
	if (delta > 180.0f)
	{
		delta -= 360.0f;
	}

	return delta;
}

float LerpAngleShortestPath(float start, float end, float amount)
{
	return start + ShortestAngleDelta(start, end) * amount;
}

Vector2 PointOnOrbit(Vector2 center, float angle, float radius)
{
	return Vector2Add(center, Vector2Scale(Utils::AngleToVector2(angle), radius));
}

Vector2 LerpAroundBody(Vector2 center, float startAngle, float endAngle, float startRadius, float endRadius,
					   float amount)
{
	const float angle = LerpAngleShortestPath(startAngle, endAngle, amount);
	const float radius = LerpFloat(startRadius, endRadius, amount);

	return PointOnOrbit(center, angle, radius);
}
} // namespace

void Enemy::DrawEnemyFace()
{
	float radius = CELL_SIZE * 1.5f  * scale;

	// DRAW MARKINGS
	for (int i = 0; i < 24; i++)
	{
		float angleDeg = clockMarkingValue + (i * 15);

		float innerRadius = (i % 2 == 0)
                ? radius - 15
                : radius - 10;

		Vector2 startOffset = Vector2Scale(Utils::AngleToVector2(angleDeg), innerRadius);
		Vector2 endOffset = Vector2Scale(Utils::AngleToVector2(angleDeg), radius - 5);

		DrawLineEx(
			Vector2Add(position, startOffset),
			Vector2Add(position, endOffset),
			(i % 2 == 0) ? 4.0f : 2.0f,
			ClockBrown()
		);
	}

	// DRAW EYES
	Vector2 leftEyePos = Vector2Add(eyeOffsets, Vector2(-25, -20));
	Vector2 rightEyePos = Vector2Add(eyeOffsets, Vector2(25, -20));

	
	leftEyePos = Vector2Scale(leftEyePos, scale);
	rightEyePos = Vector2Scale(rightEyePos, scale);
	
	leftEyePos = Vector2Add(leftEyePos, position);
	rightEyePos = Vector2Add(rightEyePos, position);


	if (GetState() == EnemyState::Recover) {
		DrawEllipseV(leftEyePos, 15 * scale, 5 * scale, ClockHands());
		DrawEllipseV(rightEyePos, 15 * scale, 5 * scale, ClockHands());
	}
	else
	{
		DrawCircleSector(leftEyePos, 20 * scale, 0 + currentEyeRotation, 180 + currentEyeRotation, 1, ClockHands());
		DrawCircleSector(rightEyePos, 20 * scale, 0 - currentEyeRotation, 180 - currentEyeRotation, 1, ClockHands());
	}
	
	// DRAW HANDSTACHE
	const Vector2 moustacheBase = GetHandstacheBase();
	if (!ShouldHideHandstache(false))
	{
		CustomDraws::DrawArrow(moustacheBase, GetHandstacheAngle(false), GetHandstacheLength(false) * scale,
							   HANDSTACHE_THICKNESS * scale, HANDSTACHE_FLETCH_LENGTH * scale, HANDSTACHE_FLETCH_ANGLE, ClockHands());
	}

	if (!ShouldHideHandstache(true))
	{
		CustomDraws::DrawArrow(moustacheBase, GetHandstacheAngle(true), GetHandstacheLength(true) * scale,
							   HANDSTACHE_THICKNESS * scale, HANDSTACHE_FLETCH_LENGTH * scale, HANDSTACHE_FLETCH_ANGLE, ClockHands());
	}

	DrawCircleV(moustacheBase, HANDSTACHE_CENTER_DOT_RADIUS, ClockHands());
}

Color Enemy::ClockHandOrange(unsigned char alpha) { return Color{196, 116, 36, alpha}; }

Vector2 Enemy::GetHandstacheBase() const { return Vector2Add(position, Vector2{0.0f, 20.0f}); }

float Enemy::GetHandstacheLength(bool isRightHand) const
{
	return CELL_SIZE * (isRightHand ? 1.5f : 2.0f);
}

float Enemy::GetHandstacheAngle(bool isRightHand) const
{
	return (isRightHand ? 90.0f - currentMoustacheGap : 90.0f + currentMoustacheGap) + currentMoustacheOffset;
}

Vector2 Enemy::GetHandstacheTip(bool isRightHand) const
{
	return Vector2Add(GetHandstacheBase(),
					  Vector2Scale(Utils::AngleToVector2(GetHandstacheAngle(isRightHand)),
								   GetHandstacheLength(isRightHand)));
}

bool Enemy::ShouldHideHandstache(bool isRightHand) const
{
	const float primaryWindUpProgress =
		baseAttackWindUpDuration > 0.0f ? Clamp01(1.0f - stateTimer / baseAttackWindUpDuration) : 1.0f;
	const bool primaryGrabbed =
		currentState == EnemyState::WindUp && primaryWindUpProgress >= HANDSTACHE_GRAB_PROGRESS &&
		currentBasicAttackIsRightSwing != isRightHand;
	const bool primarySwinging =
		punchAnimationTime < PUNCH_EFFECT_DURATION && currentBasicAttackIsRightSwing != isRightHand;

	const float spinnerWindUpProgress =
		secondaryWindUpDuration > 0.0f ? Clamp01(1.0f - stateTimer / secondaryWindUpDuration) : 1.0f;
	const bool spinningGrabbed = currentSecondaryAttack == 2 && currentState == EnemyState::SecondaryWindUp &&
								 spinnerWindUpProgress >= HANDSTACHE_GRAB_PROGRESS;
	const bool spinningActive = currentSecondaryAttack == 2 && currentState == EnemyState::SecondaryAttack;

	return primaryGrabbed || primarySwinging || spinningGrabbed || spinningActive;
}

Vector2 Enemy::GetReachAroundHandPoint(bool isRightHand, float progress) const
{
	const float side = isRightHand ? 1.0f : -1.0f;
	const Vector2 start = Vector2Add(position, Vector2{side * CELL_SIZE * 1.45f, CELL_SIZE * 0.08f});
	const Vector2 controlOne = Vector2Add(position, Vector2{side * CELL_SIZE * 2.25f, CELL_SIZE * 0.18f});
	const Vector2 controlTwo =
		Vector2Add(GetHandstacheTip(isRightHand), Vector2{side * CELL_SIZE * 0.35f, CELL_SIZE * 0.2f});

	return Utils::BezierLerp(start, GetHandstacheTip(isRightHand), controlOne, controlTwo, SmoothStep(progress));
}

Vector2 Enemy::GetAttackHandPointAtRange(float sweepAngle, float rangeCells, float lengthScale) const
{
	const float faceRadius = CELL_SIZE * 1.5f;
	const float innerRadius = faceRadius + CELL_SIZE * 0.05f;
	const float reach = innerRadius + rangeCells * CELL_SIZE * lengthScale;
	return Vector2Add(position, Vector2Scale(Utils::AngleToVector2(sweepAngle), reach));
}

Vector2 Enemy::GetAttackHandPoint(float sweepAngle, bool isRightSwing, float lengthScale) const
{
	const float rangeCells = isRightSwing ? (float)minuteHandAttackRange : (float)hourHandAttackRange;
	return GetAttackHandPointAtRange(sweepAngle, rangeCells, lengthScale);
}

float Enemy::GetPrimarySwingAngle(float progress) const
{
	const float easedProgress = SmoothStep(progress);

	return punchStartAngle + punchHookSide * punchSweepDegrees * easedProgress;
}

float Enemy::GetPrimarySwingStartAngle() const
{
	return punchStartAngle;
}

Vector2 Enemy::GetHeldHandBase(float heldAngle) const
{
	return Vector2Add(position, Vector2Scale(Utils::AngleToVector2(heldAngle), HELD_HAND_RADIUS_FROM_BODY));
}

Vector2 Enemy::GetHeldHandTip(float heldAngle, bool isRightHand) const
{
	return Vector2Add(GetHeldHandBase(heldAngle),
					  Vector2Scale(Utils::AngleToVector2(heldAngle), GetHandstacheLength(isRightHand)));
}

Vector2 Enemy::GetPrimarySwingHandBase(float progress) const
{
	return GetHeldHandBase(GetPrimarySwingAngle(progress));
}

Vector2 Enemy::GetPrimarySwingHandTip(float progress) const
{
	return GetHeldHandTip(GetPrimarySwingAngle(progress), !currentBasicAttackIsRightSwing);
}

void Enemy::DrawDetachedHand(Vector2 handPosition, unsigned char alpha, float scale, bool holdingHandstache,
							 float heldAngle, float heldHandstacheLength)
{
	const float radius = DETACHED_HAND_RADIUS * scale;

	if (holdingHandstache)
	{
		CustomDraws::DrawArrow(handPosition, heldAngle, heldHandstacheLength, HANDSTACHE_THICKNESS,
							   HANDSTACHE_FLETCH_LENGTH, HANDSTACHE_FLETCH_ANGLE, Color{255, 255, 255, alpha});
	}

	DrawCircleV(handPosition, radius * 1.18f, Color{255, 238, 212, (unsigned char)(alpha * 0.82f)});
	DrawCircleV(handPosition, radius, Color{196, 116, 36, alpha});
}

void Enemy::DrawMovementTrail()
{
	for (int i = MOVE_TRAIL_SAMPLES - 1; i > 0; i--)
	{
		if (moveTrail[i].opacity <= 0)
		{
			continue;
		}

		DrawCircleV(moveTrail[i].pos, CELL_SIZE * 1.5f, Color{196, 116, 36, (unsigned char)moveTrail[i].opacity});
	}
}

void Enemy::DrawPunchEffect()
{
	if (punchAnimationTime >= PUNCH_EFFECT_DURATION)
	{
		return;
	}

	const float progress = std::max(std::min(punchAnimationTime / PUNCH_EFFECT_DURATION, 1.0f), 0.0f);
	const float fadeProgress = std::max((progress - 0.72f) / 0.28f, 0.0f);
	const unsigned char alpha = (unsigned char)(255.0f * (1.0f - fadeProgress));
	const bool handstacheIsRight = !currentBasicAttackIsRightSwing;
	const float handstacheLength = GetHandstacheLength(handstacheIsRight);

	for (int i = CLOCK_HAND_TRAIL_SAMPLES; i > 0; i--)
	{
		const float trailProgress = std::max(progress - i * CLOCK_HAND_TRAIL_STEP, 0.0f);
		if (trailProgress <= 0.0f)
		{
			continue;
		}

		const float trailAge = (float)i / (float)(CLOCK_HAND_TRAIL_SAMPLES + 1);
		const float trailStrength = (1.0f - trailAge) * (1.0f - trailAge) * (1.0f - fadeProgress);
		const unsigned char trailAlpha = (unsigned char)(115.0f * trailStrength);
		const float trailAngle = GetPrimarySwingAngle(trailProgress);

		DrawDetachedHand(GetPrimarySwingHandBase(trailProgress), trailAlpha, 1.0f, true, trailAngle, handstacheLength);
	}

	DrawDetachedHand(GetPrimarySwingHandBase(progress), alpha, 1.0f, true, GetPrimarySwingAngle(progress),
					 handstacheLength);
}

void Enemy::DrawBasicAttackTelegraph()
{
	const float sweepStart = GetPrimarySwingStartAngle();
	const bool handstacheIsRight = !currentBasicAttackIsRightSwing;
	const float windUpProgress = Clamp01(1.0f - (stateTimer / baseAttackWindUpDuration));
	const float reachProgress = Clamp01(windUpProgress / HANDSTACHE_GRAB_PROGRESS);
	const float grabProgress = Clamp01((windUpProgress - HANDSTACHE_GRAB_PROGRESS) / (1.0f - HANDSTACHE_GRAB_PROGRESS));
	const float pullbackProgress = SmoothStep((grabProgress - PULLBACK_HOLD_PROGRESS) / (1.0f - PULLBACK_HOLD_PROGRESS));
	const Vector2 grabPoint = GetHandstacheTip(handstacheIsRight);
	const float grabAngle = GetHandstacheAngle(handstacheIsRight);
	const float handstacheLength = GetHandstacheLength(handstacheIsRight);
	const float grabRadius = Vector2Distance(position, grabPoint);
	Vector2 handPosition = GetReachAroundHandPoint(handstacheIsRight, reachProgress);
	bool holdingHandstache = false;
	float heldAngle = grabAngle;

	if (windUpProgress >= HANDSTACHE_GRAB_PROGRESS)
	{
		handPosition = LerpAroundBody(position, grabAngle, sweepStart, grabRadius, HELD_HAND_RADIUS_FROM_BODY,
									  pullbackProgress);
		holdingHandstache = true;
		heldAngle = LerpAngleShortestPath(grabAngle, sweepStart, pullbackProgress);
	}

	for (int i = 4; i > 0; i--)
	{
		const unsigned char trailAlpha = (unsigned char)(85.0f * (1.0f - (float)i / 5.0f));
		const float trailProgress = std::max(windUpProgress - (float)i * 0.09f, 0.0f);
		const bool trailIsHolding = trailProgress >= HANDSTACHE_GRAB_PROGRESS;
		Vector2 trailPosition = GetReachAroundHandPoint(handstacheIsRight,
														 Clamp01(trailProgress / HANDSTACHE_GRAB_PROGRESS));
		float trailHeldAngle = grabAngle;

		if (trailIsHolding)
		{
			const float trailGrabProgress =
				Clamp01((trailProgress - HANDSTACHE_GRAB_PROGRESS) / (1.0f - HANDSTACHE_GRAB_PROGRESS));
			const float trailPullbackProgress =
				SmoothStep((trailGrabProgress - PULLBACK_HOLD_PROGRESS) / (1.0f - PULLBACK_HOLD_PROGRESS));
			trailPosition =
				LerpAroundBody(position, grabAngle, sweepStart, grabRadius, HELD_HAND_RADIUS_FROM_BODY, trailPullbackProgress);
			trailHeldAngle = LerpAngleShortestPath(grabAngle, sweepStart, trailPullbackProgress);
		}

		DrawDetachedHand(trailPosition, trailAlpha, 0.92f, trailIsHolding, trailHeldAngle, handstacheLength);
	}

	DrawDetachedHand(handPosition, 255, 1.0f, holdingHandstache, heldAngle, handstacheLength);
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
		const float progress = Clamp01(1.0f - (stateTimer / secondaryWindUpDuration));
		const float reachProgress = Clamp01(progress / HANDSTACHE_GRAB_PROGRESS);
		const float grabProgress = SmoothStep((progress - HANDSTACHE_GRAB_PROGRESS) / (1.0f - HANDSTACHE_GRAB_PROGRESS));
		angle += spinningSecondarySpinDirection * SPINNING_SECONDARY_WINDUP_ROTATION_DEGREES * progress;
		alpha = (unsigned char)(160.0f + 95.0f * progress);
		const bool holdingHandstache = progress >= HANDSTACHE_GRAB_PROGRESS;
		const Vector2 leftReachPoint = GetReachAroundHandPoint(false, reachProgress);
		const Vector2 rightReachPoint = GetReachAroundHandPoint(true, reachProgress);
		const float leftGrabAngle = GetHandstacheAngle(false);
		const float rightGrabAngle = GetHandstacheAngle(true);
		const float leftGrabRadius = Vector2Distance(position, GetHandstacheTip(false));
		const float rightGrabRadius = Vector2Distance(position, GetHandstacheTip(true));
		const Vector2 leftPosition =
			holdingHandstache
				? LerpAroundBody(position, leftGrabAngle, angle, leftGrabRadius, HELD_HAND_RADIUS_FROM_BODY, grabProgress)
				: leftReachPoint;
		const Vector2 rightPosition =
			holdingHandstache ? LerpAroundBody(position, rightGrabAngle, angle + 180.0f, rightGrabRadius,
											   HELD_HAND_RADIUS_FROM_BODY, grabProgress)
							  : rightReachPoint;
		const float leftHeldAngle =
			holdingHandstache ? LerpAngleShortestPath(leftGrabAngle, angle, grabProgress) : leftGrabAngle;
		const float rightHeldAngle =
			holdingHandstache ? LerpAngleShortestPath(rightGrabAngle, angle + 180.0f, grabProgress) : rightGrabAngle;

		DrawDetachedHand(leftPosition, alpha, 1.0f, holdingHandstache, leftHeldAngle, GetHandstacheLength(false));
		DrawDetachedHand(rightPosition, alpha, 1.0f, holdingHandstache, rightHeldAngle, GetHandstacheLength(true));
		return;
	}
	else
	{
		for (int i = 18; i > 0; i--)
		{
			const float trailAge = (float)i / 19.0f;
			const float trailAngle = angle - spinningSecondarySpinDirection * i * 12.0f;
			const float blurStrength = (1.0f - trailAge) * (1.0f - trailAge);
			const unsigned char trailAlpha = (unsigned char)(145.0f * blurStrength);
			DrawDetachedHand(GetHeldHandBase(trailAngle), trailAlpha, 1.0f, true, trailAngle, GetHandstacheLength(false));
			DrawDetachedHand(GetHeldHandBase(trailAngle + 180.0f), trailAlpha, 1.0f, true, trailAngle + 180.0f,
							 GetHandstacheLength(true));
		}
	}

	DrawDetachedHand(GetHeldHandBase(angle), alpha, lengthScale, true, angle, GetHandstacheLength(false));
	DrawDetachedHand(GetHeldHandBase(angle + 180.0f), alpha, lengthScale, true, angle + 180.0f,
					 GetHandstacheLength(true));
}

void Enemy::Draw()
{
	Color bodyColor = ClockOrange();
	Color radiusColor = ClockWhite();
	float radius = CELL_SIZE * 1.5f;

	switch (currentState)
	{
	case EnemyState::Idle:
		bodyColor = Color{180, 115, 45, 255};
		break;
	case EnemyState::Advance:
		bodyColor = ClockOrange();
		break;
	case EnemyState::WindUp:
		bodyColor = ClockOrange();
		break;
	case EnemyState::Attack:
		bodyColor = ClockOrange();
		break;
	case EnemyState::Recover:
		bodyColor = ClockRest();
		break;
	case EnemyState::SecondaryWindUp:
		bodyColor = ClockOrange();
		radius += sinf((float)GetTime() * 18.0f) * 7.0f;
		break;
	case EnemyState::SecondaryAttack:
		bodyColor = ClockOrange();
		break;
	case EnemyState::SecondaryRecover:
		bodyColor = ClockRest();
		break;
	case EnemyState::SpecialWindUp:
		bodyColor = ClockOrange();
		radius += sinf((float)GetTime() * 14.0f) * 10.0f;
		break;
	case EnemyState::SpecialAttack:
		bodyColor = ClockOrange();
		break;
	case EnemyState::SpecialRecover:
		bodyColor = ClockRest();
		break;
	}

	if (timeSinceLastHit < 1)
	{
		if (fmodf(timeSinceLastHit  * 8, 2) > 1)
		{
			Color a = radiusColor;
			radiusColor = bodyColor;
			bodyColor = a;
		}
	}

	if (worldSpace == WorldSpace::Floaty) bodyColor = ClockOrange();
	DrawMovementTrail();

	DrawCircleV(position, (radius + CELL_SIZE * 0.13f) * scale, radiusColor);
	DrawCircleV(position, radius * scale, bodyColor);
	DrawEnemyFace();

	
	if (worldSpace == WorldSpace::Floaty) return;

	if (currentState == EnemyState::WindUp)
	{
		DrawBasicAttackTelegraph();
	}

	DrawPunchEffect();
	DrawSecondaryAttackEffect();
}

Rectangle Enemy::GetBBoxWorld()
{
	if (worldSpace == WorldSpace::Floaty) return Rectangle(0, 0, 0, 0);
	return Rectangle(position.x - CELL_SIZE * 1.5f, position.y - CELL_SIZE * 1.5f, CELL_SIZE * 3, CELL_SIZE * 3);
}
