#include "arena_manager.h"
#include "entity.h"
#include "raymath.h"
#include "utils.h"
#include <algorithm>
#include <cmath>

namespace
{
constexpr int BOSS_FOOTPRINT_HALF_CELLS = 1;
constexpr float WIND_UP_DURATION = 0.5f;
constexpr float ATTACK_RECOVER_DURATION = 0.9f;
constexpr float CYCLE_RECOVER_DURATION = 1.2f;
constexpr float CHASE_TIMEOUT_RECOVER_DURATION = 0.55f;
constexpr float PUNCH_EFFECT_DURATION = 1.0f;
constexpr int PUNCH_TRAIL_SEGMENTS = 24;

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
	}

	return "Unknown";
}

int Enemy::GetChaseMovesTaken() const { return chaseMovesTaken; }

int Enemy::GetChaseBudget() const { return chaseBudget; }

int Enemy::GetNormalAttackCount() const { return normalAttackCount; }

int Enemy::GetNormalAttacksPerCycle() const { return normalAttacksPerCycle; }

void Enemy::SetTargetGridPosition(Vector2 target) { targetGridPosition = target; }

bool Enemy::OccupiesGridPosition(Vector2 target) const { return BossFootprintContainsCell(gridPosition, target); }

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
		stateTimer = WIND_UP_DURATION;
		attackTargetX = (int)targetGridPosition.x;
		attackTargetY = (int)targetGridPosition.y;
		break;
	case EnemyState::Attack:
		stateTimer = 0.0f;
		break;
	case EnemyState::Recover:
		EnterRecover(ATTACK_RECOVER_DURATION);
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
	const int dx = attackTargetX - GridX(gridPosition);
	const int dy = attackTargetY - GridY(gridPosition);

	if (abs(dx) >= abs(dy) && dx != 0)
	{
		return Vector2{dx > 0 ? 1.0f : -1.0f, 0.0f};
	}

	if (dy != 0)
	{
		return Vector2{0.0f, dy > 0 ? 1.0f : -1.0f};
	}

	return Vector2{1.0f, 0.0f};
}

void Enemy::TriggerPunchEffect()
{
	punchAnimationTime = 0.0f;
	punchPeakThreshold = 0.0f;
	punchDirection = GetPunchDirectionToTarget();
	punchHandPosition = Vector2{0.0f, 0.0f};
}

void Enemy::UpdatePunchEffect(float deltaTime)
{
	punchAnimationTime += deltaTime;
	if (punchAnimationTime >= PUNCH_EFFECT_DURATION)
	{
		return;
	}

	const float punchLerp = std::max(sinf(3.5f * powf(punchAnimationTime, 0.5f)), 0.0f);
	const Vector2 handStart = Vector2{0.0f, CELL_SIZE * 0.45f};
	const Vector2 handEnd = Vector2{CELL_SIZE * 1.2f, 0.0f};
	const Vector2 controlOne = Vector2{CELL_SIZE * 0.45f, CELL_SIZE * 0.6f};
	const Vector2 controlTwo = Vector2{CELL_SIZE * 1.2f, CELL_SIZE * 0.25f};
	const float angle = Utils::Vector2ToAngle(punchDirection) * DEG2RAD;

	punchPeakThreshold = std::max(punchLerp * PUNCH_TRAIL_SEGMENTS, punchPeakThreshold);
	punchHandPosition = Vector2Rotate(Utils::BezierLerp(handStart, handEnd, controlOne, controlTwo, punchLerp), angle);
}

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
		if (BossFootprintIsAdjacentToCell(gridPosition, targetGridPosition))
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
					!BossFootprintIsAdjacentToCell(gridPosition, targetGridPosition))
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
		bool hit = ((int)targetGridPosition.x == attackTargetX && (int)targetGridPosition.y == attackTargetY);
		if (hit)
		{
			// TODO: deal damage
		}
		normalAttackCount++;
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
				normalAttackCount = 0;
				EnterState(EnemyState::Idle);
			}
			else
			{
				EnterState(EnemyState::Advance);
			}
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

	const Vector2 punchOrigin = Vector2Add(position, Vector2Scale(punchDirection, CELL_SIZE * 0.95f));
	const Vector2 handStart = Vector2{0.0f, CELL_SIZE * 0.45f};
	const Vector2 handEnd = Vector2{CELL_SIZE * 1.2f, 0.0f};
	const Vector2 controlOne = Vector2{CELL_SIZE * 0.45f, CELL_SIZE * 0.6f};
	const Vector2 controlTwo = Vector2{CELL_SIZE * 1.2f, CELL_SIZE * 0.25f};
	const float angle = Utils::Vector2ToAngle(punchDirection) * DEG2RAD;

	for (int i = PUNCH_TRAIL_SEGMENTS; i > 0; i--)
	{
		if (i >= punchPeakThreshold)
		{
			continue;
		}

		Vector2 trailPos =
			Utils::BezierLerp(handStart, handEnd, controlOne, controlTwo, (float)i / (float)PUNCH_TRAIL_SEGMENTS);
		trailPos = Vector2Rotate(trailPos, angle);

		const int opacity = std::max(255 - (int)(punchAnimationTime * (460.0f - i * 6.0f)), 0);
		DrawCircleV(Vector2Add(punchOrigin, trailPos), CELL_SIZE * 0.28f,
					Color{255, 116, 64, (unsigned char)opacity});
	}

	DrawCircleV(Vector2Add(punchOrigin, punchHandPosition), CELL_SIZE * 0.38f, Color{255, 184, 84, 255});
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
		bodyColor = RED;
		radius += sinf((float)GetTime() * 24.0f) * 5.0f;
		break;
	case EnemyState::Attack:
		bodyColor = WHITE;
		radius += 6.0f;
		break;
	case EnemyState::Recover:
		bodyColor = Fade(ORANGE, 0.45f);
		break;
	}

	DrawCircleV(position, radius, bodyColor);

	if (currentState == EnemyState::WindUp)
	{
		const Vector2 targetWorld = ArenaManager::GridPositionToWorld(Vector2{(float)attackTargetX, (float)attackTargetY});
		const Rectangle targetRect = {targetWorld.x - CELL_SIZE / 2.0f, targetWorld.y - CELL_SIZE / 2.0f, CELL_SIZE,
									  CELL_SIZE};
		DrawRectangleRec(targetRect, Fade(RED, 0.25f));
		DrawRectangleLinesEx(targetRect, 3.0f, RED);
	}

	DrawPunchEffect();
}

Rectangle Enemy::GetBBoxWorld()
{
	return Rectangle(position.x - CELL_SIZE * 1.5f, position.y - CELL_SIZE * 1.5f, CELL_SIZE * 3, CELL_SIZE * 3);
}
