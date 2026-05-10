#pragma once

class DangerEffects{
public:
  DangerEffects();
  void Update();
  void Draw();
  static void DisplayHurt();
  static DangerEffects* singleton;
  
  float targetOpacity;
  float opacity;

  float targetVolume;
  float volume = 0;
};