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

enum class EnemyEmotion
{
	Happy,
	Angry
};

struct EnemyTrail
{
	Vector2 pos;
	int opacity;
};

enum class WorldSpace
{
	Grid,
	Floaty
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
	float GetOpacity() { return opacity; }
	void Hurt(float amount);
	bool OccupiesGridPosition(Vector2 target);
	bool TryPushGridPosition(Vector2 direction);

	bool isInGrid;
	Vector2 gridPosition;
	Rectangle GetBBoxWorld();

  private:
	// Shared tuning
	static constexpr int BOSS_FOOTPRINT_HALF_CELLS = 1;
	static constexpr float ATTACK_RECOVER_DURATION = 0.25f;
	static constexpr float CYCLE_RECOVER_DURATION = 0.35f;
	static constexpr float CHASE_TIMEOUT_RECOVER_DURATION = 0.55f;
	static constexpr float PUNCH_EFFECT_DURATION = 0.75f;
	static constexpr float CLOCK_HAND_SWEEP_DEGREES = 125.0f;
	static constexpr float CLOCK_HAND_TARGET_OVERSHOOT_DEGREES = 35.0f;
	static constexpr float CLOCK_HAND_PULLBACK_DEGREES = 28.0f;
	static constexpr int CLOCK_HAND_TRAIL_SAMPLES = 14;
	static constexpr float CLOCK_HAND_TRAIL_STEP = 0.02f;
	static constexpr int SECONDARY_PULSE_SEGMENTS = 96;
	static constexpr float BASIC_ATTACK_DAMAGE = 15.0f;
	static constexpr float SECONDARY_ATTACK_DAMAGE = 20.0f;
	static constexpr int MOVE_TRAIL_SAMPLES = 120;
	static constexpr int MOVE_TRAIL_OPACITY = 255;
	static constexpr int MOVE_TRAIL_FADE = 25;

	// State flow
	void EnterState(EnemyState nextState);
	void EnterRecover(float duration);
	void UpdateIdle();
	void UpdateAdvance();
	bool TryMoveTowardTarget();
	void UpdateWindUp(float deltaTime);
	void UpdateRecover(float deltaTime);
	void UpdateSecondaryWindUp(float deltaTime);
	void UpdateSecondaryRecover(float deltaTime);
	void UpdateSpecialWindUp(float deltaTime);
	void UpdateSpecialRecover(float deltaTime);

	// Attack behavior
	void TryPrimaryAttack();
	void TriggerPunchEffect();
	void UpdatePunchEffect(float deltaTime);
	bool PunchEffectHitsPlayerAtProgress(float progress) const;
	void PrimaryAttack();
	void CompletePrimaryAttackCycle();
	void SecondaryAttack(float deltaTime);
	void PulseSecondaryAttack(float deltaTime);
	void SpinningTopSecondaryAttack(float deltaTime);
	void FinishSecondaryAttack();
	void RunSelectedSpecialAttack();
	void SpecialAttack1();
	void SpecialAttack2();
	void SpecialAttack3();
	void SpecialAttack4();

	// Drawing
	void UpdateMovementTrail();
	void DrawMovementTrail();
	void DrawPunchEffect();
	void DrawBasicAttackTelegraph();
	void DrawSecondaryAttackEffect();
	void DrawSpinningSecondaryAttackEffect();
	void DrawDetachedHand(Vector2 handPosition, unsigned char alpha, float scale = 1.0f,
						  bool holdingHandstache = false, float heldAngle = 0.0f,
						  float heldHandstacheLength = 0.0f);

	// Targeting
	Vector2 GetPunchDirectionToTarget() const;
	float GetPrimarySwingStartAngle() const;
	float GetPrimarySwingAngle(float progress) const;
	Vector2 GetPrimarySwingHandBase(float progress) const;
	Vector2 GetPrimarySwingHandTip(float progress) const;
	Vector2 GetAttackHandPoint(float sweepAngle, bool isRightSwing, float lengthScale = 1.0f) const;
	Vector2 GetAttackHandPointAtRange(float sweepAngle, float rangeCells, float lengthScale = 1.0f) const;
	Vector2 GetHeldHandBase(float heldAngle) const;
	Vector2 GetHeldHandTip(float heldAngle, bool isRightHand) const;
	Vector2 GetReachAroundHandPoint(bool isRightHand, float progress) const;
	Vector2 GetHandstacheBase() const;
	float GetHandstacheLength(bool isRightHand) const;
	Vector2 GetHandstacheTip(bool isRightHand) const;
	float GetHandstacheAngle(bool isRightHand) const;
	bool ShouldHideHandstache(bool isRightHand) const;
	int GetNextBasicAttackRange() const;
	int GetCurrentBasicAttackRange() const;

	// Helpers
	static int DistanceFromBossFootprint(Vector2 bossCenter, Vector2 cell);
	bool BossFootprintContainsCell(Vector2 bossCenter, Vector2 cell);
	static bool IsBossFootprintValid(Vector2 bossCenter);
	static Color ClockHandOrange(unsigned char alpha);
									
	void UpdateEnemyFace();
	void DrawEnemyFace();

	// State
	EnemyState currentState = EnemyState::Idle;
	WorldSpace worldSpace = WorldSpace::Grid;
	float scale = 1;
	float specialStateTimeLeft = 0;
	float specialStateTimeIn = 0;
	float opacity = 1; // 0 - 1
	int maxHealth = 400;
	float timeSinceLastHit = 10;

	// Facial Features
	EnemyEmotion currentEmotion = EnemyEmotion::Angry;
	float currentEyeRotation = 0;
	float TargetEyeRotation() { return currentEmotion == EnemyEmotion::Angry ? 45.0f : 180.0f; };

	Vector2 eyeOffsets = Vector2(0, 0);
	Vector2 targetPupilOffsets;

	float maxEyeMovement = 10;

	float clockMarkingOffset = 0;
	float clockMarkingValue = 0;
	float clockMarkingLerp = 0.2f;

	// Facial Features - Moustache
	Vector2 positionLastFrame;

	float TargetMoustacheGap(float x) 
	{
		if (x < -0.2f) return 15.0f;
		if (x > 0.2f) return 45.0f;
		return 30.0f;
	};
	float currentMoustacheGap = 0;
	float moustacheGapLerp = 0.1f;
	
	float TargetMoustacheOffset(float x)
	{
		if (x < -0.2f) return -25.0f;
		if (x > 0.2f) return 25.0f;
		return 0.0f;
	}
	float currentMoustacheOffset = 0;
	float moustacheOffsetLerp = 0.1f;

	// Movement
	Vector2 targetGridPosition;
	float lerpSpeed = 0.2f;
	float moveCooldown = 0.5f;
	float currentMoveCooldown = 0;
	int chaseBudget = 10;
	int chaseMovesTaken = 0;
	EnemyTrail moveTrail[MOVE_TRAIL_SAMPLES] = {};

	// WindUp / Attack
	float stateTimer = 0;
	float baseAttackWindUpDuration = 0.65f;
	int attackTargetX = 0;
	int attackTargetY = 0;
	float punchAnimationTime = 1000.0f;
	bool punchEffectHasHit = false;
	float primaryAttackCooldown = 1.3f;
	float primaryAttackMovementLockDuration = 0.0f;
	float primaryAttackMovementLockTimer = 0.0f;
	float currentPrimaryAttackCooldown = 0.0f;
	Vector2 punchDirection = Vector2{1.0f, 0.0f};
	float punchHookSide = 1.0f;
	float punchStartAngle = 0.0f;
	float punchSweepDegrees = CLOCK_HAND_SWEEP_DEGREES;
	bool nextBasicAttackIsRightSwing = true;
	bool currentBasicAttackIsRightSwing = true;
	int hourHandAttackRange = 1;
	int minuteHandAttackRange = 2;
	float secondaryWindUpDuration = 1.4f;
	float secondaryAttackDuration = 0.9f;
	float secondaryRecoverDuration = 0.45f;
	int secondaryAttackRange = 3;
	bool secondaryAttackHasHit = false;
	float secondaryPulsePreviousRadius = 0.0f;
	int currentSecondaryAttack = 1;
	int nextSecondaryAttack = 1;
	float spinningSecondaryAttackDuration = 5.0f;
	static constexpr float SPINNING_SECONDARY_WINDUP_ROTATION_DEGREES = 70.0f;
	float spinningSecondarySpinAngle = 0.0f;
	float spinningSecondarySpinSpeed = 680.0f;
	float spinningSecondaryDamageCooldown = 0.0f;
	float spinningSecondaryDamageInterval = 0.55f;
	int spinningSecondarySpinDirection = 1;
	static constexpr float SPINNING_SECONDARY_DAMAGE = 10.0f;
	float specialWindUpDuration = 1.6f;
	float specialRecoverDuration = 0.65f;
	int currentSpecialAttack = 2;
	int nextSpecialAttack = 2;
	int queuedClockHandSpinDirection = 0;

	// Attack cycle
	int normalAttackCount = 0;
	int normalAttacksPerCycle = 3;
	int primaryCyclesCompleted = 0;
	int primaryCyclesBeforeSpecial = 2;

	// Enemy Colours
	Color ClockWhite() {
		if (worldSpace == WorldSpace::Floaty && specialStateTimeIn >= 1 && specialStateTimeLeft >= 1)
			return Color{255, 255, 255, (unsigned char)(Enemy::GetOpacity() * 64.0f)};
		return Color{255, 255, 255, (unsigned char)(Enemy::GetOpacity() * 255.0f)};
	}
	Color ClockHands() {
		return Color{255, 255, 255, (unsigned char)(Enemy::GetOpacity() * 255.0f)};
	}
	Color ClockBrown() { return Color{127, 106, 79, (unsigned char)(Enemy::GetOpacity() * 255.0f)}; }
	Color ClockOrange() 
	{ 
		if (worldSpace == WorldSpace::Floaty && specialStateTimeIn >= 1 && specialStateTimeLeft >= 1)
			return Color{255, 161, 0, (unsigned char)(Enemy::GetOpacity() * 64.0f)};
		return Color{255, 161, 0, (unsigned char)(Enemy::GetOpacity() * 255.0f)};
	}
	Color ClockRest() { return Fade(ClockOrange(), 0.45f); }
};
