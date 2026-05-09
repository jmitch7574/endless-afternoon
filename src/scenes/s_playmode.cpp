#include "arena_manager.h"
#include "custom_draws.h"
#include "scene.h"
#include <cmath>
#include <raylib-cpp.hpp>
#include "arena_manager.h"
#include "custom_draws.h"
#include "utils.h"

PlayMode *playScene;

PlayMode::PlayMode()
	: player(Vector2{10, 10}), enemy(Vector2{5, 5}), minuteHand(Vector2{960, 540}, -90.0f, 440.0f, 8.0f, WHITE),
	  hourHand(Vector2{960, 540}, -90.0f, 280.0f, 12.0f, WHITE)
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

  float playerAngleToCenter = Utils::NormalizeAngle(Utils::Vector2ToAngle(Vector2Normalize(Vector2Subtract(player.GetPosition(),minuteHand.GetPosition()))));

  float hourAngle = Utils::NormalizeAngle(hourHand.GetAngle());
  float minuteAngle = Utils::NormalizeAngle(minuteHand.GetAngle());

  bool collision;

  if (hourAngle <= minuteAngle)
  {
    collision = minuteAngle > playerAngleToCenter && playerAngleToCenter > hourAngle;
  }
  else
  {
    collision = minuteAngle > playerAngleToCenter || playerAngleToCenter > hourAngle;
  }

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
  DrawText(TextFormat("Hour Angle: %f", Utils::NormalizeAngle(hourHand.GetAngle())), 20, 140, 20, WHITE);
  DrawText(TextFormat("Minute Angle: %f", Utils::NormalizeAngle(minuteHand.GetAngle())), 20, 170, 20, WHITE);
  DrawText(TextFormat("Player Angle: %f", Utils::NormalizeAngle(Utils::Vector2ToAngle(Vector2Normalize(Vector2Subtract(player.GetPosition(),minuteHand.GetPosition()))))), 20, 200, 20, WHITE);
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

  float hourAngle   = Utils::NormalizeAngle(hourHand.GetAngle());
  float minuteAngle = Utils::NormalizeAngle(minuteHand.GetAngle());
  Color red = Color(255, 0, 0, 128);

  if (hourAngle <= minuteAngle)
  {
      // Simple case: no wrap
      DrawCircleSector(minuteHand.GetPosition(), 1000, hourAngle, minuteAngle, 1, red);
  }
  else
  {
      // Wrapping case: draw from hourAngle to 360, then 0 to minuteAngle
      DrawCircleSector(minuteHand.GetPosition(), 1000, hourAngle,   360.0f,    1, red);
      DrawCircleSector(minuteHand.GetPosition(), 1000, 0.0f,        minuteAngle, 1, red);
  }
}
