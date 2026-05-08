#pragma once
#include <memory>
#include "Scene.h"

class SceneManager
{
private:
  std::unique_ptr<Scene> currentScene;

public:
  SceneManager()
  {
    SetScene(std::make_unique<MainMenu>());
  };

  void SetScene(std::unique_ptr<Scene> scene)
  {
    currentScene = std::move(scene);
  }

  void Update() const
  {
    if (currentScene)
      currentScene->Update();
  }

  void Draw() const
  {
    if (currentScene)
      currentScene->Draw();
  }
};