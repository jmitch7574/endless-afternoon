#pragma once

#include "raylib.h"
#include "renderer.h"

// Grid Constants
#define CELL_SIZE 45.0f
#define CELL_COUNT 21
#define GRID_CENTER Vector2(RENDER_TEXTURE_WIDTH / 2.0f, RENDER_TEXTURE_HEIGHT / 2.0f)
#define GRID_TOTAL_SIZE_PX (CELL_SIZE * CELL_COUNT)

class ArenaManager
{
  public:
	// Grid Helper Functions
	static Vector2 GridPositionToWorld(Vector2 GridPos);
	static bool IsValidGridPosition(Vector2 GridPos);
	static float DistanceToOctagonEdge(float angleDeg);

	// Arena Rendering Functions
	static void DrawLevelGrid();
	static void DrawOctagonBoundary();
	static void MaskOutsideOctagon();
	static Vector2 ClockMarkerPosition(int hour);
	static Vector2 ClockMarkerOutwardDirection(int hour);
	static void DrawClockMarker(int hour, Vector2 position, Color color);
	static void DrawClockMarkers(Color color = WHITE);
};
