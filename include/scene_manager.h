#pragma once
#include "scene.h"
#include <memory>


class SceneManager
{
  private:
	std::unique_ptr<Scene> currentScene;

  public:
	int hitstunFrames = 0;
	SceneManager() {

	};

	void SetScene(std::unique_ptr<Scene> scene) { currentScene = std::move(scene); }

	void Update()
	{
		if (hitstunFrames-- > 0) return;
		
		if (currentScene)
			currentScene->Update();
	}

	void Draw()
	{
		if (currentScene)
			currentScene->Draw();
	}
};

extern SceneManager g_SceneManager;