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
	void TryAttack(Vector2 dir);
	void TryDash(Vector2 dir);

	void Update() override;
	void Draw() override;
	bool IsInvulnerable();
	float GetMaxHealth() const;
	float GetStamina() const;
	float GetMaxStamina() const;
	void FullHeal();

	void Hurt(float amount, DamageType damageType);
	void Knockback(Vector2 Knockback);

	Vector2 gridPosition;

  protected:
	float lerpSpeed = 0.2f;
	float moveCooldown = 0.25f;
	float currentMoveCooldown = 0;
	float attackCooldown = 0.75f;
	float attackCooldownTimer = 0.0f;
	float dashCooldown = 0.65f;
	float dashCooldownTimer = 0.0f;
	float dashInvulnerabilityDuration = 0.6f;
	float dashInvulnerabilityTimer = 0.0f;
	int dashRange = 3;

	float maxStamina = 100.0f;
	float stamina = 100.0f;
	float dashStaminaCost = 35.0f;
	float staminaRecoverRate = 8.1f;

	float healCooldown = 15;
	float healRate = 10; // Per Second
	float timeSinceLastDamage;

	bool isPunchFlipped = false;

	PlayerTrail trail[120] = {0};
	PlayerTrail punchTrail[40] = {0};

	bool dashedThisFrame = false;
	bool movedThisFrame = false;
	bool attackedThisFrame = false;
	float hitAnimationTime = 1000;
	float hitFlashTimer = 0.0f;
	Vector2 currentDirection = Vector2(1, 0);
	Vector2 attackDirection = Vector2(1, 0);
	Vector2 handpos;
	float peakThreshold = 0;
};
