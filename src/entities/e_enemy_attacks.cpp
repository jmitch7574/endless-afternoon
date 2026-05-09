#include "entities/enemy.h"
#include "raymath.h"
#include "scene.h"
#include <algorithm>

void Enemy::TriggerPunchEffect()
{
	punchAnimationTime = 0.0f;
	punchDirection = GetPunchDirectionToTarget();
	punchHookSide = currentBasicAttackIsRightSwing ? 1.0f : -1.0f;
}

void Enemy::UpdatePunchEffect(float deltaTime) { punchAnimationTime += deltaTime; }

void Enemy::PrimaryAttack()
{
	TriggerPunchEffect();

	const int currentPlayerDistance = DistanceFromBossFootprint(gridPosition, targetGridPosition);
	const bool hit = currentPlayerDistance > 0 && currentPlayerDistance <= GetCurrentBasicAttackRange();

	if (hit && playScene != nullptr)
	{
		playScene->player.Hurt(BASIC_ATTACK_DAMAGE, D_Enemy);
	}

	normalAttackCount++;
	nextBasicAttackIsRightSwing = !currentBasicAttackIsRightSwing;

	EnterRecover(normalAttackCount >= normalAttacksPerCycle ? CYCLE_RECOVER_DURATION : ATTACK_RECOVER_DURATION);
}

void Enemy::SecondaryAttack(float deltaTime)
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
		EnterState(EnemyState::SecondaryRecover);
	}
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
