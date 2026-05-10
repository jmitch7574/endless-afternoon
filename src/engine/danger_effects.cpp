#include "danger_effects.h"
#include "raylib.h"
#include "renderer.h"
#include "scene.h"
#include "resource_loader.h"


DangerEffects::DangerEffects() {
  targetVolume = 0;
  volume = 0;

  targetOpacity = 0;
  opacity = 0;

  singleton = this;
}

float SFXTimer;


DangerEffects* DangerEffects::singleton = nullptr;

void DangerEffects::Update() 
{ 
  targetOpacity = targetOpacity = 128 - (playScene->player.GetHealth() / 75.0f) * 128; 
  targetOpacity = std::max(targetOpacity, 0.0f);
  opacity = Lerp(opacity, targetOpacity, 0.05f);

  SFXTimer += GetFrameTime();

  if (playScene->player.GetHealth() > 60)
  {
    targetVolume = 0;
  }
  else if (playScene->player.GetHealth() > 40)
  {
    targetVolume = 0.02f;
  }
  else if (playScene->player.GetHealth() > 30)
  {
    targetVolume = 0.2f;
  }
  else if (playScene->player.GetHealth() > 20)
  {
    targetVolume = 0.4f;
  }
  else
  {
    targetVolume = 0.8f;
  }

  volume = Lerp(volume, targetVolume, 0.001f);

  SetSoundVolume(Resources::s_heartbeat, volume);

  for (int i = 0; i< 8; i++)
  {
    SetSoundVolume(Resources::s_tocks[i], volume);
  }

  if (SFXTimer > 2 - targetVolume * 1.2f)
  {
    PlaySound(Resources::s_heartbeat);
    //PlaySound(Resources::s_tocks[GetRandomValue(0, 8)]);

    SFXTimer = 0;
  }
}

void DangerEffects::Draw() 
{
  DrawRectangle(0, 0, RENDER_TEXTURE_WIDTH, RENDER_TEXTURE_HEIGHT, Color(45, 0, 0, opacity));
  DrawText(TextFormat("Target Volume: %f", targetVolume), 20, 230, 20, WHITE);
}

void DangerEffects::DisplayHurt() 
{
  singleton->opacity = fmaxf(singleton->opacity, 80);
}
