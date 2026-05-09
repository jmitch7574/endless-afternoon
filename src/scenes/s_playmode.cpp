#include "arena_manager.h"
#include "custom_draws.h"
#include "scene.h"
#include <cmath>
#include <raylib-cpp.hpp>

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
}
void PlayMode::Draw()
{
	ClearBackground(BLACK);
	ArenaManager::DrawLevelGrid();
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
	DrawText(TextFormat("Enemy State: %s", enemy.GetStateName()), 20, 45, 20, WHITE);
	DrawText(TextFormat("Chase: %d/%d", enemy.GetChaseMovesTaken(), enemy.GetChaseBudget()), 20, 70, 20, WHITE);
	DrawText(TextFormat("Punches: %d/%d", enemy.GetNormalAttackCount(), enemy.GetNormalAttacksPerCycle()), 20, 95, 20,
			 WHITE);
#endif
}
