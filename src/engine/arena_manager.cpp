#include "arena_manager.h"
#include "raymath.h"
#include "renderer.h"
#include <cmath>
#include <cstring>

// Grid layout
#define GRID_TOTAL_SIZE_PX (CELL_SIZE * CELL_COUNT)
#define GRID_CENTER Vector2(RENDER_TEXTURE_WIDTH / 2.0f, RENDER_TEXTURE_HEIGHT / 2.0f)
#define GRID_ORIGIN_X (GRID_CENTER.x - GRID_TOTAL_SIZE_PX / 2.0f)
#define GRID_ORIGIN_Y (GRID_CENTER.y - GRID_TOTAL_SIZE_PX / 2.0f)
#define GRID_HALF_SIZE (GRID_TOTAL_SIZE_PX / 2.0f)

// Octagon shape
#define OCTAGON_FLAT_EDGE_CELLS 9
#define OCTAGON_CORNER_CUT_CELLS 6
#define OCTAGON_HALF_FLAT_EDGE_PX ((OCTAGON_FLAT_EDGE_CELLS * CELL_SIZE) / 2.0f)

#define ROMAN_GLYPH_WIDTH 18.0f
#define ROMAN_GLYPH_HEIGHT 46.0f
#define ROMAN_GLYPH_GAP 7.0f
#define ROMAN_STROKE_THICKNESS 5.0f
#define ROMAN_OUTSIDE_GAP 30.0f

// Octagon/grid masking bounds
#define BOUNDS_LEFT (GRID_CENTER.x - GRID_HALF_SIZE)
#define BOUNDS_RIGHT (GRID_CENTER.x + GRID_HALF_SIZE)
#define BOUNDS_TOP (GRID_CENTER.y - GRID_HALF_SIZE)
#define BOUNDS_BOTTOM (GRID_CENTER.y + GRID_HALF_SIZE)

// Helper functions for drawing the octagon boundary
namespace
{
struct OctagonBounds
{
	float left, right, top, bottom;
	float flatLeft, flatRight, flatTop, flatBottom;
};

OctagonBounds GetOctagonBounds()
{
	const float cornerSize = OCTAGON_CORNER_CUT_CELLS * CELL_SIZE;
	const float left = GRID_ORIGIN_X;
	const float right = GRID_ORIGIN_X + GRID_TOTAL_SIZE_PX;
	const float top = GRID_ORIGIN_Y;
	const float bottom = GRID_ORIGIN_Y + GRID_TOTAL_SIZE_PX;
	return {left, right, top, bottom, left + cornerSize, right - cornerSize, top + cornerSize, bottom - cornerSize};
}

Vector2 LocalNumeralPoint(Vector2 center, Vector2 outward, float x, float y)
{
	const Vector2 right = Vector2{-outward.y, outward.x};
	return Vector2Add(center, Vector2Add(Vector2Scale(right, x), Vector2Scale(outward, -y)));
}

void DrawNumeralStroke(Vector2 center, Vector2 outward, float x1, float y1, float x2, float y2, float thickness,
					   Color color)
{
	const Vector2 start = LocalNumeralPoint(center, outward, x1, y1);
	const Vector2 end = LocalNumeralPoint(center, outward, x2, y2);
	const Color shadow = Color{0, 0, 0, (unsigned char)((float)color.a * 0.75f)};

	DrawLineEx(start, end, thickness + 3.0f, shadow);
	DrawLineEx(start, end, thickness, color);
}

void DrawRomanGlyph(char glyph, Vector2 center, Vector2 outward, float width, float height, float thickness, Color color)
{
	const float halfWidth = width * 0.5f;
	const float halfHeight = height * 0.5f;

	switch (glyph)
	{
	case 'I':
		DrawNumeralStroke(center, outward, 0.0f, -halfHeight, 0.0f, halfHeight, thickness, color);
		DrawNumeralStroke(center, outward, -halfWidth * 0.48f, -halfHeight, halfWidth * 0.48f, -halfHeight,
						  thickness, color);
		DrawNumeralStroke(center, outward, -halfWidth * 0.48f, halfHeight, halfWidth * 0.48f, halfHeight, thickness,
						  color);
		break;
	case 'V':
		DrawNumeralStroke(center, outward, -halfWidth, -halfHeight, 0.0f, halfHeight, thickness, color);
		DrawNumeralStroke(center, outward, halfWidth, -halfHeight, 0.0f, halfHeight, thickness, color);
		break;
	case 'X':
		DrawNumeralStroke(center, outward, -halfWidth, -halfHeight, halfWidth, halfHeight, thickness, color);
		DrawNumeralStroke(center, outward, halfWidth, -halfHeight, -halfWidth, halfHeight, thickness, color);
		break;
	default:
		break;
	}
}

void DrawPrimitiveRomanNumeral(const char *label, Vector2 center, Vector2 outward, Color color)
{
	const int glyphCount = (int)std::strlen(label);
	const float totalWidth = ROMAN_GLYPH_WIDTH * (float)glyphCount + ROMAN_GLYPH_GAP * (float)(glyphCount - 1);
	const Vector2 right = Vector2{-outward.y, outward.x};

	for (int i = 0; i < glyphCount; i++)
	{
		const float xOffset =
			-totalWidth * 0.5f + ROMAN_GLYPH_WIDTH * 0.5f + (float)i * (ROMAN_GLYPH_WIDTH + ROMAN_GLYPH_GAP);
		const Vector2 glyphCenter = Vector2Add(center, Vector2Scale(right, xOffset));
		DrawRomanGlyph(label[i], glyphCenter, outward, ROMAN_GLYPH_WIDTH, ROMAN_GLYPH_HEIGHT, ROMAN_STROKE_THICKNESS,
					   color);
	}
}

float ClockMarkerAngleDeg(int hour) { return (float)hour * 30.0f - 90.0f; }

const char *ClockMarkerLabel(int hour)
{
	const char *romanNumerals[12] = {"I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI", "XII"};
	return romanNumerals[hour - 1];
}
} // namespace

// Convert grid position to world position
Vector2 ArenaManager::GridPositionToWorld(Vector2 GridPos)
{
	return Vector2Add(Vector2(GRID_ORIGIN_X + CELL_SIZE / 2, GRID_ORIGIN_Y + CELL_SIZE / 2),
					  Vector2Scale(GridPos, CELL_SIZE));
}

// Check if a world position is within the grid
bool ArenaManager::IsValidGridPosition(Vector2 worldPosition)
{
	float dx = fabsf(worldPosition.x - GRID_CENTER.x);
	float dy = fabsf(worldPosition.y - GRID_CENTER.y);

	if (dx > GRID_HALF_SIZE || dy > GRID_HALF_SIZE)
	{
		return false;
	}

	if (dx + dy + 5.0f > GRID_HALF_SIZE + OCTAGON_HALF_FLAT_EDGE_PX)
	{
		return false;
	}

	return true;
}

float ArenaManager::DistanceToOctagonEdge(float angleDeg)
{
	const float angle = angleDeg * DEG2RAD;
	const float x = fabsf(cosf(angle));
	const float y = fabsf(sinf(angle));
	const float diagonalLimit = GRID_HALF_SIZE + OCTAGON_HALF_FLAT_EDGE_PX;

	float distance = diagonalLimit / (x + y);

	if (x > 0.0f)
	{
		distance = fminf(distance, GRID_HALF_SIZE / x);
	}

	if (y > 0.0f)
	{
		distance = fminf(distance, GRID_HALF_SIZE / y);
	}

	return distance;
}

// Draw the level grid
void ArenaManager::DrawLevelGrid()
{
	const float endX = GRID_ORIGIN_X + GRID_TOTAL_SIZE_PX;
	const float endY = GRID_ORIGIN_Y + GRID_TOTAL_SIZE_PX;

	const Color gridColor = Fade(WHITE, 0.3f);

	for (int i = 0; i <= CELL_COUNT; i++)
	{
		float offset = i * CELL_SIZE;
		DrawLineV(Vector2{GRID_ORIGIN_X + offset, GRID_ORIGIN_Y}, Vector2{GRID_ORIGIN_X + offset, endY}, gridColor);
		DrawLineV(Vector2{GRID_ORIGIN_X, GRID_ORIGIN_Y + offset}, Vector2{endX, GRID_ORIGIN_Y + offset}, gridColor);
	}
}

// Draw the octagon boundary
void ArenaManager::DrawOctagonBoundary()
{
	const auto b = GetOctagonBounds();
	const Vector2 vertices[8] = {{b.flatLeft, b.top},	  {b.flatRight, b.top},	   {b.right, b.flatTop},
								 {b.right, b.flatBottom}, {b.flatRight, b.bottom}, {b.flatLeft, b.bottom},
								 {b.left, b.flatBottom},  {b.left, b.flatTop}};

	for (int i = 0; i < 8; i++)
	{
		DrawLineEx(vertices[i], vertices[(i + 1) % 8], 4.0f, WHITE);
	}
}

// Mask outside octagon
void ArenaManager::MaskOutsideOctagon()
{
	const auto b = GetOctagonBounds();

	DrawRectangleRec(Rectangle{0, 0, (float)RENDER_TEXTURE_WIDTH, BOUNDS_TOP}, BLACK);
	DrawRectangleRec(
		Rectangle{0, BOUNDS_BOTTOM, (float)RENDER_TEXTURE_WIDTH, (float)RENDER_TEXTURE_HEIGHT - BOUNDS_BOTTOM}, BLACK);
	DrawRectangleRec(Rectangle{0, 0, BOUNDS_LEFT, (float)RENDER_TEXTURE_HEIGHT}, BLACK);
	DrawRectangleRec(
		Rectangle{BOUNDS_RIGHT, 0, (float)RENDER_TEXTURE_WIDTH - BOUNDS_RIGHT, (float)RENDER_TEXTURE_HEIGHT}, BLACK);

	DrawTriangle(Vector2{b.right, b.top}, Vector2{b.flatRight, b.top}, Vector2{b.right, b.flatTop}, BLACK);
	DrawTriangle(Vector2{b.left, b.top}, Vector2{b.left, b.flatTop}, Vector2{b.flatLeft, b.top}, BLACK);
	DrawTriangle(Vector2{b.right, b.bottom}, Vector2{b.right, b.flatBottom}, Vector2{b.flatRight, b.bottom}, BLACK);
	DrawTriangle(Vector2{b.left, b.bottom}, Vector2{b.flatLeft, b.bottom}, Vector2{b.left, b.flatBottom}, BLACK);
}

// Draw the clock markers
Vector2 ArenaManager::ClockMarkerOutwardDirection(int hour)
{
	const float angle = ClockMarkerAngleDeg(hour) * DEG2RAD;

	return Vector2{cosf(angle), sinf(angle)};
}

Vector2 ArenaManager::ClockMarkerPosition(int hour)
{
	const float angleDeg = ClockMarkerAngleDeg(hour);
	const float boundaryDist = DistanceToOctagonEdge(angleDeg);
	const float outwardOffset = ROMAN_GLYPH_HEIGHT * 0.5f + ROMAN_OUTSIDE_GAP;
	const Vector2 outward = ClockMarkerOutwardDirection(hour);

	return Vector2{GRID_CENTER.x + (boundaryDist + outwardOffset) * outward.x,
				   GRID_CENTER.y + (boundaryDist + outwardOffset) * outward.y};
}

void ArenaManager::DrawClockMarker(int hour, Vector2 position, Color color)
{
	DrawPrimitiveRomanNumeral(ClockMarkerLabel(hour), position, ClockMarkerOutwardDirection(hour), color);
}

void ArenaManager::DrawClockMarkers(Color color)
{
	for (int h = 1; h <= 12; h++)
	{
		DrawClockMarker(h, ClockMarkerPosition(h), color);
	}
}
