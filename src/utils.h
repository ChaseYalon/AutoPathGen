#pragma once

#include "raylib.h"
#include <string>

Vector2 wpilibCoordsToPixels(float fieldX, float fieldY, float fieldWidth, float fieldHeight,
							 float screenWidth, float screenHeight);

Vector2 pixelsToWpilibCoords(Vector2 pixel, float fieldWidth, float fieldHeight, float screenWidth,
							 float screenHeight);

std::string replaceAll(std::string str, const std::string& from, const std::string& to);

std::string getCoordFmt(float f);
