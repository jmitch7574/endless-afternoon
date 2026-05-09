#include "raylib.h"

class Utils {
public:
  static float Vector2ToAngle(Vector2 vec);
  static Vector2 AngleToVector2(float angle);
  static Vector2 BezierLerp(Vector2 p1, Vector2 p2, Vector2 c1, Vector2 c2, float t);
  static bool LineIntersectsCircle(Vector2 p1, Vector2 p2, Vector2 center, float radius);
  static float NormalizeAngle(float a);
};