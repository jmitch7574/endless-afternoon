#include "raylib-cpp.hpp"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif
#include <scene_manager.h>

SceneManager sceneManager;

int renderTextureWidth = 1920;
int renderTextureHeight = 1080;
RenderTexture2D target;

void UpdateDrawFrame(void); // Update and Draw one frame

// Initialisation
int main() {
  raylib::Window window(1920, 1080, "raylib-cpp [core] example - basic window");

  ToggleFullscreen();
  target = LoadRenderTexture(renderTextureWidth, renderTextureHeight);

  sceneManager = SceneManager();

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else

  SetTargetFPS(60);

  while (!window.ShouldClose()) {
    // Render Texture - Actual Game
    BeginTextureMode(target);

    UpdateDrawFrame();

    EndTextureMode();

    // Flip Render Texture to Any Resolution
    BeginDrawing();

    ClearBackground(BLACK);

    float scaleX = (float)GetScreenWidth() / renderTextureWidth;
    float scaleY = (float)GetScreenHeight() / renderTextureHeight;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;

    int scaledWidth = (int)(renderTextureWidth * scale);
    int scaledHeight = (int)(renderTextureHeight * scale);

    int offsetX = (GetScreenWidth() - scaledWidth) / 2;
    int offsetY = (GetScreenHeight() - scaledHeight) / 2;

    DrawTexturePro(target.texture,
                   Rectangle{0, 0, (float)target.texture.width,
                             (float)-target.texture
                                 .height}, // Source rectangle (flip vertically)
                   Rectangle{(float)offsetX, (float)offsetY, (float)scaledWidth,
                             (float)scaledHeight}, // Destination rectangle
                   Vector2{0, 0},                  // Origin at top-left
                   0.0f,                           // No rotation
                   WHITE                           // Tint
    );

    EndDrawing();
    UpdateDrawFrame();
  }

#endif

  return 0;
}

// Enum Scene Logic Here
void UpdateDrawFrame(void) {
  sceneManager.Update();
  sceneManager.Draw();
}
