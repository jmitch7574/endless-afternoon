#include "raylib-cpp.hpp"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#include "renderer.h"
#include "screen_shake.h"
#include <chrono>
#include "resource_loader.h"
#include "scene_manager.h"
#include "utils.h"
#include <memory>

// Update the scene that is currently active, and draw it to the render texture.
void UpdateDrawFrame()
{
	const float deltaTime = GetFrameTime();
	g_SceneManager.Update();
	ScreenShake::Update(deltaTime);

	BeginTextureMode(rt_RenderTexture);
	g_SceneManager.Draw();
	EndTextureMode();

	BeginDrawing();
	Renderer::BlipRenderTexture();

#ifdef ARCADEMIA
	DrawText("Arcademia Edition", 20, 20, 32, WHITE);
#endif

	EndDrawing();
}

// Main enrty to game starts here.
int main()
{
	raylib::Window window(RENDER_TEXTURE_WIDTH, RENDER_TEXTURE_HEIGHT, "Countdown");

	Renderer::InitRenderTexture();
	Utils::SeedRandom();
	g_SceneManager.SetScene(std::make_unique<MainMenu>());

#if defined(PLATFORM_WEB)
	emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
	SetTargetFPS(60);
	SetWindowState(FLAG_WINDOW_UNDECORATED);
	SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
	SetWindowPosition(0, 0);

	g_SceneManager.SetScene(std::make_unique<MainMenu>());

  SetWindowState(FLAG_WINDOW_UNDECORATED);
  SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
  SetWindowPosition(0, 0);

	InitAudioDevice();
	Resources::Load();

	while (!window.ShouldClose())
	{
		UpdateDrawFrame();
	}

#endif

	return 0;
}
