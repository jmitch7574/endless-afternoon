#pragma once
#include <raylib-cpp.hpp>

struct PlayerTrail
{
	Vector2 pos;
	int opacity;
};

class Entity
{
  protected:
	raylib::Texture2D texture;
	raylib::Vector2 position;
	int health;

	Vector2 gridPosition;
	float lerpSpeed = 0.2f;
	float moveCooldown = 0.25f;
	float currentMoveCooldown = 0;

  public:
	Entity() = default;
	Entity(raylib::Vector2 startPos) : position(startPos) {}
	Vector2 GetPosition() { return position; };
	float GetHealth() { return health; };
	virtual ~Entity() = default;

	virtual void Update() = 0;
	virtual void Draw() = 0;
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
	bool IsInvulnerable() const;

	void Hurt(float amount);
	void Knockback(Vector2 Knockback);

	Vector2 gridPosition;

  protected:
	float lerpSpeed = 0.2f;
	float moveCooldown = 0.25f;
	float currentMoveCooldown = 0;
	float dashDoubleTapWindow = 0.35f;
	float dashCooldown = 0.65f;
	float dashInvulnerabilityDuration = 0.4f;
	float dashInvulnerabilityTimer = 0.0f;
	float lastDashTapTime = -1000.0f;
	Vector2 lastDashTapDirection = Vector2{0.0f, 0.0f};
	int dashRange = 3;

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

enum class EnemyState
{
	Idle,
	Advance,
	WindUp,
	Attack,
	Recover,
	SecondaryWindUp,
	SecondaryAttack,
	SecondaryRecover
};

class Enemy : public Entity
{
  public:
	// Constructor / Destructor
	Enemy(raylib::Vector2 startGridPos);
	~Enemy(void);

	// Update / Draw
	void Update() override;
	void Draw() override;

	// Getters / Setters
	void SetTargetGridPosition(Vector2 target);
	EnemyState GetState() const;
	const char *GetStateName() const;
	int GetChaseMovesTaken() const;
	int GetChaseBudget() const;
	int GetNormalAttackCount() const;
	int GetNormalAttacksPerCycle() const;
	bool OccupiesGridPosition(Vector2 target) const;
	bool TryPushGridPosition(Vector2 direction);

	// Collision / Grid-based movement
	bool isInGrid;
	Vector2 gridPosition;
	Rectangle GetBBoxWorld();

  private:
	void EnterState(EnemyState nextState);
	void EnterRecover(float duration);
	void TriggerPunchEffect();
	void UpdatePunchEffect(float deltaTime);
	void DrawPunchEffect();
	void DrawBasicAttackTelegraph();
	void DrawSecondaryAttackEffect();
	Vector2 GetPunchDirectionToTarget() const;
	int GetNextBasicAttackRange() const;
	int GetCurrentBasicAttackRange() const;

	// State
	EnemyState currentState = EnemyState::Idle;

	// Movement
	Vector2 targetGridPosition;
	float lerpSpeed = 0.2f;
	float moveCooldown = 0.5f;
	float currentMoveCooldown = 0;
	int aggroRange = 6;
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

	// Attack cycle
	int normalAttackCount = 0;
	int normalAttacksPerCycle = 3;
};

class ClockHand : public Entity
{
  public:
	float GetAngle();
	ClockHand(raylib::Vector2 pivot, float angleDeg, float length, float thickness, Color color);
	~ClockHand(void);

	void Update() override;
	void Draw() override;
	void Advance();
	void BeginBigDeadlySpin();
	Vector2 GetLargeExtendedPoint();

	bool activated = false;

  protected:
	float angleDeg;
	float length;
	float thickness;
	Color color;

	float bigDeadlySpinTime = 0;
	float bigDeadlySpinCoefficient = 40;
	bool inBigDeadlySpin = false;

	float advanceTime = 0;
	bool isAdvancing = false;
};
