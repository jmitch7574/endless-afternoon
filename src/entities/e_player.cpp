#include "arena_manager.h"
#include "entity.h"
#include "keybinds.h"
#include <scene.h>
#include <iostream>
#include "raylib-cpp.hpp"
#include "utils.h"

namespace
{
constexpr int ARENA_CENTER_GRID = CELL_COUNT / 2;

int GridX(Vector2 gridPosition)
{
	return (int)gridPosition.x;
}

int GridY(Vector2 gridPosition)
{
	return (int)gridPosition.y;
}

bool IsEnemyBlockingGridPosition(Vector2 gridPosition)
{
	return playScene != nullptr && playScene->enemy.isInGrid && playScene->enemy.OccupiesGridPosition(gridPosition);
}

bool IsWalkableGridPosition(Vector2 gridPosition)
{
	return ArenaManager::IsValidGridPosition(ArenaManager::GridPositionToWorld(gridPosition)) &&
		   !IsEnemyBlockingGridPosition(gridPosition);
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


struct PlayerTrail
{
	Vector2 pos;
	int opacity;
};

PlayerTrail trail[120] = {0};
PlayerTrail punchTrail[40] = {0};

bool movedThisFrame = false;
bool attackedThisFrame = false;
float hitAnimationTime = 1000;
Vector2 currentDirection = Vector2(1, 0);
Vector2 handpos;
float peakThreshold = 0;

Player::Player(raylib::Vector2 startPos) : Entity(startPos)
{
	gridPosition = startPos;
	position = ArenaManager::GridPositionToWorld(gridPosition);
}

Player::~Player(void) {}

void Player::Update()
{

  movedThisFrame = false;
  attackedThisFrame = false;
  
	// Lerp Entity Position to Grid Position
	position = Vector2Lerp(position, ArenaManager::GridPositionToWorld(gridPosition), lerpSpeed);
	currentMoveCooldown -= GetFrameTime();
  hitAnimationTime += GetFrameTime();

	Vector2 gridPositionLastFrame = gridPosition;
	if (IsKeyDown(MOVE_LEFT))
		TryMove(Vector2(-1, 0));
	if (IsKeyDown(MOVE_RIGHT))
		TryMove(Vector2(1, 0));
	if (IsKeyDown(MOVE_UP))
		TryMove(Vector2(0, -1));
	if (IsKeyDown(MOVE_DOWN))
		TryMove(Vector2(0, 1));

  if (movedThisFrame) 
  {
    currentMoveCooldown = moveCooldown;
  }
  if (attackedThisFrame)
  {
    currentMoveCooldown = moveCooldown * 3;
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

  
  float hitAnimationLerp = sinf((3.5 * pow(hitAnimationTime, 0.5f)));
  hitAnimationLerp = std::max(hitAnimationLerp, 0.0f);

  Vector2 handStart = Vector2(0, 20);
  Vector2 handEnd   = Vector2(45, 0);
  Vector2 ControlOne = Vector2(20, 25);
  Vector2 ControlTwo = Vector2(45, 10);

  float threshold = hitAnimationLerp * 20;
  
  peakThreshold = std::max(threshold, peakThreshold);

  threshold = std::max(threshold, peakThreshold);


  if (hitAnimationTime < 1)
  {
    handpos = Utils::BezierLerp(handStart, handEnd, ControlOne, ControlTwo, hitAnimationLerp);
    float angle = Utils::Vector2ToAngle(currentDirection) * PI / 180.0f;

    handpos = Vector2Rotate(handpos, angle);

  }
  for (int i = 0; i < 20; i++)
  {
    if (i >= threshold) continue;

    Vector2 trailPos = Utils::BezierLerp(handStart, handEnd, ControlOne, ControlTwo, (float)i / 20.0f);
    float angle = Utils::Vector2ToAngle(currentDirection) * PI / 180.0f;
    trailPos = Vector2Rotate(trailPos, angle);


    punchTrail[i] = {
      trailPos,
      std::max(255 - (int)(hitAnimationTime * (400 - i * 5)), 0)
    };
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

		DrawCircleV(position + punchTrail[i].pos, CELL_SIZE / 6.0f, Color{ 102, 191, 255, (unsigned char) punchTrail[i].opacity });
	}

	DrawCircleV(position, CELL_SIZE / 2.0f, SKYBLUE);

  if (hitAnimationTime < 1)
  {
    DrawCircleV(position + handpos, CELL_SIZE / 6.0f, SKYBLUE);
  }
}

void Player::TryMove(Vector2 dir) {
  if (currentMoveCooldown > 0) return;
  if (attackedThisFrame) return;

  Vector2 moveDir = dir;
  Vector2 cornerAssistDir = GetBoundaryAssistDirection(gridPosition, dir);
  if (!Vector2Equals(cornerAssistDir, dir) && IsWalkableGridPosition(Vector2Add(gridPosition, cornerAssistDir)))
  {
    moveDir = cornerAssistDir;
  }

  Vector2 nextGridPosition = Vector2Add(gridPosition, moveDir);
  Vector2 hypotheticalWorldPos = ArenaManager::GridPositionToWorld(nextGridPosition);

  if (!ArenaManager::IsValidGridPosition(hypotheticalWorldPos)) return;

  movedThisFrame = true;

  bool enemyCollide = IsEnemyBlockingGridPosition(nextGridPosition);
  
  if (enemyCollide){
    // Successful Attack
    attackedThisFrame = true;
    
    std::cout << "Enemy Attacked";

    hitAnimationTime = 0;
    currentDirection = moveDir;

    peakThreshold = 0;


    return;
  }

  gridPosition = nextGridPosition;
  currentDirection = moveDir;
}
