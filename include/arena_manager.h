#pragma once

#include "raylib.h"

// Grid Constants
#define CELL_SIZE 45.0f
#define CELL_COUNT 21

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
