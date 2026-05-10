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
  static Sound s_clockSpinTock;
  static Sound s_enemyHurt;
  static Sound s_playerHurt;
  static Sound s_numeralsLaunch;
  static Sound s_greenLightUp;

  static void Load();
  static Sound GetHeartbeat();
  static void UpdateHeartbeatSound(float value);
  static Sound GetTock();
  static Sound GetWhoosh();
  static Sound GetSlam();
  static Sound GetSpin();
  static Sound GetClockSpinTock();
  static Sound GetEnemyHurt();
  static Sound GetPlayerHurt();
  static Sound GetNumeralsLaunch();
  static Sound GetGreenLightUp();
};