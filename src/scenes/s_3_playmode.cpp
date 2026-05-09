#include "arena_manager.h"
#include "renderer.h"
#include "scene.h"
#include "scene_manager.h"
#include <algorithm>
#include <cmath>
#include <raylib-cpp.hpp>
#include "utils.h"

namespace
{
constexpr float RED_LIGHT_TOTAL_DURATION = 10.0f;
constexpr float RED_LIGHT_APPLY_DURATION = 5.0f;
constexpr float RED_LIGHT_TELEGRAPH_DURATION = 1.0f;
constexpr float RED_LIGHT_WAVE_INTERVAL = 0.55f;
constexpr float RED_LIGHT_DAMAGE_INTERVAL = 0.5f;
constexpr float RED_LIGHT_DAMAGE = 5.0f;
constexpr int RED_LIGHT_CELLS_PER_WAVE = 5;

bool IsSameGridCell(Vector2 a, Vector2 b)
{
	return (int)a.x == (int)b.x && (int)a.y == (int)b.y;
}

Rectangle GridCellBounds(Vector2 gridPosition)
{
	const Vector2 worldPosition = ArenaManager::GridPositionToWorld(gridPosition);
	const float padding = 3.0f;
	return Rectangle{worldPosition.x - CELL_SIZE * 0.5f + padding, worldPosition.y - CELL_SIZE * 0.5f + padding,
					 CELL_SIZE - padding * 2.0f, CELL_SIZE - padding * 2.0f};
}
} // namespace

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
	UpdateRedLightGreenLight(GetFrameTime());

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
}
void PlayMode::Draw()
{
	ClearBackground(BLACK);
	ArenaManager::DrawLevelGrid();

	DrawRedLightGreenLight();
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
	UpdateBossPhaseFromHealth();
}

void PlayMode::StartRedLightGreenLight()
{
	redLightGreenLightActive = true;
	redLightGreenLightTimer = 0.0f;
	redLightNextWaveTimer = 0.0f;
	redLightDamageCooldown = 0.0f;
	redLightCells.clear();
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

void PlayMode::UpdateBossPhaseFromHealth()
{
	const float healthPercent =
		enemy.GetMaxHealth() > 0 ? enemy.GetHealth() / (float)enemy.GetMaxHealth() : 0.0f;

	int nextPhase = 0;
	if (healthPercent <= 0.25f)
	{
		nextPhase = 3;
	}
	else if (healthPercent <= 0.5f)
	{
		nextPhase = 2;
	}
	else if (healthPercent <= 0.75f)
	{
		nextPhase = 1;
	}

	while (bossHealthPhase < nextPhase)
	{
		bossHealthPhase++;
		minuteHand.Advance();
		minuteHand.activated = true;
		hourHand.activated = true;
	}
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

	for (int section = 1; section < 4; section++)
	{
		const float sectionX = barX + barWidth * (float)section / 4.0f;
		DrawLineEx(Vector2{sectionX, barY}, Vector2{sectionX, barY + barHeight}, 3.0f, WHITE);
	}

	const char *healthText = TextFormat("CLOCK  %.0f / %d", enemy.GetHealth(), enemy.GetMaxHealth());
	const int textSize = 22;
	const int textWidth = MeasureText(healthText, textSize);
	DrawText(healthText, (int)(barX + (barWidth - textWidth) * 0.5f), (int)(barY + 3.0f), textSize, WHITE);
}

void PlayMode::UpdateRedLightGreenLight(float deltaTime)
{
	if (!redLightGreenLightActive)
	{
		return;
	}

	redLightGreenLightTimer += deltaTime;
	redLightDamageCooldown -= deltaTime;

	if (redLightGreenLightTimer <= RED_LIGHT_APPLY_DURATION - RED_LIGHT_TELEGRAPH_DURATION)
	{
		redLightNextWaveTimer -= deltaTime;
		if (redLightNextWaveTimer <= 0.0f)
		{
			QueueRedLightCells();
			redLightNextWaveTimer += RED_LIGHT_WAVE_INTERVAL;
		}
	}

	for (RedLightCell &cell : redLightCells)
	{
		if (cell.telegraphTimer > 0.0f)
		{
			cell.telegraphTimer -= deltaTime;
		}
	}

	if (redLightDamageCooldown <= 0.0f && IsPlayerOnActiveRedLightCell())
	{
		player.Hurt(RED_LIGHT_DAMAGE);
		redLightDamageCooldown = RED_LIGHT_DAMAGE_INTERVAL;
	}

	if (redLightGreenLightTimer >= RED_LIGHT_TOTAL_DURATION)
	{
		redLightGreenLightActive = false;
		redLightCells.clear();
	}
}

void PlayMode::DrawRedLightGreenLight()
{
	if (!redLightGreenLightActive)
	{
		return;
	}

	float fade = 1.0f;
	if (redLightGreenLightTimer > RED_LIGHT_APPLY_DURATION)
	{
		fade = 1.0f - (redLightGreenLightTimer - RED_LIGHT_APPLY_DURATION) /
							 (RED_LIGHT_TOTAL_DURATION - RED_LIGHT_APPLY_DURATION);
		fade = std::clamp(fade, 0.0f, 1.0f);
	}

	for (int x = 0; x < CELL_COUNT; x++)
	{
		for (int y = 0; y < CELL_COUNT; y++)
		{
			const Vector2 gridPosition = Vector2{(float)x, (float)y};
			if (!ArenaManager::IsValidGridPosition(ArenaManager::GridPositionToWorld(gridPosition)))
			{
				continue;
			}

			DrawRectangleRec(GridCellBounds(gridPosition), Color{35, 175, 95, (unsigned char)(58 * fade)});
		}
	}

	for (const RedLightCell &cell : redLightCells)
	{
		const Rectangle bounds = GridCellBounds(cell.gridPosition);
		if (cell.telegraphTimer > 0.0f)
		{
			const float pulse = 0.45f + 0.55f * sinf((float)GetTime() * 12.0f);
			DrawRectangleRec(bounds, Color{255, 188, 48, (unsigned char)((80 + 90 * pulse) * fade)});
			DrawRectangleLinesEx(bounds, 3.0f, Color{255, 70, 52, (unsigned char)(230 * fade)});
		}
		else
		{
			DrawRectangleRec(bounds, Color{220, 42, 38, (unsigned char)(185 * fade)});
			DrawRectangleLinesEx(bounds, 2.0f, Color{255, 228, 210, (unsigned char)(90 * fade)});
		}
	}
}

void PlayMode::QueueRedLightCells()
{
	int cellsQueued = 0;
	int attempts = 0;

	while (cellsQueued < RED_LIGHT_CELLS_PER_WAVE && attempts < 100)
	{
		attempts++;
		const Vector2 candidate = Vector2{(float)GetRandomValue(0, CELL_COUNT - 1), (float)GetRandomValue(0, CELL_COUNT - 1)};

		if (!ArenaManager::IsValidGridPosition(ArenaManager::GridPositionToWorld(candidate)) ||
			enemy.OccupiesGridPosition(candidate) || IsRedLightCellQueued(candidate))
		{
			continue;
		}

		redLightCells.push_back(RedLightCell{candidate, RED_LIGHT_TELEGRAPH_DURATION});
		cellsQueued++;
	}
}

bool PlayMode::IsRedLightCellQueued(Vector2 gridPosition) const
{
	for (const RedLightCell &cell : redLightCells)
	{
		if (IsSameGridCell(cell.gridPosition, gridPosition))
		{
			return true;
		}
	}

	return false;
}

bool PlayMode::IsPlayerOnActiveRedLightCell() const
{
	for (const RedLightCell &cell : redLightCells)
	{
		if (cell.telegraphTimer <= 0.0f && IsSameGridCell(cell.gridPosition, player.gridPosition))
		{
			return true;
		}
	}

	return false;
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
