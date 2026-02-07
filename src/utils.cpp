#include "utils.h"
#include <cstdio>

Vector2 wpilibCoordsToPixels(float fieldX, float fieldY, float fieldWidth, float fieldHeight,
							 float screenWidth, float screenHeight)
{
	Vector2 pixel;
	pixel.x = screenWidth - (fieldX / fieldWidth) * screenWidth;	 // flip X
	pixel.y = screenHeight - (fieldY / fieldHeight) * screenHeight;	 // flip Y
	return pixel;
}

Vector2 pixelsToWpilibCoords(Vector2 pixel, float fieldWidth, float fieldHeight, float screenWidth,
							 float screenHeight)
{
	Vector2 field;
	field.x = fieldWidth * (1.0f - (pixel.x / screenWidth));
	field.y = fieldHeight * (1.0f - (pixel.y / screenHeight));
	return field;
}

std::string replaceAll(std::string str, const std::string& from, const std::string& to)
{
	if (from.empty())
		return str;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
	return str;
}

std::string getCoordFmt(float f)
{
	char buf[32];
	snprintf(buf, sizeof(buf), "%.2f", f);
	return std::string(buf);
}

Segment GetSegmentPoints(const AppState& state, size_t i)
{
	Vector2 p0, p1, p2, p3;

	if (i == 0)
	{
		p0 = {(float) state.robotState.x, (float) state.robotState.y};
		p1 = state.robotState.handleOut;
	}
	else
	{
		p0 = state.waypoints[i - 1].pos;
		p1 = state.waypoints[i - 1].handleOut;
	}

	p2 = state.waypoints[i].handleIn;
	p3 = state.waypoints[i].pos;

	return Segment(p0, p1, p2, p3);
}
