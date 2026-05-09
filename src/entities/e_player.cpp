#include "arena_manager.h"
#include "entities/enemy.h"
#include "entities/player.h"
#include "keybinds.h"
#include "raylib-cpp.hpp"
#include "scene.h"
#include "utils.h"
#include <algorithm>
#include <cmath>

namespace
{
constexpr int ARENA_CENTER_GRID = CELL_COUNT / 2;

int GridX(Vector2 gridPosition) { return (int)gridPosition.x; }

int GridY(Vector2 gridPosition) { return (int)gridPosition.y; }

bool IsEnemyBlockingGridPosition(Vector2 gridPosition)
{
	return playScene != nullptr && playScene->enemy.isInGrid && playScene->enemy.OccupiesGridPosition(gridPosition);
}

bool IsWalkableGridPosition(Vector2 gridPosition)
{
	return ArenaManager::IsValidGridPosition(ArenaManager::GridPositionToWorld(gridPosition)) &&
		   !IsEnemyBlockingGridPosition(gridPosition);
}

Vector2 NormalizeGridDirection(Vector2 direction)
{
	Vector2 normalizedDirection = Vector2{0.0f, 0.0f};

	if (direction.x > 0.0f)
	{
		normalizedDirection.x = 1.0f;
	}
	else if (direction.x < 0.0f)
	{
		normalizedDirection.x = -1.0f;
	}

	if (direction.y > 0.0f)
	{
		normalizedDirection.y = 1.0f;
	}
	else if (direction.y < 0.0f)
	{
		normalizedDirection.y = -1.0f;
	}

	return normalizedDirection;
}

bool IsZeroDirection(Vector2 direction) { return direction.x == 0.0f && direction.y == 0.0f; }

Vector2 GetHeldMovementDirection()
{
	Vector2 direction = Vector2{0.0f, 0.0f};

	if (IsKeyDown(MOVE_LEFT))
	{
		direction.x -= 1.0f;
	}
	if (IsKeyDown(MOVE_RIGHT))
	{
		direction.x += 1.0f;
	}
	if (IsKeyDown(MOVE_UP))
	{
		direction.y -= 1.0f;
	}
	if (IsKeyDown(MOVE_DOWN))
	{
		direction.y += 1.0f;
	}

	return NormalizeGridDirection(direction);
}

bool IsValidArenaGridPosition(Vector2 gridPosition)
{
	return ArenaManager::IsValidGridPosition(ArenaManager::GridPositionToWorld(gridPosition));
}

int DirectionTowardArenaCenter(int gridCoordinate)
{
	if (gridCoordinate < ARENA_CENTER_GRID)
	{
		return 1;
	}

	if (gridCoordinate > ARENA_CENTER_GRID)
	{
		return -1;
	}

	return 1;
}

Vector2 GetBoundaryAssistDirection(Vector2 playerGridPosition, Vector2 inputDir)
{
	if (IsValidArenaGridPosition(Vector2Add(playerGridPosition, inputDir)))
	{
		return inputDir;
	}

	Vector2 candidates[2] = {inputDir, inputDir};

	if ((int)inputDir.x == 0)
	{
		const int slideX = DirectionTowardArenaCenter(GridX(playerGridPosition));
		candidates[0] = Vector2{(float)slideX, inputDir.y};
		candidates[1] = Vector2{(float)-slideX, inputDir.y};
	}
	else if ((int)inputDir.y == 0)
	{
		const int slideY = DirectionTowardArenaCenter(GridY(playerGridPosition));
		candidates[0] = Vector2{inputDir.x, (float)slideY};
		candidates[1] = Vector2{inputDir.x, (float)-slideY};
	}

	for (const Vector2 &candidate : candidates)
	{
		if (IsWalkableGridPosition(Vector2Add(playerGridPosition, candidate)))
		{
			return candidate;
		}
	}

	return inputDir;
}
} // namespace

Player::Player(raylib::Vector2 startPos) : Entity(startPos)
{
	health = 100;
	gridPosition = startPos;
	position = ArenaManager::GridPositionToWorld(gridPosition);
}

Player::~Player(void) {}

bool Player::IsInvulnerable() { return dashInvulnerabilityTimer > 0.0f; }

void Player::Update()
{

	movedThisFrame = false;
	attackedThisFrame = false;
	dashedThisFrame = false;

	// Lerp Entity Position to Grid Position
	position = Vector2Lerp(position, ArenaManager::GridPositionToWorld(gridPosition), lerpSpeed);
	const float deltaTime = GetFrameTime();
	currentMoveCooldown -= deltaTime;
	attackCooldownTimer -= deltaTime;
	dashCooldownTimer -= deltaTime;
	dashInvulnerabilityTimer -= deltaTime;
	hitAnimationTime += deltaTime;

	Vector2 actionDirection = GetHeldMovementDirection();
	if (IsZeroDirection(actionDirection))
	{
		actionDirection = currentDirection;
	}

	if (IsKeyPressed(PRIMARY))
	{
		TryAttack(actionDirection);
	}

	if (!attackedThisFrame && IsKeyPressed(SECONDARY))
	{
		TryDash(actionDirection);
	}

	if (!dashedThisFrame)
	{
		if (IsKeyDown(MOVE_LEFT))
			TryMove(Vector2(-1, 0));
		if (IsKeyDown(MOVE_RIGHT))
			TryMove(Vector2(1, 0));
		if (IsKeyDown(MOVE_UP))
			TryMove(Vector2(0, -1));
		if (IsKeyDown(MOVE_DOWN))
			TryMove(Vector2(0, 1));
	}

	if (movedThisFrame && !dashedThisFrame)
	{
		currentMoveCooldown = moveCooldown;
	}
	if (dashedThisFrame)
	{
		currentMoveCooldown = 0.0f;
	}
	for (int i = 119; i > 0; i--)
	{
		trail[i] = trail[i - 1];
		trail[i].opacity -= 25;

		if (Vector2Equals(trail[i].pos, position))
		{
			trail[i].opacity = 0;
		}
	}

	trail[0] = {position, 255};

	// THE FIST

	float hitAnimationLerp = sinf(3.5f * sqrtf(hitAnimationTime));
	hitAnimationLerp = std::max(hitAnimationLerp, 0.0f);

	Vector2 handStart = Vector2(0, 20);
	Vector2 handEnd = Vector2(45, 0);
	Vector2 ControlOne = Vector2(20, 25);
	Vector2 ControlTwo = Vector2(45, 10);

	float threshold = hitAnimationLerp * 20;

	peakThreshold = std::max(threshold, peakThreshold);

	threshold = std::max(threshold, peakThreshold);

	if (hitAnimationTime < 1)
	{
		handpos = Utils::BezierLerp(handStart, handEnd, ControlOne, ControlTwo, hitAnimationLerp);
		if (isPunchFlipped)
			handpos.y = -handpos.y;
		float angle = Utils::Vector2ToAngle(attackDirection) * PI / 180.0f;

		handpos = Vector2Rotate(handpos, angle);
	}
	for (int i = 0; i < 20; i++)
	{
		if (i >= threshold)
			continue;

		Vector2 trailPos = Utils::BezierLerp(handStart, handEnd, ControlOne, ControlTwo, (float)i / 20.0f);
		if (isPunchFlipped)
			trailPos.y = -trailPos.y;
		float angle = Utils::Vector2ToAngle(attackDirection) * PI / 180.0f;
		trailPos = Vector2Rotate(trailPos, angle);

		punchTrail[i] = {trailPos, std::max(255 - (int)(hitAnimationTime * (400 - i * 5)), 0)};
	}
}

void Player::Draw()
{
	for (int i = 119; i > 0; i--)
	{
		if (trail[i].opacity < 0)
			continue;
		DrawCircleV(trail[i].pos, CELL_SIZE / 2.0f, Color{50, 120, 160, (unsigned char)trail[i].opacity});
	}

	for (int i = 20; i > 0; i--)
	{
		if (punchTrail[i].opacity < 0)
			continue;

		DrawCircleV(position + punchTrail[i].pos, CELL_SIZE / 6.0f,
					Color{102, 191, 255, (unsigned char)punchTrail[i].opacity});
	}
	if (hitAnimationTime < 1)
	{

		DrawCircleV(position + handpos, CELL_SIZE / 6.0f,
					Color{102, 191, 255, (unsigned char)(255 - hitAnimationTime * 255)});
	}

	const Color playerColor = IsInvulnerable() ? Color{170, 235, 255, 255} : SKYBLUE;
	DrawCircleV(position, CELL_SIZE / 2.0f, playerColor);

	if (hitAnimationTime < 1)
	{
		DrawCircleV(position + handpos, CELL_SIZE / 6.0f, playerColor);
	}
}

void Player::TryMove(Vector2 dir)
{
	if (currentMoveCooldown > 0)
		return;

	Vector2 moveDir = dir;
	Vector2 cornerAssistDir = GetBoundaryAssistDirection(gridPosition, dir);
	if (!Vector2Equals(cornerAssistDir, dir) && IsWalkableGridPosition(Vector2Add(gridPosition, cornerAssistDir)))
	{
		moveDir = cornerAssistDir;
	}

	Vector2 nextGridPosition = Vector2Add(gridPosition, moveDir);
	Vector2 hypotheticalWorldPos = ArenaManager::GridPositionToWorld(nextGridPosition);

	if (!ArenaManager::IsValidGridPosition(hypotheticalWorldPos))
		return;

	if (IsEnemyBlockingGridPosition(nextGridPosition))
	{
		return;
	}

	movedThisFrame = true;
	gridPosition = nextGridPosition;
	currentDirection = moveDir;
}

void Player::TryAttack(Vector2 dir)
{
	if (attackCooldownTimer > 0.0f || dashedThisFrame)
	{
		return;
	}

	const Vector2 attackDir = NormalizeGridDirection(dir);
	if (IsZeroDirection(attackDir))
	{
		return;
	}

	attackedThisFrame = true;
	attackCooldownTimer = attackCooldown;
	hitAnimationTime = 0.0f;
	attackDirection = attackDir;
	currentDirection = attackDir;
	peakThreshold = 0.0f;
	isPunchFlipped = GetRandomValue(0, 1) == 1;

	if (playScene != nullptr && IsEnemyBlockingGridPosition(Vector2Add(gridPosition, attackDir)))
	{
		playScene->EnemyHit(25.0f);
	}
}

void Player::TryDash(Vector2 dir)
{
	if (dashCooldownTimer > 0.0f)
	{
		return;
	}

	Vector2 dashDir = NormalizeGridDirection(dir);
	if (IsZeroDirection(dashDir))
	{
		return;
	}

	const Vector2 startGridPosition = gridPosition;
	Vector2 destination = startGridPosition;
	bool hitEnemy = false;
	bool foundLandingPastEnemy = false;
	Vector2 landingPastEnemy = startGridPosition;
	int enemyHitStep = 0;

	for (int step = 1; step <= dashRange; step++)
	{
		const Vector2 candidate = Vector2Add(startGridPosition, Vector2Scale(dashDir, (float)step));
		if (!IsValidArenaGridPosition(candidate))
		{
			break;
		}

		if (IsEnemyBlockingGridPosition(candidate))
		{
			hitEnemy = true;
			enemyHitStep = step;
			continue;
		}

		if (hitEnemy)
		{
			landingPastEnemy = candidate;
			foundLandingPastEnemy = true;
			break;
		}

		destination = candidate;
	}

	if (hitEnemy && playScene != nullptr)
	{
		destination = foundLandingPastEnemy ? landingPastEnemy : startGridPosition;
		const Vector2 enemyPushDirection = Vector2Scale(dashDir, -1.0f);
		if (playScene->enemy.TryPushGridPosition(enemyPushDirection))
		{
			Vector2 pushedLanding = startGridPosition;
			bool foundPushedLanding = false;

			for (int step = enemyHitStep; step <= dashRange; step++)
			{
				const Vector2 candidate = Vector2Add(startGridPosition, Vector2Scale(dashDir, (float)step));
				if (!IsValidArenaGridPosition(candidate))
				{
					break;
				}

				if (!IsEnemyBlockingGridPosition(candidate))
				{
					pushedLanding = candidate;
					foundPushedLanding = true;
					break;
				}
			}

			if (foundPushedLanding)
			{
				destination = pushedLanding;
			}
			else if (foundLandingPastEnemy && !IsEnemyBlockingGridPosition(landingPastEnemy))
			{
				destination = landingPastEnemy;
			}
		}
	}

	if (Vector2Equals(destination, startGridPosition))
	{
		return;
	}

	gridPosition = destination;
	currentDirection = dashDir;
	if (hitEnemy && playScene != nullptr)
	{
		playScene->EnemyHit(50.0f);
	}
	movedThisFrame = true;
	dashedThisFrame = true;
	dashCooldownTimer = dashCooldown;
	dashInvulnerabilityTimer = dashInvulnerabilityDuration;
}

void Player::Hurt(float amount, DamageType damageType)
{
	if (IsInvulnerable() && damageType != D_EvilZone)
	{
		return;
	}

	health = std::max(0.0f, health - amount);
}

void Player::Knockback(Vector2 Knockback) {}
