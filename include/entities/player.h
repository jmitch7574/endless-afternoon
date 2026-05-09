#pragma once

#include "entity.h"

struct PlayerTrail
{
	Vector2 pos;
	int opacity;
};

class Player : public Entity
{
  public:
	Player(raylib::Vector2 startPos);
	~Player(void);

	void TryMove(Vector2 dir);
	void TryDash(Vector2 dir);

	void Update() override;
	void Draw() override;
	bool IsInvulnerable();

	void Hurt(float amount, DamageType damageType);
	void Knockback(Vector2 Knockback);

	Vector2 gridPosition;

  protected:
	float lerpSpeed = 0.2f;
	float moveCooldown = 0.25f;
	float currentMoveCooldown = 0;
	float dashDoubleTapWindow = 0.35f;
	float dashCooldown = 0.65f;
	float dashCooldownTimer = 0.0f;
	float dashInvulnerabilityDuration = 0.4f;
	float dashInvulnerabilityTimer = 0.0f;
	float lastDashTapTime = -1000.0f;
	Vector2 lastDashTapDirection = Vector2{0.0f, 0.0f};
	int dashRange = 3;

	float healCooldown = 6;
	float healRate = 10; // Per Second
	float timeSinceLastDamage;

	bool isPunchFlipped = false;

	PlayerTrail trail[120] = {0};
	PlayerTrail punchTrail[40] = {0};

	bool dashedThisFrame = false;
	bool movedThisFrame = false;
	bool attackedThisFrame = false;
	float hitAnimationTime = 1000;
	Vector2 currentDirection = Vector2(1, 0);
	Vector2 handpos;
	float peakThreshold = 0;
};
