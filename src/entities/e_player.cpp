#include "entity.h"

Player::Player(raylib::Vector2 startPos) : Entity(startPos) {}
Player::~Player(void) {}

void Player::Update() {}
void Player::Draw() {
  DrawCircleLinesV(position, 16.0f, SKYBLUE);
}