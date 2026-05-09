#include "keybinds.h"
#include "raylib-cpp.hpp"
#include "scene.h"
#include "scene_manager.h"
#include <format>
#include <string>

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
}

Cutscene::~Cutscene() {}

void Cutscene::Update()
{
#ifndef NDEBUG
	g_SceneManager.SetScene(std::make_unique<PlayMode>());
	return;
#endif
	ticks++;

	shownText = (ticks / 5);

	currentString = lines[currentLine];

	textToDisplay = currentString.substr(0, shownText);

	if (IsKeyPressed(PRIMARY))
		cutsceneSkipped = true;

	if (shownText > currentString.length() + 30 && currentLine < 2)
	{
		currentLine++;
		ticks = 0;
	}

	if ((currentLine == 2 && shownText > currentString.length() + 30) || cutsceneSkipped)
	{
		fadeTicks++;

		if (fadeTicks == 180)
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

	int actualFade = std::min(fadeTicks * 2, 255);

	DrawRectangle(0, 0, 1920, 1080, Color(0, 0, 0, actualFade));
}