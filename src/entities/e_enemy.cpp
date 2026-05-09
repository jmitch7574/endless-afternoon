#include "arena_manager.h"
#include "custom_draws.h"
#include "entity.h"
#include "raymath.h"
#include "utils.h"
#include <algorithm>
#include <cmath>

namespace
{
constexpr int BOSS_FOOTPRINT_HALF_CELLS = 1;
constexpr float ATTACK_RECOVER_DURATION = 0.9f;
constexpr float CYCLE_RECOVER_DURATION = 1.2f;
constexpr float CHASE_TIMEOUT_RECOVER_DURATION = 0.55f;
constexpr float PUNCH_EFFECT_DURATION = 0.75f;
constexpr float CLOCK_HAND_SWEEP_DEGREES = 125.0f;
constexpr int CLOCK_HAND_TRAIL_SAMPLES = 14;
constexpr float CLOCK_HAND_TRAIL_STEP = 0.02f;
constexpr int SECONDARY_PULSE_SEGMENTS = 96;

int GridX(Vector2 cell) { return (int)cell.x; }

int GridY(Vector2 cell) { return (int)cell.y; }

int DistanceFromBossFootprint(Vector2 bossCenter, Vector2 cell)
{
	const int dx = abs(GridX(cell) - GridX(bossCenter));
	const int dy = abs(GridY(cell) - GridY(bossCenter));
	const int outsideX = std::max(dx - BOSS_FOOTPRINT_HALF_CELLS, 0);
	const int outsideY = std::max(dy - BOSS_FOOTPRINT_HALF_CELLS, 0);

	return outsideX + outsideY;
}

bool BossFootprintContainsCell(Vector2 bossCenter, Vector2 cell)
{
	return DistanceFromBossFootprint(bossCenter, cell) == 0;
}

bool BossFootprintIsAdjacentToCell(Vector2 bossCenter, Vector2 cell)
{
	return DistanceFromBossFootprint(bossCenter, cell) == 1;
}

bool IsBossFootprintValid(Vector2 bossCenter)
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

void DrawEnemyFace(Vector2 center, float radius)
{
	const Rectangle leftEye = {center.x - radius * 0.52f, center.y - radius * 0.18f, radius * 0.62f, radius * 0.22f};
	const Rectangle rightEye = {center.x + radius * 0.52f, center.y - radius * 0.18f, radius * 0.62f, radius * 0.22f};

	DrawRectanglePro(leftEye, Vector2{leftEye.width * 0.5f, leftEye.height * 0.5f}, 38.0f, WHITE);
	DrawRectanglePro(rightEye, Vector2{rightEye.width * 0.5f, rightEye.height * 0.5f}, -38.0f, WHITE);

	DrawCircleV(Vector2Add(center, Vector2{-radius * 0.30f, radius * 0.03f}), radius * 0.16f, WHITE);
	DrawCircleV(Vector2Add(center, Vector2{radius * 0.30f, radius * 0.03f}), radius * 0.16f, WHITE);

	DrawCircleV(Vector2Add(center, Vector2{0.0f, radius * 0.28f}), radius * 0.13f, WHITE);
}

Color ClockHandOrange(unsigned char alpha) { return Color{196, 116, 36, alpha}; }

void DrawAttackClockHand(Vector2 clockCenter, float sweepAngle, bool isRightSwing, unsigned char alpha,
						 float lengthScale = 1.0f)
{
	const float faceRadius = CELL_SIZE * 1.5f;
	const float innerRadius = faceRadius + CELL_SIZE * 0.05f;
	const float handLength = (isRightSwing ? CELL_SIZE * 1.5f : CELL_SIZE) * lengthScale;
	const float handThickness = isRightSwing ? CELL_SIZE * 0.14f : CELL_SIZE * 0.22f;
	const float fletchLength = (isRightSwing ? CELL_SIZE * 0.22f : CELL_SIZE * 0.28f) * lengthScale;
	const Vector2 sweepDirection = Utils::AngleToVector2(sweepAngle);
	const Vector2 handPivot = Vector2Add(clockCenter, Vector2Scale(sweepDirection, innerRadius));

	CustomDraws::DrawArrow(handPivot, sweepDirection, handLength, handThickness, fletchLength, 32.0f,
						   ClockHandOrange(alpha));
	DrawCircleV(handPivot, handThickness * 0.62f, Color{230, 148, 58, alpha});
}

} // namespace

Enemy::Enemy(raylib::Vector2 startGridPos) : Entity(ArenaManager::GridPositionToWorld(startGridPos))
{
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
	}

	return "Unknown";
}

int Enemy::GetChaseMovesTaken() const { return chaseMovesTaken; }

int Enemy::GetChaseBudget() const { return chaseBudget; }

int Enemy::GetNormalAttackCount() const { return normalAttackCount; }

int Enemy::GetNormalAttacksPerCycle() const { return normalAttacksPerCycle; }

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
		stateTimer = secondaryWindUpDuration;
		break;
	case EnemyState::SecondaryAttack:
		stateTimer = secondaryAttackDuration;
		secondaryAttackHasHit = false;
		secondaryPulsePreviousRadius = 0.0f;
		break;
	case EnemyState::SecondaryRecover:
		stateTimer = secondaryRecoverDuration;
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

void Enemy::TriggerPunchEffect()
{
	punchAnimationTime = 0.0f;
	punchDirection = GetPunchDirectionToTarget();
	punchHookSide = currentBasicAttackIsRightSwing ? 1.0f : -1.0f;
}

void Enemy::UpdatePunchEffect(float deltaTime) { punchAnimationTime += deltaTime; }

void Enemy::Update()
{
	const float deltaTime = GetFrameTime();

	position = Vector2Lerp(position, ArenaManager::GridPositionToWorld(gridPosition), lerpSpeed);
	currentMoveCooldown -= deltaTime;
	UpdatePunchEffect(deltaTime);

	int dist = DistanceFromBossFootprint(gridPosition, targetGridPosition);

	switch (currentState)
	{
	// Step 4: Idle — wait until player is within 6 cells
	case EnemyState::Idle:
		if (dist <= aggroRange)
		{
			EnterState(EnemyState::Advance);
		}
		break;

	// Steps 1+2+4: Advance — move toward player, stop when adjacent, never overlap
	case EnemyState::Advance:
		if (dist > 0 && dist <= GetNextBasicAttackRange())
		{
			// Step 2: adjacent — start wind up
			EnterState(EnemyState::WindUp);
		}
		else if (currentMoveCooldown <= 0)
		{
			const bool isOverlappingTarget = BossFootprintContainsCell(gridPosition, targetGridPosition);
			const Vector2 directions[] = {
				Vector2{1.0f, 0.0f},
				Vector2{-1.0f, 0.0f},
				Vector2{0.0f, 1.0f},
				Vector2{0.0f, -1.0f},
			};
			Vector2 bestNextPos = gridPosition;
			int bestDistance = isOverlappingTarget ? -1 : dist;
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

			if (foundMove)
			{
				gridPosition = bestNextPos;
				chaseMovesTaken++;
				currentMoveCooldown = moveCooldown;

				if (chaseMovesTaken >= chaseBudget &&
					DistanceFromBossFootprint(gridPosition, targetGridPosition) > GetNextBasicAttackRange())
				{
					EnterRecover(CHASE_TIMEOUT_RECOVER_DURATION);
				}
			}
			else
			{
				EnterRecover(CHASE_TIMEOUT_RECOVER_DURATION);
			}
		}
		break;

	// Step 5: WindUp — wait 0.5s, then attack the locked cell
	case EnemyState::WindUp:
		stateTimer -= deltaTime;
		if (stateTimer <= 0)
		{
			EnterState(EnemyState::Attack);
		}
		break;

	// Step 5: Attack — check if player is still on the target cell
	case EnemyState::Attack:
	{
		TriggerPunchEffect();
		const int currentPlayerDistance = DistanceFromBossFootprint(gridPosition, targetGridPosition);
		bool hit = currentPlayerDistance > 0 && currentPlayerDistance <= GetCurrentBasicAttackRange();
		if (hit)
		{
			// TODO: deal damage
		}
		normalAttackCount++;
		nextBasicAttackIsRightSwing = !currentBasicAttackIsRightSwing;
		EnterRecover(normalAttackCount >= normalAttacksPerCycle ? CYCLE_RECOVER_DURATION : ATTACK_RECOVER_DURATION);
		break;
	}

	// Step 5: Recover — wait 1.0s (punish window), then go back to Advance
	case EnemyState::Recover:
		stateTimer -= deltaTime;
		if (stateTimer <= 0)
		{
			if (normalAttackCount >= normalAttacksPerCycle)
			{
				EnterState(EnemyState::SecondaryWindUp);
			}
			else
			{
				EnterState(EnemyState::Advance);
			}
		}
		break;
	case EnemyState::SecondaryWindUp:
		stateTimer -= deltaTime;
		if (stateTimer <= 0)
		{
			EnterState(EnemyState::SecondaryAttack);
		}
		break;
	case EnemyState::SecondaryAttack:
	{
		stateTimer -= deltaTime;
		const float progress = std::max(std::min(1.0f - (stateTimer / secondaryAttackDuration), 1.0f), 0.0f);
		const float currentPulseRadius = progress * secondaryAttackRange;
		const int playerDistance = DistanceFromBossFootprint(gridPosition, targetGridPosition);

		if (!secondaryAttackHasHit && playerDistance > 0 && playerDistance <= secondaryAttackRange &&
			(float)playerDistance > secondaryPulsePreviousRadius && (float)playerDistance <= currentPulseRadius)
		{
			secondaryAttackHasHit = true;
			// TODO: deal damage
		}

		secondaryPulsePreviousRadius = currentPulseRadius;

		if (stateTimer <= 0)
		{
			EnterState(EnemyState::SecondaryRecover);
		}
		break;
	}
	case EnemyState::SecondaryRecover:
		stateTimer -= deltaTime;
		if (stateTimer <= 0)
		{
			normalAttackCount = 0;
			EnterState(dist <= aggroRange ? EnemyState::Advance : EnemyState::Idle);
		}
		break;
	}
}

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
	}

	DrawCircleV(position, radius + CELL_SIZE * 0.13f, WHITE);
	DrawCircleV(position, radius, bodyColor);
	DrawEnemyFace(position, radius);

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
