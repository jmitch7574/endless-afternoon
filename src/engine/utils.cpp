#include "utils.h"
#include "raylib-cpp.hpp"
#include <chrono>

void Utils::SeedRandom()
{
	const auto now = std::chrono::system_clock::now();
	const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
	SetRandomSeed((unsigned int)seconds);
}

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

bool Utils::LineIntersectsCircle(Vector2 p1, Vector2 p2, Vector2 center, float radius)
{
    Vector2 d = Vector2Subtract(p2, p1);       // segment direction
    Vector2 f = Vector2Subtract(p1, center);   // vector from circle to line start

    float a = Vector2DotProduct(d, d);
    float b = 2 * Vector2DotProduct(f, d);
    float c = Vector2DotProduct(f, f) - radius * radius;

    float discriminant = b*b - 4*a*c;

    if (discriminant < 0)
    {
        // no intersection
        return false;
    }

    discriminant = sqrtf(discriminant);

    float t1 = (-b - discriminant) / (2*a);
    float t2 = (-b + discriminant) / (2*a);

    // check if either t is within [0,1] => intersection along the segment
    if ((t1 >= 0 && t1 <= 1) || (t2 >= 0 && t2 <= 1))
        return true;

    return false;
}

float Utils::NormalizeAngle(float a) 
{
    a = fmod(a, 360.0f);
    if (a < 0) a += 360.0f;
    return a;
}
