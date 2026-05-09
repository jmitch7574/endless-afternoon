#pragma once
#include <raylib-cpp.hpp>

class Entity
{
  protected:
	raylib::Vector2 position;
	float health = 0.0f;

  public:
	Entity() = default;
	Entity(raylib::Vector2 startPos) : position(startPos) {}
	Vector2 GetPosition() const { return position; };
	float GetHealth() const { return health; };
	virtual ~Entity() = default;

	virtual void Update() = 0;
	virtual void Draw() = 0;
};

typedef enum DamageType
{
	D_Enemy,
	D_EvilZone
} DamageType;