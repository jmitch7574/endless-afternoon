#include "arena_manager.h"
#include "entities/enemy.h"
#include "raymath.h"
#include <algorithm>
#include <cstdlib>
#include "scene.h"

namespace
{
int GridX(Vector2 cell) { return (int)cell.x; }

int GridY(Vector2 cell) { return (int)cell.y; }
} // namespace

int Enemy::DistanceFromBossFootprint(Vector2 bossCenter, Vector2 cell)
{
	const int dx = std::abs(GridX(cell) - GridX(bossCenter));
	const int dy = std::abs(GridY(cell) - GridY(bossCenter));
	const int outsideX = std::max(dx - BOSS_FOOTPRINT_HALF_CELLS, 0);
	const int outsideY = std::max(dy - BOSS_FOOTPRINT_HALF_CELLS, 0);

	return std::max(outsideX, outsideY);
}

bool Enemy::BossFootprintContainsCell(Vector2 bossCenter, Vector2 cell)
{
	return DistanceFromBossFootprint(bossCenter, cell) == 0;
}

bool Enemy::IsBossFootprintValid(Vector2 bossCenter)
{
	for (int x = -BOSS_FOOTPRINT_HALF_CELLS; x <= BOSS_FOOTPRINT_HALF_CELLS; x++)
	{
		for (int y = -BOSS_FOOTPRINT_HALF_CELLS; y <= BOSS_FOOTPRINT_HALF_CELLS; y++)
		{
			const Vector2 footprintCell = Vector2Add(bossCenter, Vector2{(float)x, (float)y});
			if (!ArenaManager::IsValidGridPosition(ArenaManager::GridPositionToWorld(footprintCell)))
			{
				return false;
			}
		}
	}

	return true;
}

Enemy::Enemy(raylib::Vector2 startGridPos) : Entity(ArenaManager::GridPositionToWorld(startGridPos))
{
	health = (float)maxHealth;
	gridPosition = startGridPos;
	targetGridPosition = startGridPos;
	isInGrid = true;
}
Enemy::~Enemy(void) {}

EnemyState Enemy::GetState() const { return currentState; }

const char *Enemy::GetStateName() const
{
	switch (currentState)
	{
	case EnemyState::Idle:
		return "Idle";
	case EnemyState::Advance:
		return "Advance";
	case EnemyState::WindUp:
		return "WindUp";
	case EnemyState::Attack:
		return "Attack";
	case EnemyState::Recover:
		return "Recover";
	case EnemyState::SecondaryWindUp:
		return "SecondaryWindUp";
	case EnemyState::SecondaryAttack:
		return "SecondaryAttack";
	case EnemyState::SecondaryRecover:
		return "SecondaryRecover";
	case EnemyState::SpecialWindUp:
		return "SpecialWindUp";
	case EnemyState::SpecialAttack:
		return "SpecialAttack";
	case EnemyState::SpecialRecover:
		return "SpecialRecover";
	}

	return "Unknown";
}

int Enemy::GetChaseMovesTaken() const { return chaseMovesTaken; }

int Enemy::GetChaseBudget() const { return chaseBudget; }

int Enemy::GetNormalAttackCount() const { return normalAttackCount; }

int Enemy::GetNormalAttacksPerCycle() const { return normalAttacksPerCycle; }

int Enemy::GetMaxHealth() const { return maxHealth; }

void Enemy::Hurt(float amount) { health = std::max(0.0f, health - amount); timeSinceLastHit = 0; }

void Enemy::SetTargetGridPosition(Vector2 target) { targetGridPosition = target; }

bool Enemy::OccupiesGridPosition(Vector2 target) const { return BossFootprintContainsCell(gridPosition, target); }

bool Enemy::TryPushGridPosition(Vector2 direction)
{
	const Vector2 nextGridPosition = Vector2Add(gridPosition, direction);
	if (!IsBossFootprintValid(nextGridPosition))
	{
		return false;
	}

	gridPosition = nextGridPosition;
	return true;
}

int Enemy::GetNextBasicAttackRange() const
{
	return nextBasicAttackIsRightSwing ? minuteHandAttackRange : hourHandAttackRange;
}

int Enemy::GetCurrentBasicAttackRange() const
{
	return currentBasicAttackIsRightSwing ? minuteHandAttackRange : hourHandAttackRange;
}

void Enemy::EnterState(EnemyState nextState)
{
	currentState = nextState;

	switch (currentState)
	{
	case EnemyState::Idle:
		stateTimer = 0.0f;
		break;
	case EnemyState::Advance:
		chaseMovesTaken = 0;
		break;
	case EnemyState::WindUp:
		currentBasicAttackIsRightSwing = nextBasicAttackIsRightSwing;
		stateTimer = baseAttackWindUpDuration;
		attackTargetX = (int)targetGridPosition.x;
		attackTargetY = (int)targetGridPosition.y;
		break;
	case EnemyState::Attack:
		stateTimer = 0.0f;
		break;
	case EnemyState::Recover:
		EnterRecover(ATTACK_RECOVER_DURATION);
		break;
	case EnemyState::SecondaryWindUp:
		currentSecondaryAttack = nextSecondaryAttack;
		if (currentSecondaryAttack == 2)
		{
			spinningSecondarySpinAngle = (float)GetRandomValue(0, 359);
			spinningSecondarySpinDirection = GetRandomValue(0, 1) == 0 ? -1 : 1;
		}
		stateTimer = secondaryWindUpDuration;
		break;
	case EnemyState::SecondaryAttack:
		if (currentSecondaryAttack == 2)
		{
			stateTimer = spinningSecondaryAttackDuration;
			spinningSecondaryDamageCooldown = 0.0f;
		}
		else
		{
			stateTimer = secondaryAttackDuration;
			secondaryAttackHasHit = false;
			secondaryPulsePreviousRadius = 0.0f;
		}
		break;
	case EnemyState::SecondaryRecover:
		stateTimer = secondaryRecoverDuration;
		break;
	case EnemyState::SpecialWindUp:
		currentSpecialAttack = nextSpecialAttack;
		stateTimer = specialWindUpDuration;
		break;
	case EnemyState::SpecialAttack:
		stateTimer = 0.0f;
		break;
	case EnemyState::SpecialRecover:
		stateTimer = specialRecoverDuration;
		break;
	}
}

void Enemy::EnterRecover(float duration)
{
	currentState = EnemyState::Recover;
	stateTimer = duration;
}

Vector2 Enemy::GetPunchDirectionToTarget() const
{
	const Vector2 targetOffset = Vector2Subtract(Vector2{(float)attackTargetX, (float)attackTargetY}, gridPosition);

	if (Vector2Length(targetOffset) <= 0.0f)
	{
		return Vector2{1.0f, 0.0f};
	}

	return Vector2Normalize(targetOffset);
}

void Enemy::UpdateIdle() { EnterState(EnemyState::Advance); }

void Enemy::UpdateAdvance()
{
	TryPrimaryAttack();
	if (currentState != EnemyState::Advance)
	{
		return;
	}

	if (currentMoveCooldown > 0 || primaryAttackMovementLockTimer > 0.0f)
	{
		return;
	}

	TryMoveTowardTarget();
	TryPrimaryAttack();

	clockMarkingOffset += 27;
	clockMarkingValue -= 13;
}

bool Enemy::TryMoveTowardTarget()
{
	const bool isOverlappingTarget = BossFootprintContainsCell(gridPosition, targetGridPosition);
	const int currentDistance = DistanceFromBossFootprint(gridPosition, targetGridPosition);
	const Vector2 directions[] = {
		Vector2{1.0f, 0.0f}, Vector2{-1.0f, 0.0f}, Vector2{0.0f, 1.0f},	 Vector2{0.0f, -1.0f},
		Vector2{1.0f, 1.0f}, Vector2{1.0f, -1.0f}, Vector2{-1.0f, 1.0f}, Vector2{-1.0f, -1.0f},
	};
	Vector2 bestNextPos = gridPosition;
	int bestDistance = isOverlappingTarget ? -1 : currentDistance;
	bool foundMove = false;

	for (const Vector2 &moveDir : directions)
	{
		const Vector2 nextPos = Vector2Add(gridPosition, moveDir);
		if (!IsBossFootprintValid(nextPos))
		{
			continue;
		}

		const int nextDistance = DistanceFromBossFootprint(nextPos, targetGridPosition);
		if (isOverlappingTarget)
		{
			if (!foundMove || nextDistance > bestDistance)
			{
				bestNextPos = nextPos;
				bestDistance = nextDistance;
				foundMove = true;
			}
		}
		else if (!BossFootprintContainsCell(nextPos, targetGridPosition) && nextDistance < bestDistance)
		{
			bestNextPos = nextPos;
			bestDistance = nextDistance;
			foundMove = true;
		}
	}

	if (!foundMove)
	{
		currentMoveCooldown = moveCooldown;
		return false;
	}

	gridPosition = bestNextPos;
	chaseMovesTaken++;
	currentMoveCooldown = moveCooldown;

	if (chaseMovesTaken >= chaseBudget &&
		DistanceFromBossFootprint(gridPosition, targetGridPosition) > GetNextBasicAttackRange())
	{
		chaseMovesTaken = 0;
	}

	return true;
}

void Enemy::UpdateWindUp(float deltaTime)
{
	stateTimer -= deltaTime;
	if (stateTimer <= 0)
	{
		EnterState(EnemyState::Attack);
	}
}

void Enemy::UpdateRecover(float deltaTime)
{
	stateTimer -= deltaTime;
	if (stateTimer > 0)
	{
		return;
	}

	if (normalAttackCount < normalAttacksPerCycle)
	{
		EnterState(EnemyState::Advance);
		return;
	}

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

void Enemy::UpdateSecondaryWindUp(float deltaTime)
{
	stateTimer -= deltaTime;
	if (stateTimer <= 0)
	{
		EnterState(EnemyState::SecondaryAttack);
	}
}

void Enemy::UpdateSecondaryRecover(float deltaTime)
{
	stateTimer -= deltaTime;
	if (stateTimer <= 0)
	{
		EnterState(EnemyState::Advance);
	}
}

void Enemy::UpdateSpecialWindUp(float deltaTime)
{
	stateTimer -= deltaTime;
	if (stateTimer <= 0)
	{
		EnterState(EnemyState::SpecialAttack);
	}
}

void Enemy::UpdateSpecialRecover(float deltaTime)
{
	stateTimer -= deltaTime;
	if (stateTimer <= 0)
	{
		EnterState(EnemyState::Advance);
	}
}

void Enemy::Update()
{
	const float deltaTime = GetFrameTime();

	position = Vector2Lerp(position, ArenaManager::GridPositionToWorld(gridPosition), lerpSpeed);
	currentMoveCooldown -= deltaTime;
	primaryAttackMovementLockTimer -= deltaTime;
	currentPrimaryAttackCooldown -= deltaTime;
	timeSinceLastHit += deltaTime;
	UpdatePunchEffect(deltaTime);

	switch (currentState)
	{
	case EnemyState::Idle:
		UpdateIdle();
		break;
	case EnemyState::Advance:
		UpdateAdvance();
		break;
	case EnemyState::WindUp:
		UpdateWindUp(deltaTime);
		break;
	case EnemyState::Attack:
		PrimaryAttack();
		break;
	case EnemyState::Recover:
		UpdateRecover(deltaTime);
		break;
	case EnemyState::SecondaryWindUp:
		UpdateSecondaryWindUp(deltaTime);
		break;
	case EnemyState::SecondaryAttack:
		SecondaryAttack(deltaTime);
		break;
	case EnemyState::SecondaryRecover:
		UpdateSecondaryRecover(deltaTime);
		break;
	case EnemyState::SpecialWindUp:
		UpdateSpecialWindUp(deltaTime);
		break;
	case EnemyState::SpecialAttack:
		RunSelectedSpecialAttack();
		EnterState(EnemyState::SpecialRecover);
		break;
	case EnemyState::SpecialRecover:
		UpdateSpecialRecover(deltaTime);
		break;
	}

	UpdateEnemyFace();
}

void Enemy::UpdateEnemyFace() 
{

	currentEyeRotation = Lerp(currentEyeRotation, TargetEyeRotation(), 0.01f);
	eyeOffsets = Vector2Lerp(eyeOffsets, Vector2Scale(Vector2Normalize(Vector2Subtract(playScene->player.GetPosition(), position)), maxEyeMovement), 0.1f);

	Vector2 normalizedDistanceFromLastFrame = Vector2Normalize(Vector2Subtract(position, positionLastFrame));

	currentMoustacheGap = Lerp(currentMoustacheGap, TargetMoustacheGap(normalizedDistanceFromLastFrame.y), moustacheGapLerp);
	currentMoustacheOffset = Lerp(currentMoustacheOffset, TargetMoustacheOffset(normalizedDistanceFromLastFrame.x), moustacheOffsetLerp);

	positionLastFrame = Vector2Lerp(positionLastFrame, position, 0.4f);

	clockMarkingValue = Lerp(clockMarkingValue, clockMarkingOffset, clockMarkingLerp);
}