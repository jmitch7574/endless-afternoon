#include "grid_manager.h"
#include "raylib-cpp.hpp"
#include "renderer.h"

#define RADIUS 500
#define APOTHEM (RADIUS * 0.92388f)
#define HALF_FLAT (RADIUS * 0.38268f)

#define CENTER_POINT Vector2(RENDER_TEXTURE_WIDTH / 2, RENDER_TEXTURE_HEIGHT / 2)

#define BB_LEFT   CENTER_POINT.x - APOTHEM
#define BB_RIGHT  CENTER_POINT.x + APOTHEM
#define BB_TOP    CENTER_POINT.y - APOTHEM
#define BB_BOTTOM CENTER_POINT.y + APOTHEM

void GridManager::DrawLevelGrid() {
  
  const float endX = START_X + TOTAL_SIZE;
  const float endY = START_Y + TOTAL_SIZE;
  const Color gridColor = Fade(WHITE, 0.3f);

  for (int i = 0; i <= CELL_COUNT; i++) {
    float offset = i * CELL_SIZE;
    DrawLineV(Vector2{START_X + offset, START_Y}, Vector2{START_X + offset, endY},
              gridColor);
    DrawLineV(Vector2{START_X, START_Y + offset}, Vector2{endX, START_Y + offset},
              gridColor);
  }
}

Vector2 GridManager::GridPositionToWorld(Vector2 GridPos)
{
  return Vector2Add(Vector2(START_X + CELL_SIZE / 2, START_Y + CELL_SIZE / 2), Vector2Scale(GridPos, CELL_SIZE));
}

bool GridManager::IsValidGridPosition(Vector2 worldPosition) {
  float dx = fabsf(worldPosition.x - CENTER_POINT.x);
  float dy = fabsf(worldPosition.y - CENTER_POINT.y);

  // 1. bounding square
  if (dx > APOTHEM || dy > APOTHEM)
    return false;

  // 2. cut off corners (this replaces the triangles)
  if (dx + dy > APOTHEM + HALF_FLAT)
    return false;

  return true;
}


void GridManager::DrawLevelBoundary() {
  DrawPolyLinesEx(Vector2{960, 540}, 8, 500.0f, 22.5f, 4.0f, WHITE);
}


void GridManager::MaskOutsideOctagon() {
  // Black out everything outside the octagon. Two parts:
  //  1. Four rectangles outside the octagon's bounding box
  //  (top/bottom/left/right).
  //  2. Four triangles inside the bounding box but outside the octagon's
  //     diagonal edges (the bounding-box corners).

  // Strips outside the bounding box
  // TOP
DrawRectangleRec(
    Rectangle{0, 0, RENDER_TEXTURE_WIDTH, BB_TOP},
    BLACK
);

// BOTTOM
DrawRectangleRec(
    Rectangle{0, RENDER_TEXTURE_HEIGHT - BB_BOTTOM, RENDER_TEXTURE_WIDTH, BB_BOTTOM},
    BLACK
);

// LEFT
DrawRectangleRec(
    Rectangle{0, 0, BB_LEFT, RENDER_TEXTURE_HEIGHT},
    BLACK
);

// RIGHT
DrawRectangleRec(
    Rectangle{RENDER_TEXTURE_WIDTH - BB_RIGHT, 0, BB_RIGHT, RENDER_TEXTURE_HEIGHT},
    BLACK
);

  // Corner triangles (vertices in screen-CCW order so raylib renders them).
  // Top-right
  DrawTriangle(Vector2{CENTER_POINT.x + APOTHEM, CENTER_POINT.y - APOTHEM},
               Vector2{CENTER_POINT.x + HALF_FLAT, CENTER_POINT.y - APOTHEM},
               Vector2{CENTER_POINT.x + APOTHEM, CENTER_POINT.y - HALF_FLAT}, BLACK);
  // Top-left
  DrawTriangle(Vector2{CENTER_POINT.x - APOTHEM, CENTER_POINT.y - APOTHEM},
               Vector2{CENTER_POINT.x - APOTHEM, CENTER_POINT.y - HALF_FLAT},
               Vector2{CENTER_POINT.x - HALF_FLAT, CENTER_POINT.y - APOTHEM}, BLACK);
  // Bottom-right
  DrawTriangle(Vector2{CENTER_POINT.x + APOTHEM, CENTER_POINT.y + APOTHEM},
               Vector2{CENTER_POINT.x + APOTHEM, CENTER_POINT.y + HALF_FLAT},
               Vector2{CENTER_POINT.x + HALF_FLAT, CENTER_POINT.y + APOTHEM}, BLACK);
  // Bottom-left
  DrawTriangle(Vector2{CENTER_POINT.x - APOTHEM, CENTER_POINT.y + APOTHEM},
               Vector2{CENTER_POINT.x - HALF_FLAT, CENTER_POINT.y + APOTHEM},
               Vector2{CENTER_POINT.x - APOTHEM, CENTER_POINT.y + HALF_FLAT}, BLACK);
}


void GridManager::DrawClockMarkers() {
  // 12 filled white circles at the clock hour positions, sat just outside
  // the octagon. The octagon's edge distance varies with angle (apothem
  // along the cardinals, longer along the diagonals), so each marker is
  // positioned relative to the actual edge it's nearest to.
  
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
    const float boundaryDist = APOTHEM / cosf(angle - nearestEdgeAngle);

    const Vector2 pos = {CENTER_POINT.x + (boundaryDist + gap) * cosf(angle),
                         CENTER_POINT.y + (boundaryDist + gap) * sinf(angle)};
    DrawCircleV(pos, markerRadius, WHITE);
  }
}