#pragma once

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

  int MenuOption = true;
  bool acceptPressed = false;
};

class PlayMode : public Scene {
public:
  PlayMode();
  ~PlayMode(void);

  void Update() override;
  void Draw() override;
};