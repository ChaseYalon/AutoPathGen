#include "types.h"
#include <cmath>
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

Waypoint::Waypoint(Vector2 p, float h) : pos(p), heading(h)
{
}

Robot::Robot(int x, int y, float r) : x(x), y(y), r(r), radius(30)
{
}

Robot::Robot() : x(0), y(0), r(0), radius(30)
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
