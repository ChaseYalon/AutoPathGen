#include "field_renderer.h"
#include "app_state.h"
#include "codegen.h"
#include "sidebar.h"
#include "constants.h"
#include "utils.h"
#include <cmath>
#include <cstdlib>

void drawWaypoints(AppState& state)
{
	for (size_t i = 0; i < state.waypoints.size(); ++i)
	{
		Segment seg = GetSegmentPoints(state, i);
		int N = state.bezierSubdivisions;
		if (N < 1)
			N = 1;
		{
			Vector2 prevTrue = seg.evaluate(0.0f);
			for (int s = 1; s <= 100; ++s)
			{
				float t = (float) s / 100.0f;
				Vector2 currTrue = seg.evaluate(t);
				DrawLineEx(prevTrue, currTrue, 3.0f, (Color) {252, 212, 111, 255});
				prevTrue = currTrue;
			}
		}

		Vector2 prev = seg.evaluate(0.0f);
		for (int s = 1; s <= N; ++s)
		{
			float t = (float) s / (float) N;
			Vector2 curr = seg.evaluate(t);
			DrawLineEx(prev, curr, 5.0f, BLACK);
			prev = curr;
		}
		bool showHandles = false;
		if (state.selectedWaypointIndex == (int) i)
			showHandles = true;
		if (i > 0 && state.selectedWaypointIndex == (int) i - 1)
			showHandles = true;
		if (i == 0 && state.draggingHandleMode == 4)
			showHandles = true;

		if (showHandles)
		{
			Color handleColor = (Color) {59, 128, 4, 255};	// #3b8004
			Color stalkColor = DARKGRAY;
			float handleRadius = 20.0f;

			DrawLineEx(seg.p0, seg.p1, 2.0f, stalkColor);
			DrawCircleV(seg.p1, handleRadius, handleColor);

			DrawLineEx(seg.p3, seg.p2, 2.0f, stalkColor);
			DrawCircleV(seg.p2, handleRadius, handleColor);
		}
		bool isHovered =
			CheckCollisionPointCircle(GetMousePosition(), state.waypoints[i].pos, 45.0f);
		Color circleColor = PURPLE;
		if (state.selectedWaypointIndex == (int) i)
			circleColor = GREEN;
		else if (isHovered && !state.placingWaypoint)
			circleColor = (Color) {200, 100, 255, 255};	 // Lighter purple on hover

		DrawCircleV(state.waypoints[i].pos, isHovered ? 35.0f : 30.0f, circleColor);

		// Draw heading line
		float angleRad = (state.waypoints[i].heading - 90) * (PI / 180.0f);
		float lineLen = (isHovered ? 35.0f : 30.0f) + 10;
		Vector2 endPos = {state.waypoints[i].pos.x + lineLen * cosf(angleRad),
						  state.waypoints[i].pos.y + lineLen * sinf(angleRad)};
		DrawLineEx(state.waypoints[i].pos, endPos, 4.0f, BLACK);
	}
}

void handleWaypointInput(AppState& state)
{
	Vector2 mousePos = GetMousePosition();

	if (mousePos.x > state.fieldImageWidth || mousePos.y < state.topBar.height)
		return;

	if (state.placingWaypoint || state.showAddActionForm || state.showAddVariableForm ||
		state.showAddActionToWaypointForm || state.askForStartingPosition)
		return;

	if (state.draggingHandleMode != 0 && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
	{
		if (state.selectedWaypointIndex != -1 &&
			state.selectedWaypointIndex < (int) state.waypoints.size())
		{
			Waypoint& wp = state.waypoints[state.selectedWaypointIndex];

			if (state.draggingHandleMode == 1)
			{
				Vector2 oldPos = wp.pos;
				wp.pos = mousePos;
				wp.handleIn.x += (mousePos.x - oldPos.x);
				wp.handleIn.y += (mousePos.y - oldPos.y);
				wp.handleOut.x += (mousePos.x - oldPos.x);
				wp.handleOut.y += (mousePos.y - oldPos.y);
			}
			else if (state.draggingHandleMode == 2)
			{
				wp.handleIn = mousePos;
			}
			else if (state.draggingHandleMode == 3)
			{
				wp.handleOut = mousePos;
			}

			rebuildAutoRoutine(state);
		}

		if (state.draggingHandleMode == 4)
		{
			state.robotState.handleOut = mousePos;
			rebuildAutoRoutine(state);
		}
	}
	else
	{
		if (state.draggingHandleMode != 0)
		{
			state.draggingHandleMode = 0;
		}

		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
		{
			state.draggingHandleMode = 0;

			if (CheckCollisionPointCircle(mousePos, state.robotState.handleOut, 20.0f))
			{
				state.selectedWaypointIndex = -1;
				state.draggingHandleMode = 4;
				return;
			}

			for (size_t i = 0; i < state.waypoints.size(); ++i)
			{
				Waypoint& wp = state.waypoints[i];

				if (CheckCollisionPointCircle(mousePos, wp.handleIn, 40.0f))  // Increased hit box
				{
					state.selectedWaypointIndex = (int) i;
					state.draggingHandleMode = 2;
					updateSidebar(state);
					return;
				}

				if (CheckCollisionPointCircle(mousePos, wp.handleOut, 40.0f))  // Increased hit box
				{
					state.selectedWaypointIndex = (int) i;
					state.draggingHandleMode = 3;
					updateSidebar(state);
					return;
				}
			}

			for (size_t i = 0; i < state.waypoints.size(); ++i)
			{
				if (CheckCollisionPointCircle(mousePos, state.waypoints[i].pos, 45.0f))
				{
					state.selectedWaypointIndex = (int) i;
					if (state.selectedWaypointIndex == (int) i)
					{
						state.draggingHandleMode = 1;
					}
					updateSidebar(state);
					return;
				}
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

				size_t idx = state.waypoints.size() - 1;
				if (idx == 0)
				{
					Vector2 start;
					start = wpilibCoordsToPixels(state.robotStartX, state.robotStartY, FIELD_WIDTH,
												 FIELD_HEIGHT, state.fieldImageWidth,
												 state.fieldImageHeight);

					Vector2 end = state.waypoints[0].pos;
					Vector2 delta = {end.x - start.x, end.y - start.y};
					state.waypoints[0].handleIn = {start.x + delta.x * 0.75f,
												   start.y + delta.y * 0.75f};
					state.robotState.handleOut = {start.x + delta.x * 0.25f,
												  start.y + delta.y * 0.25f};
				}
				else
				{
					Vector2 start = state.waypoints[idx - 1].pos;
					Vector2 end = state.waypoints[idx].pos;
					Vector2 delta = {end.x - start.x, end.y - start.y};

					state.waypoints[idx - 1].handleOut = {start.x + delta.x * 0.25f,
														  start.y + delta.y * 0.25f};

					state.waypoints[idx].handleIn = {start.x + delta.x * 0.75f,
													 start.y + delta.y * 0.75f};
				}

				state.placingWaypoint = false;
			}
		}
	}
}
