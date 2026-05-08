#include "scene.h"
#include "scene_manager.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define TITLE_SIZE 96

SceneManager g_SceneManager = SceneManager();

MainMenu::MainMenu() {}
MainMenu::~MainMenu(void) {}

bool acceptPressed = false;

void MainMenu::Update()
{
  acceptPressed = IsKeyPressed(KEY_Z);
}

void MainMenu::Draw()
{
  ClearBackground(WHITE);
  Vector2 titleSize = MeasureTextEx(GetFontDefault(), "Siesta Disasta", TITLE_SIZE, 4);
  Vector2 titlePos = {(1920 - titleSize.x) / 2.0f, 225};
  DrawTextEx(GetFontDefault(), "Siesta Disasta", titlePos, TITLE_SIZE, 4, BLACK);

  GuiSetState(MenuOption == 0 ? STATE_FOCUSED : STATE_NORMAL);
  if (GuiButton(Rectangle(200, 200, 400, 400), "PLAY") || (MenuOption == 0 && acceptPressed))
  {
    g_SceneManager.SetScene(std::make_unique<PlayMode>());
  }
}
