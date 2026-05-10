#include "arena_manager.h"
#include "entities/enemy.h"
#include "raymath.h"
#include "scene.h"
#include "utils.h"
#include <algorithm>
#include <cmath>
#include <resource_loader.h>

namespace
{
float NormalizeAngle(float angle)
{
	angle = fmodf(angle, 360.0f);
	if (angle < 0.0f)
	{
		angle += 360.0f;
	}

	return angle;
}

} // namespace

void Enemy::TriggerPunchEffect()
{
	punchAnimationTime = 0.0f;
	punchEffectHasHit = false;
	PlaySound(Resources::GetWhoosh());
}

void Enemy::UpdatePunchEffect(float deltaTime)
{
	const float previousPunchAnimationTime = punchAnimationTime;
	punchAnimationTime += deltaTime;

	if (punchEffectHasHit || playScene == nullptr || previousPunchAnimationTime >= PUNCH_EFFECT_DURATION)
	{
		return;
	}

	const float previousProgress = std::max(std::min(previousPunchAnimationTime / PUNCH_EFFECT_DURATION, 1.0f), 0.0f);
	const float currentProgress = std::max(std::min(punchAnimationTime / PUNCH_EFFECT_DURATION, 1.0f), 0.0f);
	const int sampleCount = 5;

	for (int i = 0; i <= sampleCount; i++)
	{
		const float t = (float)i / (float)sampleCount;
		const float sampleProgress = previousProgress + (currentProgress - previousProgress) * t;
		if (PunchEffectHitsPlayerAtProgress(sampleProgress))
		{
			playScene->player.Hurt(BASIC_ATTACK_DAMAGE, D_Enemy);
			punchEffectHasHit = true;
			return;
		}
	}
}

bool Enemy::PunchEffectHitsPlayerAtProgress(float progress) const
{
	if (playScene == nullptr)
	{
		return false;
	}

	const Vector2 handBase = GetPrimarySwingHandBase(progress);
	const Vector2 handTip = GetPrimarySwingHandTip(progress);
	const float playerRadius = CELL_SIZE * 0.45f;

	return Utils::LineIntersectsCircle(handBase, handTip, playScene->player.GetPosition(), playerRadius);
}

void Enemy::TryPrimaryAttack()
{
	if (currentState != EnemyState::Advance || currentPrimaryAttackCooldown > 0.0f)
	{
		return;
	}

	const int currentPlayerDistance = DistanceFromBossFootprint(gridPosition, targetGridPosition);
	if (currentPlayerDistance <= 0 || currentPlayerDistance > GetNextBasicAttackRange())
	{
		return;
	}

	currentBasicAttackIsRightSwing = nextBasicAttackIsRightSwing;
	EnterState(EnemyState::WindUp);
}

void Enemy::PrimaryAttack()
{
	TriggerPunchEffect();
	currentPrimaryAttackCooldown = primaryAttackCooldown;
	primaryAttackMovementLockTimer = primaryAttackMovementLockDuration;

	normalAttackCount++;
	nextBasicAttackIsRightSwing = !currentBasicAttackIsRightSwing;

	EnterRecover(normalAttackCount >= normalAttacksPerCycle ? CYCLE_RECOVER_DURATION : ATTACK_RECOVER_DURATION);
}

void Enemy::CompletePrimaryAttackCycle()
{
	normalAttackCount = 0;
	primaryCyclesCompleted++;

	if (primaryCyclesCompleted >= primaryCyclesBeforeSpecial)
	{
		primaryCyclesCompleted = 0;
		EnterState(EnemyState::SpecialWindUp);
	}
	else
	{
		EnterState(EnemyState::SecondaryWindUp);
	}
}

void Enemy::SecondaryAttack(float deltaTime)
{
	if (currentSecondaryAttack == 2)
	{
		SpinningTopSecondaryAttack(deltaTime);
	}
	else
	{
		PulseSecondaryAttack(deltaTime);
	}
}

void Enemy::PulseSecondaryAttack(float deltaTime)
{
	stateTimer -= deltaTime;

	const float progress = std::max(std::min(1.0f - (stateTimer / secondaryAttackDuration), 1.0f), 0.0f);
	const float currentPulseRadius = progress * secondaryAttackRange;
	const int playerDistance = DistanceFromBossFootprint(gridPosition, targetGridPosition);

	if (!secondaryAttackHasHit && playerDistance > 0 && playerDistance <= secondaryAttackRange &&
		(float)playerDistance > secondaryPulsePreviousRadius && (float)playerDistance <= currentPulseRadius)
	{
		secondaryAttackHasHit = true;

		if (playScene != nullptr)
		{
			playScene->player.Hurt(SECONDARY_ATTACK_DAMAGE, D_Enemy);
		}
	}

	secondaryPulsePreviousRadius = currentPulseRadius;

	if (stateTimer <= 0)
	{
		FinishSecondaryAttack();
	}
}

void Enemy::SpinningTopSecondaryAttack(float deltaTime)
{
	stateTimer -= deltaTime;
	const float previousSpinAngle = spinningSecondarySpinAngle;
	spinningSecondarySpinAngle =
		NormalizeAngle(spinningSecondarySpinAngle + spinningSecondarySpinDirection * spinningSecondarySpinSpeed * deltaTime);
	spinningSecondaryDamageCooldown -= deltaTime;

	if (currentMoveCooldown <= 0.0f)
	{
		TryMoveTowardTarget();
	}

	if (playScene != nullptr && spinningSecondaryDamageCooldown <= 0.0f)
	{
		constexpr int sampleCount = 6;
		const Vector2 playerPosition = playScene->player.GetPosition();
		const float playerRadius = CELL_SIZE * 0.45f;

		for (int i = 0; i <= sampleCount; i++)
		{
			const float t = (float)i / (float)sampleCount;
			const float sampleAngle = previousSpinAngle + spinningSecondarySpinDirection * spinningSecondarySpinSpeed * deltaTime * t;
			const bool hitByLeftHand =
				Utils::LineIntersectsCircle(GetHeldHandBase(sampleAngle), GetHeldHandTip(sampleAngle, false),
											playerPosition, playerRadius);
			const bool hitByRightHand =
				Utils::LineIntersectsCircle(GetHeldHandBase(sampleAngle + 180.0f), GetHeldHandTip(sampleAngle + 180.0f, true),
											playerPosition, playerRadius);

			if (hitByLeftHand || hitByRightHand)
			{
				playScene->player.Hurt(SPINNING_SECONDARY_DAMAGE, D_Enemy);
				spinningSecondaryDamageCooldown = spinningSecondaryDamageInterval;
				break;
			}
		}	
	}

	if (stateTimer <= 0)
	{
		FinishSecondaryAttack();
	}
}

void Enemy::FinishSecondaryAttack()
{
	nextSecondaryAttack = currentSecondaryAttack == 1 ? 2 : 1;
	EnterState(EnemyState::SecondaryRecover);
}

void Enemy::RunSelectedSpecialAttack()
{
	switch (currentSpecialAttack)
	{
	case 2:
		SpecialAttack2();
		nextSpecialAttack = 3;
		break;
	case 3:
		SpecialAttack3();
		nextSpecialAttack = 4;
		break;
	case 4:
		SpecialAttack4();
		nextSpecialAttack = 2;
		break;
	default:
		SpecialAttack1();
		nextSpecialAttack = 2;
		break;
	}
}

void Enemy::SpecialAttack1() 
{
	
}

void Enemy::SpecialAttack2()
{
	if (playScene == nullptr)
	{
		return;
	}

	int spinDirection = queuedClockHandSpinDirection;
	if (spinDirection == 0)
	{
		spinDirection = GetRandomValue(0, 99) < 50 ? 1 : -1;
		queuedClockHandSpinDirection = -spinDirection;
	}
	else
	{
		queuedClockHandSpinDirection = 0;
	}

	const float spinDegrees = 360.0f + 90.0f * (float)GetRandomValue(0, 3);
	playScene->minuteHand.BeginBigDeadlySpin(spinDirection, spinDegrees);
	playScene->hourHand.BeginBigDeadlySpin(spinDirection, spinDegrees);
	
	specialStateTimeLeft = 9;
	specialStateTimeIn = 0;
}

void Enemy::SpecialAttack3()
{
	if (playScene == nullptr)
	{
		return;
	}

	playScene->StartRedLightGreenLight();
	specialStateTimeLeft = 12;
	specialStateTimeIn = 0;
}

void Enemy::SpecialAttack4()
{
	if (playScene == nullptr)
	{
		return;
	}

	playScene->StartRomanNumeralAttack();
}
