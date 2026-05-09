#include "entity.h"
#include "grid_manager.h"

Player::Player(raylib::Vector2 startPos) : Entity(startPos) {
  gridPosition = startPos;
  position = GridManager::GridPositionToWorld(gridPosition);
}

Player::~Player(void) {}

void Player::Update() {
  // Lerp Entity Position to Grid Position
  position = Vector2Lerp(position, GridManager::GridPositionToWorld(gridPosition), lerpSpeed);
  currentMoveCooldown -= GetFrameTime();

  if (IsKeyDown(KEY_LEFT)) TryMove(Vector2(-1, 0));
  if (IsKeyDown(KEY_RIGHT)) TryMove(Vector2(1, 0));
  if (IsKeyDown(KEY_UP)) TryMove(Vector2(0, -1));
  if (IsKeyDown(KEY_DOWN)) TryMove(Vector2(0, 1));
}

void Player::Draw() {
  DrawCircleV(position, CELL_SIZE / 2.0f, SKYBLUE);
}

void Player::TryMove(Vector2 dir) {
  if (currentMoveCooldown < 0) {
    currentMoveCooldown = moveCooldown;
    gridPosition = Vector2Add(gridPosition, dir);
  }
}