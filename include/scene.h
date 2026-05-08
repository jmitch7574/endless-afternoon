#pragma once

class Scene {
public:
  Scene() = default;
  virtual ~Scene() = default;

  virtual void Update() const = 0;
  virtual void Draw() const = 0;
};

class MainMenu : public Scene {
public:
  MainMenu();
  ~MainMenu(void);

  void Update() const override;
  void Draw() const override;
};