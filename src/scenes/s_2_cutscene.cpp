#include "keybinds.h"
#include "raylib-cpp.hpp"
#include "scene.h"
#include "scene_manager.h"
#include <algorithm>
#include <format>
#include <string>

namespace
{
constexpr int TEXT_TICKS_PER_CHARACTER = 5;
constexpr int FAST_FORWARD_TICK_MULTIPLIER = 5;
constexpr int LINE_END_PAUSE_CHARACTERS = 30;
constexpr int CUTSCENE_FADE_TICKS = 180;

int GetTextAdvanceTicks()
{
	return IsKeyDown(SECONDARY) ? FAST_FORWARD_TICK_MULTIPLIER : 1;
}
} // namespace

std::string lines[] = {
	"The Siesta is a short nap taken during the \nearly afternoon, a moment of peace \nafter a morning's work",
	"They should come and go with no issue, and \nshould be followed by an easy wake, \nand a feeling of "
	"revitalization",
	"Though sometimes, people find themselves \nparalyzed, unable to wake for what feels \nlike eternity, an endless "
	"afternoon..."};

int currentLine;
int shownText;
int ticks;
std::string currentString;
std::string textToDisplay;
int fadeTicks = 0;
bool cutsceneSkipped = false;

Cutscene::Cutscene()
{
	ticks = 0;
	currentLine = 0;
	shownText = 0;
	fadeTicks = 0;
	cutsceneSkipped = false;
	currentString = lines[currentLine];
	textToDisplay.clear();
}

Cutscene::~Cutscene() {}

void Cutscene::Update()
{
#ifndef NDEBUG
	g_SceneManager.SetScene(std::make_unique<PlayMode>());
	return;
#endif
	ticks += GetTextAdvanceTicks();

	shownText = (ticks / TEXT_TICKS_PER_CHARACTER);

	currentString = lines[currentLine];

	textToDisplay = currentString.substr(0, shownText);

	if (IsKeyPressed(PRIMARY))
		cutsceneSkipped = true;

	if (shownText > currentString.length() + LINE_END_PAUSE_CHARACTERS && currentLine < 2)
	{
		currentLine++;
		ticks = 0;
	}

	if ((currentLine == 2 && shownText > currentString.length() + LINE_END_PAUSE_CHARACTERS) || cutsceneSkipped)
	{
		fadeTicks++;

		if (fadeTicks == CUTSCENE_FADE_TICKS)
		{
			g_SceneManager.SetScene(std::make_unique<PlayMode>());
		}
	}
}

void Cutscene::Draw()
{
	ClearBackground(BLACK);
	DrawText(TextSubtext(currentString.c_str(), 0, shownText), 200, 300, 64, WHITE);
	DrawText(std::format("Press {} to Skip", ACCEPT_STRING).c_str(), 50, 50, 32, WHITE);
	DrawText(std::format("Hold {} to Speed Up", SECONDARY_STRING).c_str(), 50, 90, 28, Fade(WHITE, 0.72f));

	int actualFade = std::min(fadeTicks * 2, 255);

	DrawRectangle(0, 0, 1920, 1080, Color(0, 0, 0, actualFade));
}
