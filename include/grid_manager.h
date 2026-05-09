#pragma once

#include "raylib.h"

#define CELL_SIZE 48
#define CELL_COUNT 33

#define TOTAL_SIZE (CELL_SIZE * CELL_COUNT)

#define START_X (960 - TOTAL_SIZE / 2.0f)
#define START_Y (540 - TOTAL_SIZE / 2.0f)

#define RADIUS 500
#define APOTHEM (RADIUS * 0.92388f)
#define HALF_FLAT (RADIUS * 0.38268f)

#define CENTER_POINT Vector2(RENDER_TEXTURE_WIDTH / 2, RENDER_TEXTURE_HEIGHT / 2)

#define BB_LEFT   CENTER_POINT.x - APOTHEM
#define BB_RIGHT  CENTER_POINT.x + APOTHEM
#define BB_TOP    CENTER_POINT.y - APOTHEM
#define BB_BOTTOM CENTER_POINT.y + APOTHEM

class GridManager {
public:
  static void DrawLevelGrid();
  static Vector2 GridPositionToWorld(Vector2 GridPos);
  static bool IsValidGridPosition(Vector2 GridPos);
  static void DrawLevelBoundary();
  static void MaskOutsideOctagon();
  static void DrawClockMarkers();
};
