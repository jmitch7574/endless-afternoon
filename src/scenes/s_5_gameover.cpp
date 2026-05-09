#include "arena_manager.h"
#include "keybinds.h"
#include "renderer.h"
#include "scene.h"
#include "scene_manager.h"
#include <cmath>
#include <memory>

namespace
{
constexpr int TITLE_SIZE = 96;
constexpr Color CLOCK_ORANGE = Color{230, 148, 58, 255};
constexpr Color CLOCK_BROWN = Color{185, 112, 40, 255};
constexpr Color DANGER_RED = Color{210, 58, 58, 255};

const Vector2 SCREEN_CENTER = Vector2{RENDER_TEXTURE_WIDTH / 2.0f, RENDER_TEXTURE_HEIGHT / 2.0f};

void DrawCenteredText(const char *text, float y, int fontSize, Color color)
{
	const Vector2 size = MeasureTextEx(GetFontDefault(), text, (float)fontSize, 4.0f);
	const Vector2 pos = Vector2{(RENDER_TEXTURE_WIDTH - size.x) * 0.5f, y};
	DrawTextEx(GetFontDefault(), text, pos, (float)fontSize, 4.0f, color);
}

void DrawArenaBackdrop()
{
	ClearBackground(BLACK);
	ArenaManager::DrawLevelGrid();
	ArenaManager::MaskOutsideOctagon();
	ArenaManager::DrawOctagonBoundary();
	ArenaManager::DrawClockMarkers();
}

void DrawDefeatClockPlate()
{
	DrawPoly(SCREEN_CENTER, 8, 286.0f, 22.5f, Fade(CLOCK_BROWN, 0.76f));
	DrawPolyLinesEx(SCREEN_CENTER, 8, 294.0f, 22.5f, 8.0f, WHITE);
	DrawPolyLinesEx(SCREEN_CENTER, 8, 244.0f, 22.5f, 4.0f, Fade(DANGER_RED, 0.78f));

	for (int h = 0; h < 12; h++)
	{
		const float angle = (h * 30.0f - 90.0f) * DEG2RAD;
		const float outerRadius = 232.0f;
		const float markerLength = h % 3 == 0 ? 30.0f : 17.0f;
		const Vector2 outer =
			Vector2{SCREEN_CENTER.x + cosf(angle) * outerRadius, SCREEN_CENTER.y + sinf(angle) * outerRadius};
		const Vector2 inner = Vector2{SCREEN_CENTER.x + cosf(angle) * (outerRadius - markerLength),
									  SCREEN_CENTER.y + sinf(angle) * (outerRadius - markerLength)};
		DrawLineEx(inner, outer, h % 3 == 0 ? 7.0f : 4.0f, Fade(WHITE, 0.86f));
	}

	DrawLineEx(SCREEN_CENTER, Vector2{SCREEN_CENTER.x - 150.0f, SCREEN_CENTER.y + 116.0f}, 13.0f, DANGER_RED);
	DrawLineEx(SCREEN_CENTER, Vector2{SCREEN_CENTER.x + 144.0f, SCREEN_CENTER.y - 122.0f}, 17.0f, Fade(WHITE, 0.86f));
	DrawLineEx(Vector2{SCREEN_CENTER.x - 92.0f, SCREEN_CENTER.y - 94.0f},
			   Vector2{SCREEN_CENTER.x - 30.0f, SCREEN_CENTER.y - 38.0f}, 6.0f, DANGER_RED);
	DrawLineEx(Vector2{SCREEN_CENTER.x + 70.0f, SCREEN_CENTER.y + 44.0f},
			   Vector2{SCREEN_CENTER.x + 134.0f, SCREEN_CENTER.y + 96.0f}, 6.0f, DANGER_RED);
	DrawCircleV(SCREEN_CENTER, 24.0f, WHITE);
	DrawCircleV(SCREEN_CENTER, 12.0f, DANGER_RED);
}
} // namespace

GameOverScreen::GameOverScreen() {}

GameOverScreen::~GameOverScreen() {}

void GameOverScreen::Update()
{
	if (IsKeyPressed(PRIMARY))
	{
		g_SceneManager.SetScene(std::make_unique<PlayMode>());
	}
}

void GameOverScreen::Draw()
{
	DrawArenaBackdrop();
	DrawDefeatClockPlate();

	DrawCenteredText("DEFEAT", 155.0f, TITLE_SIZE, DANGER_RED);
	DrawCenteredText("Time caught up with you.", 640.0f, 36, WHITE);
	DrawCenteredText(TextFormat("Press %s to fight again", ACCEPT_STRING), 742.0f, 28, Fade(WHITE, 0.68f));
}
