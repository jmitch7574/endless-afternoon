#include "entity.h"

Enemy::Enemy(raylib::Vector2 startPos) : Entity(startPos) {}
Enemy::~Enemy(void) {}

void Enemy::Update() {}
void Enemy::Draw() {
  DrawCircleLinesV(position, 48.0f, ORANGE);
}