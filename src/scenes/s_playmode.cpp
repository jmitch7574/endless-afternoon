#include "arena_manager.h"
#include "custom_draws.h"
#include "renderer.h"
#include "scene.h"
#include "scene_manager.h"
#include <algorithm>
#include <cmath>
#include <raylib-cpp.hpp>
#include "arena_manager.h"
#include "custom_draws.h"
#include "utils.h"
#include "danger_effects.h"

PlayMode *playScene;

PlayMode::PlayMode()
	: player(Vector2{10, 10}), enemy(Vector2{5, 5}), minuteHand(Vector2{960, 540}, -90.0f, 440.0f, 8.0f, WHITE),
	  hourHand(Vector2{960, 540}, -90.0f, 280.0f, 12.0f, WHITE), dangerEffects()
{
	playScene = this;
}
PlayMode::~PlayMode(void) { playScene = nullptr; }

void PlayMode::Update()
{
	if (victoryTriggered || gameOverTriggered)
	{
		resultTransitionTimer -= GetFrameTime();
		if (resultTransitionTimer <= 0.0f)
		{
			if (victoryTriggered)
			{
				g_SceneManager.SetScene(std::make_unique<VictoryScreen>());
			}
			else
			{
				g_SceneManager.SetScene(std::make_unique<GameOverScreen>());
			}
		}
		return;
	}

	player.Update();
	if (enemy.GetHealth() <= 0)
	{
		BeginVictory();
		return;
	}
	if (player.GetHealth() <= 0)
	{
		BeginGameOver();
		return;
	}

	if (enemy.isInGrid)
	{
		enemy.SetTargetGridPosition(player.gridPosition);
		enemy.Update();
	}
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
          player.Hurt(40, D_EvilZone);
          timeSinceEvilZoneTick = 0;
      }
      else if (timeSinceEvilZoneTick >= 1.0f)
      {
          player.Hurt(5, D_EvilZone);
          timeSinceEvilZoneTick = 0;
      }
  }

	if (enemy.GetHealth() <= 0)
	{
		BeginVictory();
	}
	else if (player.GetHealth() <= 0)
	{
		BeginGameOver();
	}

  dangerEffects.Update();
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
	if (!gameOverTriggered)
	{
		player.Draw();
	}
	if (enemy.isInGrid)
	{
		enemy.Draw();
	}
	DrawEnemyHealthBar();

	CustomDraws::DrawArrow(Vector2(400, 400), 0, 200, 10, 50, 90, GOLD);

  dangerEffects.Draw();

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

void PlayMode::EnemyHit(float damage) 
{
  enemy.Hurt(damage);
  minuteHand.Advance();
  minuteHand.activated = true;
  hourHand.activated = true;
}

void PlayMode::BeginVictory()
{
	if (victoryTriggered || gameOverTriggered)
	{
		return;
	}

	enemy.isInGrid = false;
	victoryTriggered = true;
	resultTransitionTimer = 0.6f;
}

void PlayMode::BeginGameOver()
{
	if (victoryTriggered || gameOverTriggered)
	{
		return;
	}

	gameOverTriggered = true;
	resultTransitionTimer = 0.6f;
}

void PlayMode::DrawEnemyHealthBar()
{
	const float barWidth = 760.0f;
	const float barHeight = 28.0f;
	const float barX = ((float)RENDER_TEXTURE_WIDTH - barWidth) * 0.5f;
	const float barY = 24.0f;
	const float healthPercent =
		enemy.GetMaxHealth() > 0 ? std::clamp(enemy.GetHealth() / (float)enemy.GetMaxHealth(), 0.0f, 1.0f) : 0.0f;

	const Rectangle background = {barX, barY, barWidth, barHeight};
	const Rectangle fill = {barX + 4.0f, barY + 4.0f, (barWidth - 8.0f) * healthPercent, barHeight - 8.0f};
	const Color clockOrange = Color{196, 116, 36, 255};
	const Color darkBacking = Color{22, 18, 14, 235};

	DrawRectangleRec(background, darkBacking);
	DrawRectangleRec(fill, clockOrange);
	DrawRectangleLinesEx(background, 3.0f, WHITE);

	const char *healthText = TextFormat("CLOCK  %.0f / %d", enemy.GetHealth(), enemy.GetMaxHealth());
	const int textSize = 22;
	const int textWidth = MeasureText(healthText, textSize);
	DrawText(healthText, (int)(barX + (barWidth - textWidth) * 0.5f), (int)(barY + 3.0f), textSize, WHITE);
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
