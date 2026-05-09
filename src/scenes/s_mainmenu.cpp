#include "scene.h"
#include "scene_manager.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <iostream>
#include <keybinds.h>

#define TITLE_SIZE 96

SceneManager g_SceneManager = SceneManager();

MainMenu::MainMenu() 
{
  GuiSetStyle(DEFAULT, TEXT_SIZE, 64);
}

MainMenu::~MainMenu(void) {}

void MainMenu::Update()
{
  acceptPressed = IsKeyPressed(ACCEPT);

  if (IsKeyPressed(KEY_RIGHT)) {
    MenuOption = 1;
  }
  if (IsKeyPressed(KEY_LEFT)) {
    MenuOption = 0;
  }
}

void MainMenu::Draw()
{
  ClearBackground(WHITE);
  Vector2 titleSize = MeasureTextEx(GetFontDefault(), "Siesta Disasta", TITLE_SIZE, 4);
  Vector2 titlePos = {(1920 - titleSize.x) / 2.0f, 225};
  DrawTextEx(GetFontDefault(), "Siesta Disasta", titlePos, TITLE_SIZE, 4, BLACK);

  GuiSetState(MenuOption == 0 ? STATE_FOCUSED : STATE_NORMAL);
  if (GuiButton(Rectangle(200, 700, 400, 200), "PLAY") || (MenuOption == 0 && acceptPressed))
  {
    g_SceneManager.SetScene(std::make_unique<Cutscene>());
  }

  GuiSetState(MenuOption == 1 ? STATE_FOCUSED : STATE_NORMAL);
  if (GuiButton(Rectangle(1320, 700, 400, 200), "QUIT") || (MenuOption == 1 && acceptPressed))
  {
    CloseWindow();
  }

  if (acceptPressed) {
    std::cout << "Accept Pressed";
  }
}
