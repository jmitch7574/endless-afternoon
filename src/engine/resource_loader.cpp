#include "resource_loader.h"
#include "raylib.h"
#include <iostream>


Sound Resources::s_heartbeat;
Sound Resources::s_tocks[6];
Sound Resources::s_whooshes[6];
Sound Resources::s_slam;
Sound Resources::s_spinMove;
Sound Resources::s_clockSpinTock;
Sound Resources::s_enemyHurt;
Sound Resources::s_playerHurt;
Sound Resources::s_numeralsLaunch;
Sound Resources::s_greenLightUp;


void Resources::Load() 
{
  Resources::s_heartbeat = LoadSound("resources/sfx/heart_1.wav");
  Resources::s_slam = LoadSound("resources/sfx/slam.wav");
  Resources::s_spinMove = LoadSound("resources/sfx/spin_move.wav");
  Resources::s_clockSpinTock = LoadSound("resources/sfx/clock_spin_tock.wav");
  Resources::s_enemyHurt = LoadSound("resources/sfx/enemy_hurt.wav");
  Resources::s_playerHurt = LoadSound("resources/sfx/player_hurt.wav");
  Resources::s_numeralsLaunch = LoadSound("resources/sfx/numerals_launch.wav");
  Resources::s_greenLightUp = LoadSound("resources/sfx/green_light_shine.wav");

  SetSoundVolume(s_clockSpinTock, 0.75f);

  for (int i = 0; i < 6; i++)
  {
    s_tocks[i] = LoadSound(TextFormat("resources/sfx/tock_%i.wav", i + 1));
    s_whooshes[i] = LoadSound(TextFormat("resources/sfx/whoosh_%i.wav", i + 1));
    SetSoundPitch(s_whooshes[i], 0.5f); 
  }
}

Sound Resources::GetHeartbeat() 
{ 
  return s_heartbeat;
}

void Resources::UpdateHeartbeatSound(float value) 
{
  SetSoundVolume(s_heartbeat, value);
}

Sound Resources::GetTock() 
{
  return s_tocks[GetRandomValue(0, 5)];
}

Sound Resources::GetWhoosh() 
{
  return s_whooshes[GetRandomValue(0, 5)];
}

Sound Resources::GetSlam() 
{
  return s_slam;
}

Sound Resources::GetSpin() 
{
  return s_spinMove;
}

Sound Resources::GetClockSpinTock()
{
  return s_clockSpinTock;
}

Sound Resources::GetEnemyHurt()
{
  return s_enemyHurt;
}

Sound Resources::GetPlayerHurt()
{
  return s_playerHurt;
}

Sound Resources::GetNumeralsLaunch()
{
  return s_numeralsLaunch;
}

Sound Resources::GetGreenLightUp()
{
  return s_greenLightUp;
}