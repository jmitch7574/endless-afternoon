/*******************************************************************************************
 *
 *   raylib-cpp [core] example - Basic window (adapted for HTML5 platform)
 *
 *   This example is prepared to compile for PLATFORM_WEB, PLATFORM_DESKTOP and
 * PLATFORM_RPI As you will notice, code structure is slightly diferent to the
 * other examples... To compile it for PLATFORM_WEB just uncomment #define
 * PLATFORM_WEB at beginning
 *
 *   This example has been created using raylib-cpp (www.raylib.com)
 *   raylib is licensed under an unmodified zlib/libpng license (View raylib.h
 * for details)
 *
 *   Copyright (c) 2015 Ramon Santamaria (@raysan5)
 *
 ********************************************************************************************/

#include "raylib-cpp.hpp"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
int renderTextureWidth = 1920;
int renderTextureHeight = 1080;
RenderTexture2D target;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void); // Update and Draw one frame

//----------------------------------------------------------------------------------
// Main Enry Point
//----------------------------------------------------------------------------------
int main()
{
  // Initialization
  //--------------------------------------------------------------------------------------
  raylib::Window window(1920, 1080,
                        "raylib-cpp [core] example - basic window");

  ToggleFullscreen();
  target = LoadRenderTexture(renderTextureWidth, renderTextureHeight);

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
  SetTargetFPS(60); // Set our game to run at 60 frames-per-second
  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!window.ShouldClose()) // Detect window close button or ESC key
  {
    UpdateDrawFrame();
  }
#endif

  return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void)
{

  // Game World Drawing
  BeginTextureMode(target);

  ClearBackground(RAYWHITE);

  DrawText("Congrats! You created your first raylib-cpp window!", 160, 200, 20,
           LIGHTGRAY);

  EndTextureMode();

  // Draw Texture To Screen
  BeginDrawing();

  ClearBackground(BLACK);

  float scaleX = (float)GetScreenWidth() / renderTextureWidth;
  float scaleY = (float)GetScreenHeight() / renderTextureHeight;
  float scale = (scaleX < scaleY) ? scaleX : scaleY;

  int scaledWidth = (int)(renderTextureWidth * scale);
  int scaledHeight = (int)(renderTextureHeight * scale);

  int offsetX = (GetScreenWidth() - scaledWidth) / 2;
  int offsetY = (GetScreenHeight() - scaledHeight) / 2;

  DrawTexturePro(
      target.texture,
      Rectangle{0, 0, (float)target.texture.width, (float)-target.texture.height},        // Source rectangle (flip vertically)
      Rectangle{(float)offsetX, (float)offsetY, (float)scaledWidth, (float)scaledHeight}, // Destination rectangle
      Vector2{0, 0},                                                                      // Origin at top-left
      0.0f,                                                                               // No rotation
      WHITE                                                                               // Tint
  );

  EndDrawing();
  //----------------------------------------------------------------------------------
}
