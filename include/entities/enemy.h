#pragma once

#include "entity.h"

enum class EnemyState
{
	Idle,
	Advance,
	WindUp,
	Attack,
	Recover,
	SecondaryWindUp,
	SecondaryAttack,
	SecondaryRecover,
	SpecialWindUp,
	SpecialAttack,
	SpecialRecover
};

class Enemy : public Entity
{
  public:
	Enemy(raylib::Vector2 startGridPos);
	~Enemy(void);

	void Update() override;
	void Draw() override;

	void SetTargetGridPosition(Vector2 target);
	EnemyState GetState() const;
	const char *GetStateName() const;
	int GetChaseMovesTaken() const;
	int GetChaseBudget() const;
	int GetNormalAttackCount() const;
	int GetNormalAttacksPerCycle() const;
	int GetMaxHealth() const;
	void Hurt(float amount);
	bool OccupiesGridPosition(Vector2 target) const;
	bool TryPushGridPosition(Vector2 direction);

	bool isInGrid;
	Vector2 gridPosition;
	Rectangle GetBBoxWorld();

  private:
	// Shared tuning
	static constexpr int BOSS_FOOTPRINT_HALF_CELLS = 1;
	static constexpr float ATTACK_RECOVER_DURATION = 0.9f;
	static constexpr float CYCLE_RECOVER_DURATION = 1.2f;
	static constexpr float CHASE_TIMEOUT_RECOVER_DURATION = 0.55f;
	static constexpr float PUNCH_EFFECT_DURATION = 0.75f;
	static constexpr float CLOCK_HAND_SWEEP_DEGREES = 125.0f;
	static constexpr int CLOCK_HAND_TRAIL_SAMPLES = 14;
	static constexpr float CLOCK_HAND_TRAIL_STEP = 0.02f;
	static constexpr int SECONDARY_PULSE_SEGMENTS = 96;
	static constexpr float BASIC_ATTACK_DAMAGE = 20.0f;
	static constexpr float SECONDARY_ATTACK_DAMAGE = 30.0f;

	// State flow
	void EnterState(EnemyState nextState);
	void EnterRecover(float duration);
	void UpdateIdle();
	void UpdateAdvance();
	void UpdateWindUp(float deltaTime);
	void UpdateRecover(float deltaTime);
	void UpdateSecondaryWindUp(float deltaTime);
	void UpdateSecondaryRecover(float deltaTime);
	void UpdateSpecialWindUp(float deltaTime);
	void UpdateSpecialRecover(float deltaTime);

	// Attack behavior
	void TriggerPunchEffect();
	void UpdatePunchEffect(float deltaTime);
	void PrimaryAttack();
	void SecondaryAttack(float deltaTime);
	void RunSelectedSpecialAttack();
	void SpecialAttack1();
	void SpecialAttack2();
	void SpecialAttack3();

	// Drawing
	void DrawPunchEffect();
	void DrawBasicAttackTelegraph();
	void DrawSecondaryAttackEffect();

	// Targeting
	Vector2 GetPunchDirectionToTarget() const;
	int GetNextBasicAttackRange() const;
	int GetCurrentBasicAttackRange() const;

	// Helpers
	static int DistanceFromBossFootprint(Vector2 bossCenter, Vector2 cell);
	static bool BossFootprintContainsCell(Vector2 bossCenter, Vector2 cell);
	static bool IsBossFootprintValid(Vector2 bossCenter);
	static Color ClockHandOrange(unsigned char alpha);
	static void DrawEnemyFace(Vector2 center, float radius);
	static void DrawAttackClockHand(Vector2 clockCenter, float sweepAngle, bool isRightSwing, unsigned char alpha,
									float lengthScale = 1.0f);

	// State
	EnemyState currentState = EnemyState::Idle;
	int maxHealth = 400;

	// Movement
	Vector2 targetGridPosition;
	float lerpSpeed = 0.2f;
	float moveCooldown = 0.5f;
	float currentMoveCooldown = 0;
	int chaseBudget = 6;
	int chaseMovesTaken = 0;

	// WindUp / Attack
	float stateTimer = 0;
	float baseAttackWindUpDuration = 1.1f;
	int attackTargetX = 0;
	int attackTargetY = 0;
	float punchAnimationTime = 1000.0f;
	Vector2 punchDirection = Vector2{1.0f, 0.0f};
	float punchHookSide = 1.0f;
	bool nextBasicAttackIsRightSwing = true;
	bool currentBasicAttackIsRightSwing = true;
	int hourHandAttackRange = 1;
	int minuteHandAttackRange = 2;
	float secondaryWindUpDuration = 1.4f;
	float secondaryAttackDuration = 0.9f;
	float secondaryRecoverDuration = 0.7f;
	int secondaryAttackRange = 3;
	bool secondaryAttackHasHit = false;
	float secondaryPulsePreviousRadius = 0.0f;
	float specialWindUpDuration = 1.6f;
	float specialRecoverDuration = 10.0f;
	int currentSpecialAttack = 2;
	int nextSpecialAttack = 2;

	// Attack cycle
	int normalAttackCount = 0;
	int normalAttacksPerCycle = 3;
	int primaryCyclesCompleted = 0;
	int primaryCyclesBeforeSpecial = 2;
};
