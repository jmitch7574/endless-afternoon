#include "entity.h"
#include <cmath>

ClockHand::ClockHand(raylib::Vector2 pivot, float angleDeg, float length,
                     float thickness, Color color)
    : Entity(pivot), angleDeg(angleDeg), length(length), thickness(thickness),
      color(color) {}
ClockHand::~ClockHand(void) {}

void ClockHand::Update() {}
void ClockHand::Draw() {
  const float angleRad = angleDeg * DEG2RAD;
  const Vector2 endPoint = {position.x + length * cosf(angleRad),
                            position.y + length * sinf(angleRad)};
  DrawLineEx(position, endPoint, thickness, color);
}
