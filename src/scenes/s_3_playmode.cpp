#include "arena_manager.h"
#include "entities/entity.h"
#include "renderer.h"
#include "scene.h"
#include "scene_manager.h"
#include "utils.h"
#include <algorithm>
#include <cmath>
#include <raylib-cpp.hpp>

namespace
{
constexpr float RED_LIGHT_TOTAL_DURATION = 11.5f;
constexpr float RED_LIGHT_APPLY_DURATION = 7.0f;
constexpr float RED_LIGHT_TELEGRAPH_DURATION = 0.9f;
constexpr float RED_LIGHT_ACTIVE_DURATION = 3.2f;
constexpr float RED_LIGHT_FADE_DURATION = 1.8f;
constexpr float RED_LIGHT_WAVE_INTERVAL = 0.45f;
constexpr float RED_LIGHT_PLAYER_TARGET_INTERVAL = 1.35f;
constexpr float RED_LIGHT_DAMAGE_INTERVAL = 1.0f;
constexpr float RED_LIGHT_DAMAGE = 5.0f;
constexpr float EVIL_ZONE_DAMAGE_INTERVAL = 1.0f;
constexpr float EVIL_ZONE_DAMAGE = 5.0f;
constexpr float MOVING_CLOCK_HAND_IMPACT_DAMAGE = 20.0f;
constexpr float ROMAN_NUMERAL_ATTACK_FLASH_DURATION = 1.2f;
constexpr float ROMAN_NUMERAL_ATTACK_FIRE_DURATION = 2.2f;
constexpr float ROMAN_NUMERAL_ATTACK_HIDDEN_DURATION = 0.25f;
constexpr float ROMAN_NUMERAL_ATTACK_FADE_DURATION = 0.9f;
constexpr float ROMAN_NUMERAL_ATTACK_DAMAGE = 20.0f;
constexpr float ROMAN_NUMERAL_ATTACK_COLLISION_RADIUS = 36.0f;
constexpr float ROMAN_NUMERAL_ATTACK_TRAVEL_DISTANCE = (float)RENDER_TEXTURE_WIDTH * 1.45f;
constexpr float CLOCK_HAND_HUB_RADIUS = 18.0f;
constexpr float CLOCK_HAND_HUB_INNER_RADIUS = 8.0f;
constexpr float RESOURCE_BAR_TOTAL_WIDTH = 760.0f;
constexpr float RESOURCE_BAR_GAP = 40.0f;
constexpr float RESOURCE_BAR_WIDTH = (RESOURCE_BAR_TOTAL_WIDTH - RESOURCE_BAR_GAP) * 0.5f;
constexpr float RESOURCE_BAR_HEIGHT = 26.0f;
constexpr float RESOURCE_BAR_X = ((float)RENDER_TEXTURE_WIDTH - RESOURCE_BAR_TOTAL_WIDTH) * 0.5f;
constexpr float PLAYER_RESOURCE_BAR_Y = (float)RENDER_TEXTURE_HEIGHT - 58.0f;
constexpr float RESOURCE_BAR_LERP_SPEED = 9.0f;
constexpr int RED_LIGHT_CELLS_PER_WAVE = 7;
constexpr Color CLOCK_HAND_HUB_ORANGE = Color{196, 116, 36, 255};
constexpr Color CLOCK_ORANGE = Color{196, 116, 36, 255};

bool IsSameGridCell(Vector2 a, Vector2 b) { return (int)a.x == (int)b.x && (int)a.y == (int)b.y; }

Color LerpColor(Color from, Color to, float amount)
{
	amount = std::clamp(amount, 0.0f, 1.0f);
	return Color{(unsigned char)((float)from.r + ((float)to.r - (float)from.r) * amount),
				 (unsigned char)((float)from.g + ((float)to.g - (float)from.g) * amount),
				 (unsigned char)((float)from.b + ((float)to.b - (float)from.b) * amount),
				 (unsigned char)((float)from.a + ((float)to.a - (float)from.a) * amount)};
}

Rectangle GridCellBounds(Vector2 gridPosition)
{
	const Vector2 worldPosition = ArenaManager::GridPositionToWorld(gridPosition);
	const float padding = 3.0f;
	return Rectangle{worldPosition.x - CELL_SIZE * 0.5f + padding, worldPosition.y - CELL_SIZE * 0.5f + padding,
					 CELL_SIZE - padding * 2.0f, CELL_SIZE - padding * 2.0f};
}

void DrawClockHandHub(Vector2 position, bool activated)
{
	const Color outerColor = activated ? WHITE : Color{80, 80, 80, 255};
	const Color innerColor = activated ? CLOCK_HAND_HUB_ORANGE : Color{45, 45, 45, 255};

	DrawCircleV(position, CLOCK_HAND_HUB_RADIUS, outerColor);
	DrawCircleV(position, CLOCK_HAND_HUB_INNER_RADIUS, innerColor);
}

float LerpBarValue(float displayedValue, float targetValue, float deltaTime)
{
	const float lerpAmount = std::clamp(deltaTime * RESOURCE_BAR_LERP_SPEED, 0.0f, 1.0f);
	return displayedValue + (targetValue - displayedValue) * lerpAmount;
}

void DrawResourceBar(Rectangle bounds, float displayedValue, float actualValue, float maxValue, const char *label,
					 Color fillColor)
{
	const float percent = maxValue > 0.0f ? std::clamp(displayedValue / maxValue, 0.0f, 1.0f) : 0.0f;
	const Rectangle fill = {bounds.x + 4.0f, bounds.y + 4.0f, (bounds.width - 8.0f) * percent, bounds.height - 8.0f};
	const Color darkBacking = Color{22, 18, 14, 235};

	DrawRectangleRec(bounds, darkBacking);
	DrawRectangleRec(fill, fillColor);
	DrawRectangleLinesEx(bounds, 3.0f, WHITE);

	const char *text = TextFormat("%s  %.0f / %.0f", label, actualValue, maxValue);
	const int textSize = 20;
	const int textWidth = MeasureText(text, textSize);
	DrawText(text, (int)(bounds.x + (bounds.width - textWidth) * 0.5f), (int)(bounds.y + 3.0f), textSize, WHITE);
}
} // namespace

PlayMode *playScene;

PlayMode::PlayMode()
	: player(Vector2{10, 10}), enemy(Vector2{5, 5}), minuteHand(Vector2{960, 540}, -90.0f, 440.0f, 8.0f, WHITE),
	  hourHand(Vector2{960, 540}, -90.0f, 280.0f, 12.0f, WHITE), dangerEffects()
{
	playScene = this;
	displayedEnemyHealth = enemy.GetHealth();
	displayedPlayerHealth = player.GetHealth();
	displayedPlayerStamina = player.GetStamina();
}
PlayMode::~PlayMode(void) { playScene = nullptr; }

void PlayMode::Update()
{
	const float deltaTime = GetFrameTime();

	if (victoryTriggered || gameOverTriggered)
	{
		resultTransitionTimer -= deltaTime;
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
	UpdateRedLightGreenLight(deltaTime);
	UpdateEvilZone(deltaTime);
	UpdateMovingClockHandImpact();
	UpdateRomanNumeralAttack(deltaTime);
	UpdateBarDisplays(deltaTime);

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

	DrawRedLightGreenLight();
	DrawEvilZone();

	ArenaManager::MaskOutsideOctagon();
	ArenaManager::DrawOctagonBoundary();
	DrawRomanNumerals();
	hourHand.Draw();
	minuteHand.Draw();
	DrawClockHandHub(minuteHand.GetPosition(), minuteHand.activated || hourHand.activated);
	if (!gameOverTriggered)
	{
		player.Draw();
	}
	if (enemy.isInGrid)
	{
		enemy.Draw();
	}
	DrawEnemyHealthBar();
	DrawPlayerBars();
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
	DrawText(TextFormat("Player Angle: %f", Utils::NormalizeAngle(Utils::Vector2ToAngle(Vector2Normalize(
												Vector2Subtract(player.GetPosition(), minuteHand.GetPosition()))))),
			 20, 200, 20, WHITE);
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
	redLightPlayerTargetTimer = 0.0f;
	redLightDamageCooldown = 0.0f;
	redLightWaveCount = 0;
	redLightCells.clear();
}

void PlayMode::StartRomanNumeralAttack()
{
	romanNumeralAttackState = RomanNumeralAttackState::Flash;
	romanNumeralAttackTimer = 0.0f;
	romanNumeralMarkers.clear();

	for (int hour = 1; hour <= 12; hour++)
	{
		const Vector2 startPosition = ArenaManager::ClockMarkerPosition(hour);
		const Vector2 inwardDirection = Vector2Scale(ArenaManager::ClockMarkerOutwardDirection(hour), -1.0f);

		romanNumeralMarkers.push_back(
			RomanNumeralMarker{hour, startPosition, startPosition, startPosition, inwardDirection, true, false});
	}
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

void PlayMode::UpdateBarDisplays(float deltaTime)
{
	displayedEnemyHealth = LerpBarValue(displayedEnemyHealth, enemy.GetHealth(), deltaTime);
	displayedPlayerHealth = LerpBarValue(displayedPlayerHealth, player.GetHealth(), deltaTime);
	displayedPlayerStamina = LerpBarValue(displayedPlayerStamina, player.GetStamina(), deltaTime);
}

void PlayMode::UpdateEvilZone(float deltaTime)
{
	if (!minuteHand.activated)
	{
		timeSinceEvilZoneTick = 0.0f;
		return;
	}

	const Vector2 playerOffset = Vector2Subtract(player.GetPosition(), minuteHand.GetPosition());
	if (Vector2Length(playerOffset) <= 0.0f)
	{
		return;
	}

	const float playerAngleToCenter = Utils::NormalizeAngle(Utils::Vector2ToAngle(Vector2Normalize(playerOffset)));
	const float hourAngle = Utils::NormalizeAngle(hourHand.GetAngle());
	const float minuteAngle = Utils::NormalizeAngle(minuteHand.GetAngle());
	bool collision = false;

	if (hourAngle <= minuteAngle)
	{
		collision = minuteAngle > playerAngleToCenter && playerAngleToCenter > hourAngle;
	}
	else
	{
		collision = minuteAngle > playerAngleToCenter || playerAngleToCenter > hourAngle;
	}

	if (!collision)
	{
		timeSinceEvilZoneTick = 0.0f;
		return;
	}

	timeSinceEvilZoneTick += deltaTime;
	if (timeSinceEvilZoneTick >= EVIL_ZONE_DAMAGE_INTERVAL)
	{
		player.Hurt(EVIL_ZONE_DAMAGE, D_EvilZone);
		timeSinceEvilZoneTick -= EVIL_ZONE_DAMAGE_INTERVAL;
	}
}

void PlayMode::UpdateMovingClockHandImpact()
{
	const float playerRadius = CELL_SIZE * 0.5f;
	bool touchingMovingHand = false;

	if (minuteHand.IsMoving())
	{
		touchingMovingHand = touchingMovingHand ||
							 Utils::LineIntersectsCircle(minuteHand.GetPosition(), minuteHand.GetLargeExtendedPoint(),
														 player.GetPosition(), playerRadius);
	}

	if (hourHand.IsMoving())
	{
		touchingMovingHand =
			touchingMovingHand || Utils::LineIntersectsCircle(hourHand.GetPosition(), hourHand.GetLargeExtendedPoint(),
															  player.GetPosition(), playerRadius);
	}

	if (touchingMovingHand && !playerTouchingMovingClockHand)
	{
		player.Hurt(MOVING_CLOCK_HAND_IMPACT_DAMAGE, D_EvilZone);
	}

	playerTouchingMovingClockHand = touchingMovingHand;
}

void PlayMode::UpdateBossPhaseFromHealth()
{
	const float healthPercent = enemy.GetMaxHealth() > 0 ? enemy.GetHealth() / (float)enemy.GetMaxHealth() : 0.0f;

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
	const float barWidth = RESOURCE_BAR_TOTAL_WIDTH;
	const float barHeight = 28.0f;
	const float barX = RESOURCE_BAR_X;
	const float barY = 24.0f;
	const float healthPercent =
		enemy.GetMaxHealth() > 0 ? std::clamp(displayedEnemyHealth / (float)enemy.GetMaxHealth(), 0.0f, 1.0f) : 0.0f;

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

void PlayMode::DrawPlayerBars()
{
	const Rectangle healthBounds = {RESOURCE_BAR_X, PLAYER_RESOURCE_BAR_Y, RESOURCE_BAR_WIDTH, RESOURCE_BAR_HEIGHT};
	const Rectangle staminaBounds = {RESOURCE_BAR_X + RESOURCE_BAR_WIDTH + RESOURCE_BAR_GAP, PLAYER_RESOURCE_BAR_Y,
									 RESOURCE_BAR_WIDTH, RESOURCE_BAR_HEIGHT};

	DrawResourceBar(healthBounds, displayedPlayerHealth, player.GetHealth(), player.GetMaxHealth(), "HEALTH",
					Color{194, 55, 55, 255});
	DrawResourceBar(staminaBounds, displayedPlayerStamina, player.GetStamina(), player.GetMaxStamina(), "STAMINA",
					Color{64, 155, 225, 255});
}

void PlayMode::UpdateRedLightGreenLight(float deltaTime)
{
	if (!redLightGreenLightActive)
	{
		return;
	}

	redLightGreenLightTimer += deltaTime;
	redLightPlayerTargetTimer -= deltaTime;
	redLightDamageCooldown -= deltaTime;

	if (redLightGreenLightTimer <= RED_LIGHT_APPLY_DURATION)
	{
		redLightNextWaveTimer -= deltaTime;
		if (redLightNextWaveTimer <= 0.0f)
		{
			QueueRedLightCells();
			redLightNextWaveTimer += RED_LIGHT_WAVE_INTERVAL;
		}
	}

	if (redLightGreenLightTimer <= RED_LIGHT_APPLY_DURATION && redLightPlayerTargetTimer <= 0.0f)
	{
		QueueRedLightCell(player.gridPosition);
		redLightPlayerTargetTimer = RED_LIGHT_PLAYER_TARGET_INTERVAL;
	}

	for (RedLightCell &cell : redLightCells)
	{
		if (cell.telegraphTimer > 0.0f)
		{
			cell.telegraphTimer -= deltaTime;
		}
		else if (cell.activeTimer > 0.0f)
		{
			cell.activeTimer -= deltaTime;
		}
		else
		{
			cell.fadeTimer -= deltaTime;
		}
	}

	redLightCells.erase(
		std::remove_if(redLightCells.begin(), redLightCells.end(), [](const RedLightCell &cell)
					   { return cell.telegraphTimer <= 0.0f && cell.activeTimer <= 0.0f && cell.fadeTimer <= 0.0f; }),
		redLightCells.end());

	if (redLightDamageCooldown <= 0.0f && IsPlayerOnActiveRedLightCell())
	{
		player.Hurt(RED_LIGHT_DAMAGE, D_EvilZone);
		redLightDamageCooldown = RED_LIGHT_DAMAGE_INTERVAL;
	}

	if (redLightGreenLightTimer >= RED_LIGHT_TOTAL_DURATION && redLightCells.empty())
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

	const float arenaFade =
		redLightGreenLightTimer <= RED_LIGHT_TOTAL_DURATION
			? 1.0f
			: std::clamp(1.0f - (redLightGreenLightTimer - RED_LIGHT_TOTAL_DURATION) / RED_LIGHT_FADE_DURATION, 0.0f,
						 1.0f);

	for (int x = 0; x < CELL_COUNT; x++)
	{
		for (int y = 0; y < CELL_COUNT; y++)
		{
			const Vector2 gridPosition = Vector2{(float)x, (float)y};
			if (!ArenaManager::IsValidGridPosition(ArenaManager::GridPositionToWorld(gridPosition)))
			{
				continue;
			}

			DrawRectangleRec(GridCellBounds(gridPosition), Color{35, 175, 95, (unsigned char)(58 * arenaFade)});
		}
	}

	for (const RedLightCell &cell : redLightCells)
	{
		const Rectangle bounds = GridCellBounds(cell.gridPosition);
		if (cell.telegraphTimer > 0.0f)
		{
			const float pulse = 0.45f + 0.55f * sinf((float)GetTime() * 12.0f);
			DrawRectangleRec(bounds, Color{255, 188, 48, (unsigned char)(80 + 90 * pulse)});
			DrawRectangleLinesEx(bounds, 3.0f, Color{255, 70, 52, 230});
		}
		else if (cell.activeTimer > 0.0f)
		{
			DrawRectangleRec(bounds, Color{220, 42, 38, 190});
			DrawRectangleLinesEx(bounds, 2.0f, Color{255, 228, 210, 95});
		}
		else
		{
			const float fade = std::clamp(cell.fadeTimer / RED_LIGHT_FADE_DURATION, 0.0f, 1.0f);
			DrawRectangleRec(bounds, Color{220, 42, 38, (unsigned char)(135 * fade)});
			DrawRectangleLinesEx(bounds, 2.0f, Color{255, 228, 210, (unsigned char)(70 * fade)});
		}
	}
}

void PlayMode::QueueRedLightCells()
{
	redLightWaveCount++;
	int cellsQueued = 0;
	int attempts = 0;
	const int pattern = redLightWaveCount % 3;

	if (pattern == 0)
	{
		const bool horizontal = GetRandomValue(0, 1) == 0;
		for (int offset = -3; offset <= 3; offset++)
		{
			const Vector2 candidate = horizontal
										  ? Vector2{player.gridPosition.x + (float)offset, player.gridPosition.y}
										  : Vector2{player.gridPosition.x, player.gridPosition.y + (float)offset};
			const int beforeCount = (int)redLightCells.size();
			QueueRedLightCell(candidate);
			if ((int)redLightCells.size() > beforeCount)
			{
				cellsQueued++;
			}
		}
	}
	else if (pattern == 1)
	{
		const Vector2 center = Vector2{player.gridPosition.x + (float)GetRandomValue(-3, 3),
									   player.gridPosition.y + (float)GetRandomValue(-3, 3)};
		const Vector2 offsets[] = {Vector2{0, 0},  Vector2{1, 0}, Vector2{-1, 0}, Vector2{0, 1},
								   Vector2{0, -1}, Vector2{1, 1}, Vector2{-1, -1}};

		for (const Vector2 &offset : offsets)
		{
			const int beforeCount = (int)redLightCells.size();
			QueueRedLightCell(Vector2Add(center, offset));
			if ((int)redLightCells.size() > beforeCount)
			{
				cellsQueued++;
			}
		}
	}

	while (cellsQueued < RED_LIGHT_CELLS_PER_WAVE && attempts < 100)
	{
		attempts++;
		const Vector2 candidate =
			Vector2{(float)GetRandomValue(0, CELL_COUNT - 1), (float)GetRandomValue(0, CELL_COUNT - 1)};

		const int beforeCount = (int)redLightCells.size();
		QueueRedLightCell(candidate);
		if ((int)redLightCells.size() > beforeCount)
		{
			cellsQueued++;
		}
	}
}

void PlayMode::QueueRedLightCell(Vector2 gridPosition)
{
	if (!ArenaManager::IsValidGridPosition(ArenaManager::GridPositionToWorld(gridPosition)) ||
		enemy.OccupiesGridPosition(gridPosition) || IsRedLightCellQueued(gridPosition))
	{
		return;
	}

	redLightCells.push_back(
		RedLightCell{gridPosition, RED_LIGHT_TELEGRAPH_DURATION, RED_LIGHT_ACTIVE_DURATION, RED_LIGHT_FADE_DURATION});
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
		if (cell.telegraphTimer <= 0.0f && cell.activeTimer > 0.0f &&
			IsSameGridCell(cell.gridPosition, player.gridPosition))
		{
			return true;
		}
	}

	return false;
}

void PlayMode::UpdateRomanNumeralAttack(float deltaTime)
{
	if (romanNumeralAttackState == RomanNumeralAttackState::Inactive)
	{
		return;
	}

	romanNumeralAttackTimer += deltaTime;

	switch (romanNumeralAttackState)
	{
	case RomanNumeralAttackState::Flash:
		if (romanNumeralAttackTimer >= ROMAN_NUMERAL_ATTACK_FLASH_DURATION)
		{
			romanNumeralAttackState = RomanNumeralAttackState::Fire;
			romanNumeralAttackTimer = 0.0f;
			for (RomanNumeralMarker &marker : romanNumeralMarkers)
			{
				marker.previousPosition = marker.position;
				marker.hasHit = false;
				marker.visible = true;
			}
		}
		break;
	case RomanNumeralAttackState::Fire:
	{
		const float progress = std::clamp(romanNumeralAttackTimer / ROMAN_NUMERAL_ATTACK_FIRE_DURATION, 0.0f, 1.0f);
		const float easedProgress = progress * progress * (3.0f - 2.0f * progress);

		for (RomanNumeralMarker &marker : romanNumeralMarkers)
		{
			marker.previousPosition = marker.position;
			marker.position =
				Vector2Add(marker.startPosition,
						   Vector2Scale(marker.direction, ROMAN_NUMERAL_ATTACK_TRAVEL_DISTANCE * easedProgress));

			if (!marker.hasHit &&
				Utils::LineIntersectsCircle(marker.previousPosition, marker.position, player.GetPosition(),
											CELL_SIZE * 0.45f + ROMAN_NUMERAL_ATTACK_COLLISION_RADIUS))
			{
				player.Hurt(ROMAN_NUMERAL_ATTACK_DAMAGE, D_Enemy);
				marker.hasHit = true;
			}
		}

		if (romanNumeralAttackTimer >= ROMAN_NUMERAL_ATTACK_FIRE_DURATION)
		{
			romanNumeralAttackState = RomanNumeralAttackState::Hidden;
			romanNumeralAttackTimer = 0.0f;
			for (RomanNumeralMarker &marker : romanNumeralMarkers)
			{
				marker.position = marker.startPosition;
				marker.previousPosition = marker.startPosition;
				marker.visible = false;
			}
		}
		break;
	}
	case RomanNumeralAttackState::Hidden:
		if (romanNumeralAttackTimer >= ROMAN_NUMERAL_ATTACK_HIDDEN_DURATION)
		{
			romanNumeralAttackState = RomanNumeralAttackState::FadeIn;
			romanNumeralAttackTimer = 0.0f;
			for (RomanNumeralMarker &marker : romanNumeralMarkers)
			{
				marker.visible = true;
			}
		}
		break;
	case RomanNumeralAttackState::FadeIn:
		if (romanNumeralAttackTimer >= ROMAN_NUMERAL_ATTACK_FADE_DURATION)
		{
			romanNumeralAttackState = RomanNumeralAttackState::Inactive;
			romanNumeralAttackTimer = 0.0f;
			romanNumeralMarkers.clear();
		}
		break;
	case RomanNumeralAttackState::Inactive:
		break;
	}
}

void PlayMode::DrawRomanNumerals()
{
	if (romanNumeralAttackState == RomanNumeralAttackState::Inactive)
	{
		ArenaManager::DrawClockMarkers();
		return;
	}

	DrawRomanNumeralAttack();
}

void PlayMode::DrawRomanNumeralAttack()
{
	Color markerColor = WHITE;

	if (romanNumeralAttackState == RomanNumeralAttackState::Flash)
	{
		const float pulse = 0.5f + 0.5f * sinf((float)GetTime() * 18.0f);
		markerColor = LerpColor(WHITE, CLOCK_ORANGE, pulse);
	}
	else if (romanNumeralAttackState == RomanNumeralAttackState::Fire)
	{
		markerColor = CLOCK_ORANGE;
	}
	else if (romanNumeralAttackState == RomanNumeralAttackState::FadeIn)
	{
		const float fade = std::clamp(romanNumeralAttackTimer / ROMAN_NUMERAL_ATTACK_FADE_DURATION, 0.0f, 1.0f);
		markerColor = Color{255, 255, 255, (unsigned char)(255.0f * fade)};
	}

	for (const RomanNumeralMarker &marker : romanNumeralMarkers)
	{
		if (marker.visible)
		{
			ArenaManager::DrawClockMarker(marker.hour, marker.position, markerColor);
		}
	}
}

void PlayMode::DrawEvilZone()
{
	if (minuteHand.activated == false)
		return;

	float hourAngle = Utils::NormalizeAngle(hourHand.GetAngle());
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
		DrawCircleSector(minuteHand.GetPosition(), 1000, hourAngle, 360.0f, 1, red);
		DrawCircleSector(minuteHand.GetPosition(), 1000, 0.0f, minuteAngle, 1, red);
	}
}
