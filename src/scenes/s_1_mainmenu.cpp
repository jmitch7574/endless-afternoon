#include "arena_manager.h"
#include "keybinds.h"
#include "raymath.h"
#include "renderer.h"
#include "scene.h"
#include "scene_manager.h"
#include <algorithm>
#include <cmath>
#include <memory>

namespace
{
constexpr int TITLE_SIZE = 88;
constexpr float BUTTON_WIDTH = 320.0f;
constexpr float BUTTON_HEIGHT = 86.0f;
constexpr float BUTTON_GAP = 84.0f;
constexpr float BUTTON_Y = 790.0f;
constexpr Color CLOCK_ORANGE = Color{230, 148, 58, 255};
constexpr Color CLOCK_BROWN = Color{185, 112, 40, 255};
constexpr Color DARK_PANEL = Color{16, 13, 11, 235};

const Vector2 MENU_CENTER = Vector2{RENDER_TEXTURE_WIDTH / 2.0f, RENDER_TEXTURE_HEIGHT / 2.0f};

Rectangle PlayButtonBounds()
{
	return Rectangle{MENU_CENTER.x - BUTTON_WIDTH - BUTTON_GAP * 0.5f, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT};
}

Rectangle QuitButtonBounds()
{
	return Rectangle{MENU_CENTER.x + BUTTON_GAP * 0.5f, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT};
}

Vector2 GetRenderMousePosition()
{
	const float scaleX = (float)GetScreenWidth() / RENDER_TEXTURE_WIDTH;
	const float scaleY = (float)GetScreenHeight() / RENDER_TEXTURE_HEIGHT;
	const float scale = std::min(scaleX, scaleY);
	const float scaledWidth = RENDER_TEXTURE_WIDTH * scale;
	const float scaledHeight = RENDER_TEXTURE_HEIGHT * scale;
	const float offsetX = (GetScreenWidth() - scaledWidth) * 0.5f;
	const float offsetY = (GetScreenHeight() - scaledHeight) * 0.5f;
	const Vector2 mouse = GetMousePosition();

	return Vector2{(mouse.x - offsetX) / scale, (mouse.y - offsetY) / scale};
}

void DrawCenteredText(const char *text, float y, int fontSize, Color color)
{
	const Vector2 size = MeasureTextEx(GetFontDefault(), text, (float)fontSize, 4.0f);
	const Vector2 pos = Vector2{(RENDER_TEXTURE_WIDTH - size.x) * 0.5f, y};
	DrawTextEx(GetFontDefault(), text, pos, (float)fontSize, 4.0f, color);
}

void DrawOctagonClockPlate()
{
	DrawPoly(MENU_CENTER, 8, 285.0f, 22.5f, Fade(CLOCK_BROWN, 0.82f));
	DrawPolyLinesEx(MENU_CENTER, 8, 292.0f, 22.5f, 8.0f, WHITE);
	DrawPolyLinesEx(MENU_CENTER, 8, 244.0f, 22.5f, 4.0f, Fade(CLOCK_ORANGE, 0.7f));

	for (int h = 0; h < 12; h++)
	{
		const float angle = (h * 30.0f - 90.0f) * DEG2RAD;
		const float outerRadius = 236.0f;
		const float markerLength = h % 3 == 0 ? 28.0f : 18.0f;
		const Vector2 outer =
			Vector2{MENU_CENTER.x + cosf(angle) * outerRadius, MENU_CENTER.y + sinf(angle) * outerRadius};
		const Vector2 inner = Vector2{MENU_CENTER.x + cosf(angle) * (outerRadius - markerLength),
									  MENU_CENTER.y + sinf(angle) * (outerRadius - markerLength)};
		DrawLineEx(inner, outer, h % 3 == 0 ? 7.0f : 4.0f, WHITE);
	}
}

void DrawSelectionHand(bool playSelected)
{
	const Rectangle targetBounds = playSelected ? PlayButtonBounds() : QuitButtonBounds();
	const Vector2 target =
		Vector2{targetBounds.x + targetBounds.width * 0.5f, targetBounds.y + targetBounds.height * 0.5f};
	const Vector2 direction = Vector2Normalize(Vector2Subtract(target, MENU_CENTER));
	const Vector2 end = Vector2Add(MENU_CENTER, Vector2Scale(direction, 220.0f));
	const Vector2 hourEnd =
		Vector2Add(MENU_CENTER, Vector2Scale(Vector2Rotate(direction, playSelected ? -0.82f : 0.82f), 138.0f));

	DrawLineEx(MENU_CENTER, hourEnd, 16.0f, Fade(WHITE, 0.85f));
	DrawLineEx(MENU_CENTER, end, 12.0f, CLOCK_ORANGE);
	DrawCircleV(MENU_CENTER, 24.0f, WHITE);
	DrawCircleV(MENU_CENTER, 12.0f, CLOCK_ORANGE);
}

void DrawMenuButton(Rectangle bounds, const char *label, bool selected, bool hovered)
{
	const Color fill = selected ? Color{CLOCK_ORANGE.r, CLOCK_ORANGE.g, CLOCK_ORANGE.b, 255} : DARK_PANEL;
	const Color outline = selected || hovered ? WHITE : Fade(WHITE, 0.55f);
	const Color textColor = selected ? BLACK : WHITE;

	DrawRectangleRec(bounds, fill);
	DrawRectangleLinesEx(bounds, selected ? 5.0f : 3.0f, outline);

	const int fontSize = 48;
	const Vector2 textSize = MeasureTextEx(GetFontDefault(), label, (float)fontSize, 4.0f);
	const Vector2 textPos =
		Vector2{bounds.x + (bounds.width - textSize.x) * 0.5f, bounds.y + (bounds.height - textSize.y) * 0.5f};
	DrawTextEx(GetFontDefault(), label, textPos, (float)fontSize, 4.0f, textColor);
}
} // namespace

MainMenu::MainMenu() {}

MainMenu::~MainMenu(void) {}

void MainMenu::Update()
{
	const Vector2 mousePosition = GetRenderMousePosition();
	const bool hoverPlay = CheckCollisionPointRec(mousePosition, PlayButtonBounds());
	const bool hoverQuit = CheckCollisionPointRec(mousePosition, QuitButtonBounds());

	if (hoverPlay)
	{
		selectedOption = MenuOption::Play;
	}
	else if (hoverQuit)
	{
		selectedOption = MenuOption::Quit;
	}

	if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(MOVE_RIGHT))
	{
		selectedOption = MenuOption::Quit;
	}

	if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(MOVE_LEFT))
	{
		selectedOption = MenuOption::Play;
	}

	const bool accepted = IsKeyPressed(PRIMARY) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

	if (!accepted)
	{
		return;
	}

	if (selectedOption == MenuOption::Play)
	{
		g_SceneManager.SetScene(std::make_unique<Cutscene>());
	}
	else
	{
		CloseWindow();
	}
}

void MainMenu::Draw()
{
	ClearBackground(BLACK);

	ArenaManager::DrawLevelGrid();
	ArenaManager::MaskOutsideOctagon();
	ArenaManager::DrawOctagonBoundary();

	DrawOctagonClockPlate();
	DrawSelectionHand(selectedOption == MenuOption::Play);

	DrawCenteredText("Siesta Disasta", 150.0f, TITLE_SIZE, WHITE);
	DrawCenteredText("Free Yourself from the Hands of Time", 260.0f, 34, Fade(WHITE, 0.8f));

	const Vector2 mousePosition = GetRenderMousePosition();
	const bool playSelected = selectedOption == MenuOption::Play;
	const bool hoverPlay = CheckCollisionPointRec(mousePosition, PlayButtonBounds());
	const bool hoverQuit = CheckCollisionPointRec(mousePosition, QuitButtonBounds());

	DrawMenuButton(PlayButtonBounds(), "PLAY", playSelected, hoverPlay);
	DrawMenuButton(QuitButtonBounds(), "QUIT", !playSelected, hoverQuit);
	DrawCenteredText(TextFormat("Use Left/Right and %s", ACCEPT_STRING), 922.0f, 26, Fade(WHITE, 0.66f));
}
