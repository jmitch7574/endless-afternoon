#include "renderer.h"
#include "raylib.h"
#include "screen_shake.h"

RenderTexture2D rt_RenderTexture;

void Renderer::InitRenderTexture()
{
  rt_RenderTexture = LoadRenderTexture(RENDER_TEXTURE_WIDTH, RENDER_TEXTURE_HEIGHT);
}

void Renderer::BlipRenderTexture()
{
    ClearBackground(BLACK);

    float scaleX = (float)GetScreenWidth() / RENDER_TEXTURE_WIDTH;
    float scaleY = (float)GetScreenHeight() / RENDER_TEXTURE_HEIGHT;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;

    int scaledWidth = (int)(RENDER_TEXTURE_WIDTH * scale);
    int scaledHeight = (int)(RENDER_TEXTURE_HEIGHT * scale);

    int offsetX = (GetScreenWidth() - scaledWidth) / 2;
    int offsetY = (GetScreenHeight() - scaledHeight) / 2;
    const Vector2 shakeOffset = ScreenShake::GetOffset();

    DrawTexturePro(rt_RenderTexture.texture,
                   Rectangle{0, 0, (float)rt_RenderTexture.texture.width,
                                    (float)-rt_RenderTexture.texture
                                    .height}, // Source rectangle (flip vertically)
                   Rectangle{(float)offsetX + shakeOffset.x * scale, (float)offsetY + shakeOffset.y * scale, (float)scaledWidth,
                             (float)scaledHeight}, // Destination rectangle
                   Vector2{0, 0},                  // Origin at top-left
                   0.0f,                           // No rotation
                   WHITE                           // Tint
    );
}
