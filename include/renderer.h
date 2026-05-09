#pragma once
#include "raylib.h"

#define RENDER_TEXTURE_WIDTH  1920
#define RENDER_TEXTURE_HEIGHT 1080

class Renderer {
public:
  static void InitRenderTexture();
  static void BlipRenderTexture();
};

extern RenderTexture2D rt_RenderTexture;