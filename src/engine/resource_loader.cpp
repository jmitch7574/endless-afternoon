#include "resource_loader.h"
#include "raylib.h"
#include <iostream>


Sound Resources::s_heartbeat;
Sound Resources::s_tocks[6];
Sound Resources::s_whooshes[6];
Sound Resources::s_slam;
Sound Resources::s_spinMove;


void Resources::Load() 
{
  Resources::s_heartbeat = LoadSound("resources/sfx/heart_1.wav");
  Resources::s_slam = LoadSound("resources/sfx/slam.wav");
  Resources::s_spinMove = LoadSound("resources/sfx/spin_move.wav");

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
