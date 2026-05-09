#pragma once
#include "raylib.h"

class Resources
{
public:
  static Sound s_heartbeat;
  static Sound s_tocks[8];
  static void Load();
};