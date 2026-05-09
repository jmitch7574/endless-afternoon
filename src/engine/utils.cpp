#include "utils.h"
#include "raylib-cpp.hpp"

float Utils::Vector2ToAngle(Vector2 vec) 
{ 
  vec = Vector2Normalize(vec);
  
  return atan2(vec.y, vec.x) * 180 / PI;
}

Vector2 Utils::AngleToVector2(float angle) 
{ 
  float rotationRadians = angle * DEG2RAD;

  Vector2 direction = Vector2(cosf(rotationRadians), sinf(rotationRadians));

  return direction;
}

Vector2 Utils::BezierLerp(Vector2 p1, Vector2 p2, Vector2 c1, Vector2 c2, float t) 
{
  Vector2 a = Vector2Lerp(p1, c1, t);
  Vector2 b = Vector2Lerp(c1, c2, t);
  Vector2 c = Vector2Lerp(c2, p2, t);
  Vector2 d = Vector2Lerp(a, b, t);
  Vector2 e = Vector2Lerp(b, c, t);
  return Vector2Lerp(d, e, t);
}
