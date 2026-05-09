#include "raylib-cpp.hpp"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif
#include "scene_manager.h"
#include "renderer.h"
#include <chrono>

int renderTextureWidth = 1920;
int renderTextureHeight = 1080;
RenderTexture2D target;

void UpdateDrawFrame(void); // Update and Draw one frame

// Initialisation
int main()
{
	raylib::Window window(1920, 1080, "raylib-cpp [core] example - basic window");

  Renderer::InitRenderTexture();

	const auto p1 = std::chrono::system_clock::now();
	SetRandomSeed(std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count() );
  //DisableCursor();

#if defined(PLATFORM_WEB)
	emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else

	SetTargetFPS(60);

	g_SceneManager.SetScene(std::make_unique<MainMenu>());

  SetWindowState(FLAG_WINDOW_UNDECORATED);
  SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
  SetWindowPosition(0, 0);

  while (!window.ShouldClose())
  {

		g_SceneManager.Update();

    // Render Texture - Actual Game
    BeginTextureMode(rt_RenderTexture);

		g_SceneManager.Draw();

		EndTextureMode();

		// Flip Render Texture to Any Resolution
		BeginDrawing();

    Renderer::BlipRenderTexture();

#ifdef ARCADEMIA
    DrawText("Arcademia Edition", 20, 20, 32, WHITE);
#endif

		EndDrawing();
	}

#endif

	return 0;
}
