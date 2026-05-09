#include "resource_loader.h"
#include "raylib.h"
#include <iostream>


Sound Resources::s_heartbeat;
Sound Resources::s_tocks[8];


void Resources::Load() 
{
  Resources::s_heartbeat = LoadSound("resources/sfx/heart_1.wav");
}