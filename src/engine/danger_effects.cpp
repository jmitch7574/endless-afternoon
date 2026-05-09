#include "danger_effects.h"
#include "raylib.h"
#include "renderer.h"
#include "scene.h"
#include "resource_loader.h"

float targetOpacity;
float opacity;

float targetVolume;
float volume = 0;

DangerEffects::DangerEffects() {
  targetVolume = 0;
  volume = 0;

  targetOpacity = 0;
  opacity = 0;
}

float SFXTimer;



void DangerEffects::Update() 
{ 
  targetOpacity = targetOpacity = 128 - (playScene->player.GetHealth() / 75.0f) * 128; 
  targetOpacity = std::max(targetOpacity, 0.0f);
  opacity = Lerp(opacity, targetOpacity, 0.01f);

  SFXTimer += GetFrameTime();

  targetVolume = 0.8f - playScene->player.GetHealth() / 80.0f;
  targetVolume = std::max(targetVolume, 0.0f);

  volume = Lerp(volume, targetVolume, 0.01f);

  SetSoundVolume(Resources::s_heartbeat, volume);

  for (int i = 0; i< 8; i++)
  {
    SetSoundVolume(Resources::s_tocks[i], volume);
  }

  if (SFXTimer > 2)
  {
    PlaySound(Resources::s_heartbeat);
    //PlaySound(Resources::s_tocks[GetRandomValue(0, 8)]);

    SFXTimer = 0;
  }
}

void DangerEffects::Draw() 
{
  DrawRectangle(0, 0, RENDER_TEXTURE_WIDTH, RENDER_TEXTURE_HEIGHT, Color(45, 0, 0, (unsigned char)opacity));
}
