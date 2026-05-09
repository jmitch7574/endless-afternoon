#include "arena_manager.h"
#include "entity.h"
#include "keybinds.h"


struct PlayerTrail
{
	Vector2 pos;
	int opacity;
	float radius;
};

PlayerTrail trail[120] = {0};

Player::Player(raylib::Vector2 startPos) : Entity(startPos)
{
	gridPosition = startPos;
	position = ArenaManager::GridPositionToWorld(gridPosition);
}

Player::~Player(void) {}

void Player::Update()
{
	// Lerp Entity Position to Grid Position
	position = Vector2Lerp(position, ArenaManager::GridPositionToWorld(gridPosition), lerpSpeed);
	currentMoveCooldown -= GetFrameTime();

	Vector2 gridPositionLastFrame = gridPosition;
	if (IsKeyDown(MOVE_LEFT))
		TryMove(Vector2(-1, 0));
	if (IsKeyDown(MOVE_RIGHT))
		TryMove(Vector2(1, 0));
	if (IsKeyDown(MOVE_UP))
		TryMove(Vector2(0, -1));
	if (IsKeyDown(MOVE_DOWN))
		TryMove(Vector2(0, 1));

	if (!Vector2Equals(gridPosition, gridPositionLastFrame))
	{
		if (ArenaManager::IsValidGridPosition(ArenaManager::GridPositionToWorld(gridPosition)))
		{
			currentMoveCooldown = moveCooldown;
		}
		else
		{
			gridPosition = gridPositionLastFrame;
		}
	}

	for (int i = 119; i > 0; i--)
	{
		trail[i] = trail[i - 1];
		trail[i].radius -= 0.5f;
		trail[i].opacity -= 25;

		if (Vector2Equals(trail[i].pos, position))
		{
			trail[i].opacity = 0;
		}
	}

	trail[0] = {position, 255, CELL_SIZE / 4.0f};
}

void Player::Draw()
{
	for (int i = 119; i > 0; i--)
	{
		if (trail[i].opacity < 0)
			continue;

		int trail_radius = (CELL_SIZE / 2.0f) - ((255 - trail[i].opacity) / 10);
		DrawCircleV(trail[i].pos, CELL_SIZE / 2.0f, Color{50, 120, 160, (unsigned char)trail[i].opacity});
	}

	DrawCircleV(position, CELL_SIZE / 2.0f, SKYBLUE);
}

void Player::TryMove(Vector2 dir)
{
	if (currentMoveCooldown < 0)
	{
		gridPosition = Vector2Add(gridPosition, dir);
	}
}