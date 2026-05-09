#include "entity.h"
#include "grid_manager.h"

Enemy::Enemy(raylib::Vector2 startPos)
    : Entity(startPos), normalAttacksRemaining(normalAttacksPerCycle) {
  // Register the special attacks the enemy will cycle through. Add another
  // by appending another lambda — PerformNextAttack handles the rotation.
  specialAttacks = {
      [this]() { SpecialAttackOne(); },
      [this]() { SpecialAttackTwo(); },
  };
}
Enemy::~Enemy(void) {}

void Enemy::Update() {}
void Enemy::Draw() { 
  DrawCircleLinesV(position, CELL_SIZE * 1.5, ORANGE);
}

void Enemy::PerformNextAttack() {
  if (normalAttacksRemaining > 0) {
    NormalAttack();
    normalAttacksRemaining--;
    return;
  }

  // Cycle's normals exhausted — fire the next special and advance the index.
  if (!specialAttacks.empty()) {
    specialAttacks[nextSpecialIndex]();
    nextSpecialIndex = (nextSpecialIndex + 1) % specialAttacks.size();
  }
  normalAttacksRemaining = normalAttacksPerCycle;
}

void Enemy::NormalAttack() {}

void Enemy::SpecialAttackOne() {}

void Enemy::SpecialAttackTwo() {}

Rectangle Enemy::GetBBoxWorld()
{
  return Rectangle(position.x - CELL_SIZE * 1.5, position.y - CELL_SIZE * 1.5, CELL_SIZE * 3, CELL_SIZE * 3);
}
