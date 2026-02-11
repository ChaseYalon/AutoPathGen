#pragma once

#include "raylib.h"
#include <string>
#include <vector>

enum class AlertType { Error, Warning, Info };

struct Alert {
	std::string message;
	AlertType type;
	float creationTime;
	float duration;
	Alert(std::string msg, AlertType t, float d = 5.0f);
};

struct ProvidedVariable {
	std::string name;
	std::string value;
	ProvidedVariable(std::string name, std::string value);
};

struct Action {
	std::string name;
	std::string codegen;
	std::string originalTemplate;
	std::vector<ProvidedVariable> usedVariables;
	Action(
		std::string name, std::string codegen, std::vector<ProvidedVariable> usedVariables
	);
};

struct VariableChange {
	std::string name;
	std::string newValue;
	VariableChange(std::string n, std::string v);
};

struct WaypointContext {
	float dx, dy, dist;
	float heading, deltaHeading;
	float distBlue, distRed;
	float xCurrent, yCurrent;
	float xNext;
};

struct Waypoint {
	Vector2 pos;
	float heading;
	// Bezier handles
	Vector2 handleIn;
	Vector2 handleOut;

	std::vector<Action> boundActions;
	std::vector<VariableChange> variableChanges;
	Waypoint(Vector2 p, float h = 0.0f);
};

struct Segment {
	Vector2 p0;	 // Start Point
	Vector2 p1;	 // Start Handle (Outgoing)
	Vector2 p2;	 // End Handle (Incoming)
	Vector2 p3;	 // End Point

	Segment(Vector2 start, Vector2 startHandleOut, Vector2 endHandleIn, Vector2 end);
	Vector2 evaluate(float t) const;
};

struct Robot {
	int x;
	int y;
	float r;
	int radius;
	Vector2 handleOut;
	void draw();
	Robot(int x, int y, float r);
	Robot();
};
