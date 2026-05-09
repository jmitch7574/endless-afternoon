#include "arena_manager.h"
#include "custom_draws.h"
#include "scene.h"
#include <cmath>
#include <raylib-cpp.hpp>
#include "arena_manager.h"
#include "custom_draws.h"

PlayMode *playScene;

PlayMode::PlayMode()
	: player(Vector2{10, 10}), enemy(Vector2{5, 5}), minuteHand(Vector2{960, 540}, -90.0f, 440.0f, 8.0f, WHITE),
	  hourHand(Vector2{960, 540}, 0.0f, 280.0f, 12.0f, WHITE)
{
	playScene = this;
}
PlayMode::~PlayMode(void) { playScene = nullptr; }

void PlayMode::Update()
{
	player.Update();
	enemy.SetTargetGridPosition(player.gridPosition);
	enemy.Update();
	minuteHand.Update();
	hourHand.Update();

  if (IsKeyDown(KEY_L)) 
  {
    minuteHand.BeginBigDeadlySpin();
    hourHand.BeginBigDeadlySpin();
  }

  // Special Check - Is the player in the evil zone

  bool collision = CheckCollisionPointPoly(
    player.GetPosition(),
    new Vector2[]{
      minuteHand.GetPosition(), 
      minuteHand.GetLargeExtendedPoint(), 
      Vector2Scale(Vector2Normalize(Vector2Scale(Vector2Add(minuteHand.GetLargeExtendedPoint(),  hourHand.GetLargeExtendedPoint()), 0.5f)), 10000), 
      hourHand.GetLargeExtendedPoint()
    },
    4
  );

  timeSinceEvilZoneTick += GetFrameTime();

  if (collision)
  {
      if (timeSinceEvilZoneTick >= 3.0f)
      {
          player.Hurt(40);
          timeSinceEvilZoneTick = 0;
      }
      else if (timeSinceEvilZoneTick >= 1.0f)
      {
          player.Hurt(5);
          timeSinceEvilZoneTick = 0;
      }
  }
}
void PlayMode::Draw()
{
	ClearBackground(BLACK);
	ArenaManager::DrawLevelGrid();

  DrawEvilZone();

	ArenaManager::MaskOutsideOctagon();
	ArenaManager::DrawOctagonBoundary();
	ArenaManager::DrawClockMarkers();
	hourHand.Draw();
	minuteHand.Draw();
	player.Draw();
	enemy.Draw();

	CustomDraws::DrawArrow(Vector2(400, 400), 0, 200, 10, 50, 90, GOLD);

#ifndef NDEBUG
	DrawText(TextFormat("Player Pos: %f, %f", player.gridPosition.x, player.gridPosition.y), 20, 20, 20, WHITE);
	DrawText(TextFormat("Player Health: %f / %i", player.GetHealth(), 100), 20, 50, 20, WHITE);
	DrawText(TextFormat("Enemy State: %s", enemy.GetStateName()), 20, 70, 20, WHITE);
	DrawText(TextFormat("Chase: %d/%d", enemy.GetChaseMovesTaken(), enemy.GetChaseBudget()), 20, 90, 20, WHITE);
	DrawText(TextFormat("Punches: %d/%d", enemy.GetNormalAttackCount(), enemy.GetNormalAttacksPerCycle()), 20, 110, 20,
			 WHITE);
#endif
}

void PlayMode::EnemyHit() 
{
  minuteHand.Advance();
  minuteHand.activated = true;
  hourHand.activated = true;
}

void PlayMode::DrawEvilZone()
{
  if (minuteHand.activated == false) return;

  DrawCircleSector(minuteHand.GetPosition(), 1000, hourHand.GetAngle(), minuteHand.GetAngle(), 1, Color(255, 0, 0, 128));
}
