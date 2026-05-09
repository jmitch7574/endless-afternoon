#include "entities/enemy.h"
#include "raymath.h"
#include "scene.h"
#include <algorithm>
#include <cmath>

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

float AngleDifference(float a, float b)
{
	const float diff = fabsf(NormalizeAngle(a - b));
	return diff > 180.0f ? 360.0f - diff : diff;
}
} // namespace

void Enemy::TriggerPunchEffect()
{
	punchAnimationTime = 0.0f;
	punchDirection = GetPunchDirectionToTarget();
	punchHookSide = currentBasicAttackIsRightSwing ? 1.0f : -1.0f;
}

void Enemy::UpdatePunchEffect(float deltaTime) { punchAnimationTime += deltaTime; }

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

	const int currentPlayerDistance = DistanceFromBossFootprint(gridPosition, targetGridPosition);
	const bool hit = currentPlayerDistance > 0 && currentPlayerDistance <= GetCurrentBasicAttackRange();

	if (hit && playScene != nullptr)
	{
		playScene->player.Hurt(BASIC_ATTACK_DAMAGE, D_Enemy);
	}

	normalAttackCount++;
	nextBasicAttackIsRightSwing = !currentBasicAttackIsRightSwing;

	if (normalAttackCount >= normalAttacksPerCycle)
	{
		CompletePrimaryAttackCycle();
		return;
	}

	EnterRecover(ATTACK_RECOVER_DURATION);
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
	spinningSecondarySpinAngle =
		NormalizeAngle(spinningSecondarySpinAngle + spinningSecondarySpinDirection * spinningSecondarySpinSpeed * deltaTime);
	spinningSecondaryDamageCooldown -= deltaTime;

	if (currentMoveCooldown <= 0.0f)
	{
		TryMoveTowardTarget();
	}

	const int playerDistance = DistanceFromBossFootprint(gridPosition, targetGridPosition);
	if (playerDistance > 0 && playerDistance <= SPINNING_SECONDARY_RANGE && spinningSecondaryDamageCooldown <= 0.0f)
	{
		const Vector2 playerOffset = Vector2Subtract(targetGridPosition, gridPosition);
		const float playerAngle = atan2f(playerOffset.y, playerOffset.x) * RAD2DEG;
		const bool hitByHourHand = AngleDifference(playerAngle, spinningSecondarySpinAngle) <= 28.0f;
		const bool hitByMinuteHand = AngleDifference(playerAngle, spinningSecondarySpinAngle + 180.0f) <= 28.0f;

		if ((hitByHourHand || hitByMinuteHand) && playScene != nullptr)
		{
			playScene->player.Hurt(SPINNING_SECONDARY_DAMAGE, D_Enemy);
			spinningSecondaryDamageCooldown = spinningSecondaryDamageInterval;
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
		nextSpecialAttack = 2;
		break;
	default:
		SpecialAttack1();
		nextSpecialAttack = 2;
		break;
	}
}

void Enemy::SpecialAttack1() {}

void Enemy::SpecialAttack2()
{
	if (playScene == nullptr)
	{
		return;
	}

	playScene->minuteHand.BeginBigDeadlySpin();
	playScene->hourHand.BeginBigDeadlySpin();
}

void Enemy::SpecialAttack3()
{
	if (playScene == nullptr)
	{
		return;
	}

	playScene->StartRedLightGreenLight();
}
