#pragma once
#include <functional>
#include <raylib-cpp.hpp>
#include <vector>

class Entity {
protected:
  raylib::Texture2D texture;
  raylib::Vector2 position;

  // Grid-based movement state, shared by every entity that moves on the
  // board. Non-moving entities just leave these at their defaults.
  Vector2 gridPosition;
  float lerpSpeed = 0.2f;
  float moveCooldown = 0.25f;
  float currentMoveCooldown = 0;

public:
  Entity() = default;
  Entity(raylib::Vector2 startPos) : position(startPos) {}
  virtual ~Entity() = default;

  virtual void Update() = 0;
  virtual void Draw() = 0;
};

class Player : public Entity {
public:
  Player(raylib::Vector2 startPos);
  ~Player(void);

  void TryMove(Vector2 dir);

  void Update() override;
  void Draw() override;
  Vector2 gridPosition;

protected:
  float lerpSpeed = 0.2f;
  float moveCooldown = 0.25f;
  float currentMoveCooldown = 0;
};

class Enemy : public Entity {
public:
  Enemy(raylib::Vector2 startPos);
  ~Enemy(void);

  void Update() override;
  void Draw() override;

  // Trigger the next attack — fires a normal until the cycle's normals are
  // exhausted, then a special. After each special, advance to the next one
  // in `specialAttacks`, wrapping at the end.
  void PerformNextAttack();
  
  // Is the enemy within grid space or float space
  bool isInGrid;

  Rectangle GetBBoxWorld();
private:
  void NormalAttack();
  void SpecialAttackOne();
  void SpecialAttackTwo();

  // Specials fire in the order they appear in this list, then wrap. To add
  // another, declare/define the method and push a lambda for it onto this
  // vector in the constructor — nothing else needs to change.
  std::vector<std::function<void()>> specialAttacks;
  size_t nextSpecialIndex = 0;

  int normalAttacksPerCycle = 3;
  int normalAttacksRemaining; // initialised from normalAttacksPerCycle in ctor
};

class ClockHand : public Entity {
public:
  ClockHand(raylib::Vector2 pivot, float angleDeg, float length,
            float thickness, Color color);
  ~ClockHand(void);

  void Update() override;
  void Draw() override;

protected:
  float angleDeg;
  float length;
  float thickness;
  Color color;
};
