#pragma once

#include "raylib.h"
#include <string>
#include "app_state.h"
#include "types.h"

Vector2 wpilibCoordsToPixels(
	float fieldX,
	float fieldY,
	float fieldWidth,
	float fieldHeight,
	float screenWidth,
	float screenHeight
);

Vector2 pixelsToWpilibCoords(
	Vector2 pixel,
	float fieldWidth,
	float fieldHeight,
	float screenWidth,
	float screenHeight
);

std::string replaceAll(std::string str, const std::string& from, const std::string& to);

std::string getCoordFmt(float f);

Segment GetSegmentPoints(const AppState& state, size_t i);
