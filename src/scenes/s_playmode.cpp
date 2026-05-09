#include "arena_manager.h"
#include "scene.h"
#include <cmath>
#include <raylib-cpp.hpp>
#include "arena_manager.h"
#include "custom_draws.h"

PlayMode* playScene;

PlayMode::PlayMode()
    : player(Vector2{10, 10}), enemy(Vector2{1085, 415}),
      minuteHand(Vector2{960, 540}, -90.0f, 440.0f, 8.0f, WHITE),
      hourHand(Vector2{960, 540}, 0.0f, 280.0f, 12.0f, WHITE) {
        playScene = this;
      }
PlayMode::~PlayMode(void) {playScene = nullptr;}

void PlayMode::Update()
{
	player.Update();
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
#endif
}
