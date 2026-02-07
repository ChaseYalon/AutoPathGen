#pragma once

#include "raylib.h"
#include <string>
#include <vector>

struct ProvidedVariable
{
	std::string name;
	std::string value;
	ProvidedVariable(std::string name, std::string value);
};

struct Action
{
	std::string name;
	std::string codegen;
	std::string originalTemplate;
	std::vector<ProvidedVariable> usedVariables;
	Action(std::string name, std::string codegen, std::vector<ProvidedVariable> usedVariables);
};

struct VariableChange
{
	std::string name;
	std::string newValue;
	VariableChange(std::string n, std::string v);
};

struct Waypoint
{
	Vector2 pos;
	float heading;
	std::vector<Action> boundActions;
	std::vector<VariableChange> variableChanges;
	Waypoint(Vector2 p, float h = 0.0f);
};

struct Robot
{
	int x;
	int y;
	float r;
	int radius;
	void draw();
	Robot(int x, int y, float r);
	Robot();
};
