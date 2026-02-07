#include "codegen.h"
#include "app_state.h"
#include "constants.h"
#include "utils.h"
#include <cmath>

WaypointContext calculateContext(const AppState& state, size_t i)
{
	WaypointContext ctx;

	Vector2 prevPixelPos;
	if (i == 0)
	{
		prevPixelPos = wpilibCoordsToPixels(state.robotStartX, state.robotStartY,
											FIELD_WIDTH, FIELD_HEIGHT,
											state.fieldImageWidth, state.fieldImageHeight);
	}
	else
	{
		prevPixelPos = {state.waypoints[i - 1].pos.x,
						state.waypoints[i - 1].pos.y - state.topBar.height};
	}
	Vector2 currentPixelPos = {state.waypoints[i].pos.x,
							   state.waypoints[i].pos.y - state.topBar.height};

	Vector2 startField =
		pixelsToWpilibCoords(prevPixelPos, FIELD_WIDTH, FIELD_HEIGHT, state.fieldImageWidth,
							 state.fieldImageHeight);
	Vector2 endField = pixelsToWpilibCoords(currentPixelPos, FIELD_WIDTH, FIELD_HEIGHT,
											state.fieldImageWidth, state.fieldImageHeight);

	ctx.dx = endField.x - startField.x;
	ctx.dy = endField.y - startField.y;
	ctx.dist = sqrtf(ctx.dx * ctx.dx + ctx.dy * ctx.dy);

	ctx.distBlue = sqrtf(powf(endField.x - 4.6f, 2) + powf(endField.y - 4.0f, 2));
	ctx.distRed = sqrtf(powf(endField.x - 12.0f, 2) + powf(endField.y - 4.0f, 2));

	float prevHeading = (i == 0) ? state.robotState.r : state.waypoints[i - 1].heading;
	ctx.heading = state.waypoints[i].heading;
	ctx.deltaHeading = ctx.heading - prevHeading;

	ctx.xCurrent = endField.x;
	ctx.yCurrent = endField.y;

	ctx.xNext = 0.0f;
	if (i + 1 < state.waypoints.size())
	{
		Vector2 nextPixel = {state.waypoints[i + 1].pos.x,
							 state.waypoints[i + 1].pos.y - state.topBar.height};
		ctx.xNext = pixelsToWpilibCoords(nextPixel, FIELD_WIDTH, FIELD_HEIGHT,
											 state.fieldImageWidth, state.fieldImageHeight)
							.x;
	}
	return ctx;
}

std::string applyContext(std::string code, const WaypointContext& ctx, const std::vector<ProvidedVariable>& customVars)
{
	code = replaceAll(code, "xDistanceSinceLastWaypoint", getCoordFmt(ctx.dx));
	code = replaceAll(code, "yDistanceSinceLastWaypoint", getCoordFmt(ctx.dy));
	code = replaceAll(code, "distanceSinceLastWaypoint", getCoordFmt(ctx.dist));
	code = replaceAll(code, "rotationsSinceLastWaypoint", getCoordFmt(ctx.deltaHeading));
	code = replaceAll(code, "absoluteRotations", getCoordFmt(ctx.heading));
	code = replaceAll(code, "xCordOfNextWaypoint", getCoordFmt(ctx.xNext));
	code = replaceAll(code, "xCordOfCurrentWaypoint", getCoordFmt(ctx.xCurrent));
	code = replaceAll(code, "yCordOfCurrentWaypoint", getCoordFmt(ctx.yCurrent));
	code = replaceAll(code, "distanceFromBlueNet", getCoordFmt(ctx.distBlue));
	code = replaceAll(code, "distanceFromRedNet", getCoordFmt(ctx.distRed));

	for (const auto& cv : customVars)
	{
		code = replaceAll(code, cv.name, cv.value);
	}
	return code;
}

void rebuildAutoRoutine(AppState& state)
{
	state.autoRoutine.clear();
	if (state.waypoints.empty())
		return;

	auto currentVariables = state.customVariables;

	for (size_t i = 0; i < state.waypoints.size(); ++i)
	{
		WaypointContext ctx = calculateContext(state, i);

		// 1. Add Drive Action (to reach this waypoint)
		if (!state.driveActions.empty())
		{
			Action driveAct = state.driveActions.back();
			driveAct.codegen = applyContext(driveAct.originalTemplate, ctx, currentVariables);
			state.autoRoutine.push_back(driveAct);
		}

		// Apply Variable Changes
		for (const auto& change : state.waypoints[i].variableChanges)
		{
			for (auto& cv : currentVariables)
			{
				if (cv.name == change.name)
				{
					LOG_DEBUG("WP %d: Var %s %s -> %s", (int) i, cv.name.c_str(), cv.value.c_str(),
							  change.newValue.c_str());
					cv.value = change.newValue;
					break;
				}
			}
		}

		// 2. Add Bound Actions for this waypoint
		for (const auto& actionTemplate : state.waypoints[i].boundActions)
		{
			Action instance = actionTemplate;  // Copy
			instance.codegen = applyContext(instance.originalTemplate, ctx, currentVariables);
			state.autoRoutine.push_back(instance);
			LOG_DEBUG("Added bound action %s at WP %d", instance.name.c_str(), (int) i);
		}
	}
}
