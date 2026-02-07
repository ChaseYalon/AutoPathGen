#include "types.h"
#include <cmath>

Alert::Alert(std::string msg, AlertType t, float d)
	: message(std::move(msg)), type(t), creationTime((float) GetTime()), duration(d)
{
}

ProvidedVariable::ProvidedVariable(std::string name, std::string value)
	: name(std::move(name)), value(std::move(value))
{
}

Action::Action(std::string name, std::string codegen, std::vector<ProvidedVariable> usedVariables)
	: name(std::move(name)),
	  codegen(codegen),
	  originalTemplate(codegen),
	  usedVariables(std::move(usedVariables))
{
}

VariableChange::VariableChange(std::string n, std::string v)
	: name(std::move(n)), newValue(std::move(v))
{
}

Waypoint::Waypoint(Vector2 p, float h) : pos(p), heading(h), handleIn(p), handleOut(p)
{
}

Segment::Segment(Vector2 start, Vector2 startHandleOut, Vector2 endHandleIn, Vector2 end)
	: p0(start), p1(startHandleOut), p2(endHandleIn), p3(end)
{
}

Vector2 Segment::evaluate(float t) const
{
	float u = 1.0f - t;
	float tt = t * t;
	float uu = u * u;
	float uuu = uu * u;
	float ttt = tt * t;

	Vector2 result;
	result.x = uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x;
	result.y = uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y;
	return result;
}

Robot::Robot(int x, int y, float r)
	: x(x), y(y), r(r), radius(30), handleOut({(float) x, (float) y})
{
}

Robot::Robot() : x(0), y(0), r(0), radius(30), handleOut({0, 0})
{
}

void Robot::draw()
{
	DrawCircle(x, y, radius, BLACK);
	float angleRad = (r - 90) * (PI / 180.0f);
	float lineLen = radius + 10;
	Vector2 endPos = {x + lineLen * cosf(angleRad), y + lineLen * sinf(angleRad)};
	DrawLineEx((Vector2) {(float) x, (float) y}, endPos, 7.0f, MAGENTA);
}
