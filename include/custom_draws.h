#include "raylib.h"

class CustomDraws {
public:
  static void DrawArrow(Vector2 baseOrigin, Vector2 direction, float length, float thickness, float fletchLength, float fletchAngle, Color color);
  static void DrawArrow(Vector2 baseOrigin, float rotation, float length, float thickness, float fletchLength, float fletchAngle, Color color);
};