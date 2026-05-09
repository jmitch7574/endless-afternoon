#pragma once

#include "raylib.h"

#define CELL_SIZE 50
#define CELL_COUNT 32

#define TOTAL_SIZE (CELL_SIZE * CELL_COUNT)

#define START_X (960 - TOTAL_SIZE / 2.0f)
#define START_Y (540 - TOTAL_SIZE / 2.0f)

class GridManager {
public:
  static void DrawLevelGrid();
  static Vector2 GridPositionToWorld(Vector2 GridPos);
  bool IsValidGridPosition(Vector2 GridPos);
};
