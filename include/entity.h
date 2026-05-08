#pragma once
#include <raylib-cpp.hpp>

class Entity {
protected:
  raylib::Texture2D texture;
  raylib::Vector2 position;

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

  void Update() override;
  void Draw() override;
};

class Enemy : public Entity {

public:
  Enemy(raylib::Vector2 startPos);
  ~Enemy(void);

  void Update() override;
  void Draw() override;
};

class ClockHand : public Entity {
public:
  ClockHand(raylib::Vector2 pivot, float angleDeg, float length, float thickness,
            Color color);
  ~ClockHand(void);

  void Update() override;
  void Draw() override;

protected:
  float angleDeg;
  float length;
  float thickness;
  Color color;
};
