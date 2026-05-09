#pragma once
#include "danger_effects.h"
#include "entities/clockhand.h"
#include "entities/enemy.h"
#include "entities/player.h"
#include "danger_effects.h"

class Scene
{
  public:
	Scene() = default;
	virtual ~Scene() = default;

	virtual void Update() = 0;
	virtual void Draw() = 0;
};

class MainMenu : public Scene
{
  public:
	MainMenu();
	~MainMenu(void);

	void Update() override;
	void Draw() override;

  private:
	enum class MenuOption
	{
		Play,
		Quit
	};

	MenuOption selectedOption = MenuOption::Play;
};

class Cutscene : public Scene
{
  public:
	Cutscene();
	~Cutscene(void);

	void Update() override;
	void Draw() override;
};

class PlayMode : public Scene
{
  public:
	PlayMode();
	~PlayMode(void);

	void Update() override;
	void Draw() override;

	void EnemyHit(float damage);
	void StartRedLightGreenLight();
	void DrawEvilZone();
	void DrawEnemyHealthBar();

	Player player;
	bool isPlayerInEvilZone = false;
	float timeSinceEvilZoneTick = 100;

	Enemy enemy;
	ClockHand minuteHand;
	ClockHand hourHand;
	DangerEffects dangerEffects;

  private:
	struct RedLightCell
	{
		Vector2 gridPosition;
		float telegraphTimer;
	};

	void BeginVictory();
	void BeginGameOver();
	void UpdateBossPhaseFromHealth();
	void UpdateRedLightGreenLight(float deltaTime);
	void DrawRedLightGreenLight();
	void QueueRedLightCells();
	bool IsRedLightCellQueued(Vector2 gridPosition) const;
	bool IsPlayerOnActiveRedLightCell() const;

	bool victoryTriggered = false;
	bool gameOverTriggered = false;
	float resultTransitionTimer = 0.0f;
	int bossHealthPhase = 0;
	bool redLightGreenLightActive = false;
	float redLightGreenLightTimer = 0.0f;
	float redLightNextWaveTimer = 0.0f;
	float redLightDamageCooldown = 0.0f;
	std::vector<RedLightCell> redLightCells;
};

class VictoryScreen : public Scene
{
  public:
	VictoryScreen();
	~VictoryScreen(void);

	void Update() override;
	void Draw() override;
};

class GameOverScreen : public Scene
{
  public:
	GameOverScreen();
	~GameOverScreen(void);

	void Update() override;
	void Draw() override;
};

extern PlayMode* playScene;
