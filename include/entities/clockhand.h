#pragma once

#include "entity.h"

class ClockHand : public Entity
{
  public:
	float GetAngle();
	ClockHand(raylib::Vector2 pivot, float angleDeg, float length, float thickness, Color color);
	~ClockHand(void);

	void Update() override;
	void Draw() override;
	void Advance();
	void BeginBigDeadlySpin();
	Vector2 GetLargeExtendedPoint();

	bool activated = false;

  protected:
	float angleDeg;
	float length;
	float thickness;
	Color color;

	float bigDeadlySpinTime = 0;
	float bigDeadlySpinCoefficient = 40;
	bool inBigDeadlySpin = false;

	float advanceTime = 0;
	float advanceDuration = 1.2f;
	float advanceStartAngleDeg = 0;
	float advanceTargetAngleDeg = 0;
	float targetAngleDeg = 0;
	bool isAdvancing = false;
};
