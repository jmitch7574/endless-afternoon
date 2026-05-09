#pragma once
#include "entity.h"
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

	int MenuOption = 0;
	bool acceptPressed = false;
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
  void DrawEvilZone();
  void DrawEnemyHealthBar();

	Player player;
  bool isPlayerInEvilZone;
  float timeSinceEvilZoneTick = 100;

	Enemy enemy;
	ClockHand minuteHand;
	ClockHand hourHand;
	DangerEffects dangerEffects;

  private:
	void BeginVictory();
	void BeginGameOver();

	bool victoryTriggered = false;
	bool gameOverTriggered = false;
	float resultTransitionTimer = 0.0f;
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
