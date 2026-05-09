#include "keybinds.h"
#include "renderer.h"
#include "scene.h"
#include "scene_manager.h"
#include <memory>
#include <raylib-cpp.hpp>

namespace
{
void DrawCenteredText(const char *text, float y, int fontSize, Color color)
{
	const int textWidth = MeasureText(text, fontSize);
	DrawText(text, (RENDER_TEXTURE_WIDTH - textWidth) / 2, (int)y, fontSize, color);
}

void DrawResultScreen(const char *title, const char *subtitle, Color accent)
{
	ClearBackground(BLACK);
	DrawCircleV(Vector2{RENDER_TEXTURE_WIDTH * 0.5f, RENDER_TEXTURE_HEIGHT * 0.5f}, 360.0f, Color{24, 24, 24, 255});
	DrawCircleLines(RENDER_TEXTURE_WIDTH / 2, RENDER_TEXTURE_HEIGHT / 2, 360.0f, accent);
	DrawCenteredText(title, 355.0f, 96, accent);
	DrawCenteredText(subtitle, 500.0f, 34, WHITE);
	DrawCenteredText(TextFormat("Press %s to fight again", ACCEPT_STRING), 640.0f, 28, Color{190, 190, 190, 255});
}
} // namespace

VictoryScreen::VictoryScreen() {}

VictoryScreen::~VictoryScreen() {}

void VictoryScreen::Update()
{
	if (IsKeyPressed(ACCEPT))
	{
		g_SceneManager.SetScene(std::make_unique<PlayMode>());
	}
}

void VictoryScreen::Draw()
{
	DrawResultScreen("VICTORY", "The clock has stopped.", Color{230, 148, 58, 255});
}

GameOverScreen::GameOverScreen() {}

GameOverScreen::~GameOverScreen() {}

void GameOverScreen::Update()
{
	if (IsKeyPressed(ACCEPT))
	{
		g_SceneManager.SetScene(std::make_unique<PlayMode>());
	}
}

void GameOverScreen::Draw()
{
	DrawResultScreen("DEFEAT", "Time caught up with you.", Color{210, 58, 58, 255});
}
