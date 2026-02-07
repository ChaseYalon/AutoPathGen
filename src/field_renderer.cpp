#include "field_renderer.h"
#include "app_state.h"
#include "codegen.h"
#include "sidebar.h"
#include <cmath>
#include <cstdlib>

void drawWaypoints(AppState& state)
{
	for (size_t i = 0; i < state.waypoints.size(); ++i)
	{
		Vector2 drawPrev = (i == 0)
							   ? (Vector2) {(float) state.robotState.x, (float) state.robotState.y}
							   : state.waypoints[i - 1].pos;
		Vector2 drawCurr = state.waypoints[i].pos;

		DrawLineEx(drawPrev, drawCurr, 5.0f, BLACK);

		bool isHovered =
			CheckCollisionPointCircle(GetMousePosition(), state.waypoints[i].pos, 45.0f);
		Color circleColor = PURPLE;
		if (state.selectedWaypointIndex == (int) i)
			circleColor = GREEN;
		else if (isHovered && !state.placingWaypoint)
			circleColor = (Color) {200, 100, 255, 255};	 // Lighter purple on hover

		DrawCircleV(drawCurr, isHovered ? 35.0f : 30.0f, circleColor);

		// Draw heading line
		float angleRad = (state.waypoints[i].heading - 90) * (PI / 180.0f);
		float lineLen = (isHovered ? 35.0f : 30.0f) + 10;
		Vector2 endPos = {drawCurr.x + lineLen * cosf(angleRad),
						  drawCurr.y + lineLen * sinf(angleRad)};
		DrawLineEx(drawCurr, endPos, 4.0f, BLACK);
	}
}

void handleWaypointSelection(AppState& state)
{
	// Check for waypoint selection (mouse click near waypoint)
	if (!state.placingWaypoint && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
		!state.showAddActionForm && !state.showAddVariableForm &&
		!state.showAddActionToWaypointForm && !state.askForStartingPosition)
	{
		Vector2 mousePos = GetMousePosition();
		for (size_t i = 0; i < state.waypoints.size(); ++i)
		{
			// Increased tolerance radius from 30.0f to 45.0f for easier clicking
			if (CheckCollisionPointCircle(mousePos, state.waypoints[i].pos, 45.0f))
			{
				state.selectedWaypointIndex = (int) i;
				updateSidebar(state);
				break;
			}
		}
	}
}

void handleWaypointPlacement(AppState& state)
{
	// Handle placement
	if (state.placingWaypoint && !state.showAddActionForm && !state.showAddVariableForm &&
		!state.showAddActionToWaypointForm && !state.askForStartingPosition)
	{
		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
		{
			if (state.driveActions.empty())
			{
				TraceLog(LOG_WARNING, "NO DRIVE ACTION TEMPLATE DEFINED. ABORTING.");
				// exit(1); // Don't crash
				state.addAlert("Create a drive action before placing a waypoint!",
							   AlertType::Error);
				state.placingWaypoint = false;
				return;
			}

			Vector2 mousePos = GetMousePosition();
			if (mousePos.y > state.topBar.height && mousePos.x < state.fieldImageWidth)
			{
				float newHeading = 0.0f;
				if (state.waypoints.empty())
					newHeading = state.robotState.r;
				else
					newHeading = state.waypoints.back().heading;

				state.waypoints.push_back(Waypoint(mousePos, newHeading));

				if (!state.driveActions.empty())
				{
					rebuildAutoRoutine(state);
				}

				state.placingWaypoint = false;
			}
		}
	}
}
