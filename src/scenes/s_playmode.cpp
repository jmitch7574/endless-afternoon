#include "scene.h"
#include <cmath>
#include <raylib-cpp.hpp>
#include "grid_manager.h"

PlayMode::PlayMode()
    : player(Vector2{10, 10}), enemy(Vector2{1085, 415}),
      minuteHand(Vector2{960, 540}, -90.0f, 440.0f, 8.0f, WHITE),
      hourHand(Vector2{960, 540}, 0.0f, 280.0f, 12.0f, WHITE) {}
PlayMode::~PlayMode(void) {}

void PlayMode::Update() {
  player.Update();
  enemy.Update();
  minuteHand.Update();
  hourHand.Update();
}
void PlayMode::Draw() {
  ClearBackground(BLACK);
  GridManager::DrawLevelGrid();
  MaskOutsideOctagon();
  DrawLevelBoundary();
  DrawClockMarkers();
  hourHand.Draw();
  minuteHand.Draw();
  player.Draw();
  enemy.Draw();

#ifndef NDEBUG
  DrawText(TextFormat("Player Pos: %f, %f", player.gridPosition.x, player.gridPosition.y), 20, 20, 20, WHITE);
#endif
}

void PlayMode::DrawLevelBoundary() {
  DrawPolyLinesEx(Vector2{960, 540}, 8, 500.0f, 22.5f, 4.0f, WHITE);
}

void PlayMode::DrawClockMarkers() {
  // 12 filled white circles at the clock hour positions, sat just outside
  // the octagon. The octagon's edge distance varies with angle (apothem
  // along the cardinals, longer along the diagonals), so each marker is
  // positioned relative to the actual edge it's nearest to.
  const float radius = 500.0f;
  const float apothem = radius * 0.92388f; // cos(22.5°)
  const Vector2 c = {960, 540};
  const float gap = 30.0f;          // how far past the edge
  const float markerRadius = 12.0f; // size of each clock dot

  for (int h = 1; h <= 12; h++) {
    // Hour h sits at angle (h * 30° - 90°): 12 at top, 3 at right, etc.
    const float angle = (h * 30.0f - 90.0f) * DEG2RAD;

    // Distance from center to the octagon edge along this angle.
    // Edge midpoints are at multiples of 45°; the relevant edge is the
    // one whose midpoint angle is nearest to `angle`.
    const float quarterPi = PI / 4.0f;
    const float nearestEdgeAngle = roundf(angle / quarterPi) * quarterPi;
    const float boundaryDist = apothem / cosf(angle - nearestEdgeAngle);

    const Vector2 pos = {c.x + (boundaryDist + gap) * cosf(angle),
                         c.y + (boundaryDist + gap) * sinf(angle)};
    DrawCircleV(pos, markerRadius, WHITE);
  }
}

void PlayMode::MaskOutsideOctagon() {
  // Black out everything outside the octagon. Two parts:
  //  1. Four rectangles outside the octagon's bounding box
  //  (top/bottom/left/right).
  //  2. Four triangles inside the bounding box but outside the octagon's
  //     diagonal edges (the bounding-box corners).
  const float radius = 500.0f;
  const float apothem = radius * 0.92388f;  // cos(22.5°)
  const float halfFlat = radius * 0.38268f; // sin(22.5°)
  const Vector2 c = {960, 540};
  const float screenW = 1920.0f;
  const float screenH = 1080.0f;
  const float bbLeft = c.x - apothem;
  const float bbRight = c.x + apothem;
  const float bbTop = c.y - apothem;
  const float bbBottom = c.y + apothem;

  // Strips outside the bounding box
  DrawRectangleRec(Rectangle{0, 0, screenW, bbTop}, BLACK);
  DrawRectangleRec(Rectangle{0, bbBottom, screenW, screenH - bbBottom}, BLACK);
  DrawRectangleRec(Rectangle{0, bbTop, bbLeft, bbBottom - bbTop}, BLACK);
  DrawRectangleRec(
      Rectangle{bbRight, bbTop, screenW - bbRight, bbBottom - bbTop}, BLACK);

  // Corner triangles (vertices in screen-CCW order so raylib renders them).
  // Top-right
  DrawTriangle(Vector2{c.x + apothem, c.y - apothem},
               Vector2{c.x + halfFlat, c.y - apothem},
               Vector2{c.x + apothem, c.y - halfFlat}, BLACK);
  // Top-left
  DrawTriangle(Vector2{c.x - apothem, c.y - apothem},
               Vector2{c.x - apothem, c.y - halfFlat},
               Vector2{c.x - halfFlat, c.y - apothem}, BLACK);
  // Bottom-right
  DrawTriangle(Vector2{c.x + apothem, c.y + apothem},
               Vector2{c.x + apothem, c.y + halfFlat},
               Vector2{c.x + halfFlat, c.y + apothem}, BLACK);
  // Bottom-left
  DrawTriangle(Vector2{c.x - apothem, c.y + apothem},
               Vector2{c.x - halfFlat, c.y + apothem},
               Vector2{c.x - apothem, c.y + halfFlat}, BLACK);
}