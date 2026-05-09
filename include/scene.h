#pragma once
#include "entity.h"

class Scene {
public:
  Scene() = default;
  virtual ~Scene() = default;

  virtual void Update() = 0;
  virtual void Draw() = 0;
};

class MainMenu : public Scene {
public:
  MainMenu();
  ~MainMenu(void);

  void Update() override;
  void Draw() override;

  int MenuOption = 0;
  bool acceptPressed = false;
};

class Cutscene : public Scene {
  public:
  Cutscene();
  ~Cutscene(void);

  void Update() override;
  void Draw() override;
};

class PlayMode : public Scene {
public:
  PlayMode();
  ~PlayMode(void);

  void Update() override;
  void Draw() override;

private:
  Player player;
  Enemy enemy;
  ClockHand minuteHand;
  ClockHand hourHand;

  void DrawLevelBoundary();
  void MaskOutsideOctagon();
  void DrawClockMarkers();
};