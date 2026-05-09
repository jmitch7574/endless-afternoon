#pragma once
#include "scene.h"
#include <memory>


class SceneManager
{
  private:
	std::unique_ptr<Scene> currentScene;

  public:
	SceneManager() {

	};

	void SetScene(std::unique_ptr<Scene> scene) { currentScene = std::move(scene); }

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

extern SceneManager g_SceneManager;
