#pragma once
#include "raylib.h"

class Resources
{
public:
  static Sound s_heartbeat;
  static Sound s_tocks[];
  static Sound s_whooshes[];
  static Sound s_slam;
  static Sound s_spinMove;

  static void Load();
  static Sound GetHeartbeat();
  static void UpdateHeartbeatSound(float value);
  static Sound GetTock();
  static Sound GetWhoosh();
  static Sound GetSlam();
  static Sound GetSpin();
};