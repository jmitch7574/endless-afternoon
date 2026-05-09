#include "grid_manager.h"
#include "raylib-cpp.hpp"

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

bool GridManager::IsValidGridPosition(Vector2 GridPos) {
  return true;
}