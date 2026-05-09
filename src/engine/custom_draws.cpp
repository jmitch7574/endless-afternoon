#include "custom_draws.h"
#include "raylib-cpp.hpp"

void CustomDraws::DrawArrow(Vector2 baseOrigin, Vector2 direction, float length, float thickness, float fletchLength, float fletchAngle, Color color)
{
  fletchAngle = 180 - fletchAngle;
  fletchAngle = fletchAngle * (PI / 180.0f);
  
  Vector2 endPos = Vector2Add(baseOrigin,Vector2Scale(direction, length));

  // Main Length
  DrawLineEx(baseOrigin, endPos, thickness, color);

  endPos = Vector2Subtract(endPos, Vector2Scale(direction, thickness / 2.5f));


  Vector2 leftFletchDirection = {
    (direction.x * cosf(fletchAngle)) - (direction.y * sinf(fletchAngle)),
    (direction.x * sinf(fletchAngle)) + (direction.y * cosf(fletchAngle))
  };

  
  Vector2 rightFletchDirection = {
    (direction.x * cosf(-fletchAngle)) - (direction.y * sinf(-fletchAngle)),
    (direction.x * sinf(-fletchAngle)) + (direction.y * cosf(-fletchAngle))
  };

  
  DrawLineEx(endPos, Vector2Add(endPos, Vector2Scale(leftFletchDirection, fletchLength)), thickness, color);
  DrawLineEx(endPos, Vector2Add(endPos, Vector2Scale(rightFletchDirection, fletchLength)), thickness, color);
}

// Rotation of 0 = Vector2.up
void CustomDraws::DrawArrow(Vector2 baseOrigin, float rotation, float length, float thickness, float fletchLength, float fletchAngle, Color color)
{
  float rotationRadians = rotation * DEG2RAD;

  Vector2 direction = Vector2(cosf(rotationRadians), sinf(rotationRadians));

  direction = Vector2Normalize(direction);

  DrawArrow(baseOrigin, direction, length, thickness, fletchLength, fletchAngle, color);
}
