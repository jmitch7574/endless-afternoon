#include "arena_manager.h"
#include "entity.h"
#include "keybinds.h"
#include <scene.h>
#include <iostream>
#include "raylib-cpp.hpp"
#include "utils.h"

Player::Player(raylib::Vector2 startPos) : Entity(startPos)
{
  health = 100;
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
  if (hitAnimationTime < 1)
  {

    DrawCircleV(position + handpos, CELL_SIZE / 6.0f, Color{ 102, 191, 255, (unsigned char)(255 - hitAnimationTime * 255) });
  }

	DrawCircleV(position, CELL_SIZE / 2.0f, SKYBLUE);

}

void Player::TryMove(Vector2 dir) {
  if (currentMoveCooldown > 0) return;
  if (attackedThisFrame) return;

  Vector2 hypotheticalWorldPos = ArenaManager::GridPositionToWorld(Vector2Add(gridPosition, dir));

  if (!ArenaManager::IsValidGridPosition(hypotheticalWorldPos)) return;

  movedThisFrame = true;

  bool enemyCollide = CheckCollisionCircleRec(hypotheticalWorldPos, CELL_SIZE / 2.0f, playScene->enemy.GetBBoxWorld());
  
  if (enemyCollide && playScene->enemy.isInGrid){
    // Successful Attack
    attackedThisFrame = true;
    
    std::cout << "Enemy Attacked";

    hitAnimationTime = 0;
    currentDirection = dir;

    peakThreshold = 0;

    playScene->EnemyHit();


    return;
  }

  gridPosition = Vector2Add(gridPosition, dir);
  currentDirection = dir;
}

void Player::Hurt(float amount) 
{
  health -= amount;
}

void Player::Knockback(Vector2 Knockback) 
{
  
}
