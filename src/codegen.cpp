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
		prevPixelPos =
			wpilibCoordsToPixels(state.robotStartX, state.robotStartY, FIELD_WIDTH, FIELD_HEIGHT,
								 state.fieldImageWidth, state.fieldImageHeight);
	}
	else
	{
		prevPixelPos = {state.waypoints[i - 1].pos.x,
						state.waypoints[i - 1].pos.y - state.topBar.height};
	}
	Vector2 currentPixelPos = {state.waypoints[i].pos.x,
							   state.waypoints[i].pos.y - state.topBar.height};

	Vector2 startField = pixelsToWpilibCoords(prevPixelPos, FIELD_WIDTH, FIELD_HEIGHT,
											  state.fieldImageWidth, state.fieldImageHeight);
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

std::string applyContext(std::string code, const WaypointContext& ctx,
						 const std::vector<ProvidedVariable>& customVars)
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

	// Robot Start State
	Vector2 pixelPosPrev =
		wpilibCoordsToPixels(state.robotStartX, state.robotStartY, FIELD_WIDTH, FIELD_HEIGHT,
							 state.fieldImageWidth, state.fieldImageHeight);
	float headingPrev = state.robotState.r;

	for (size_t i = 0; i < state.waypoints.size(); ++i)
	{
		Vector2 p0, p1, p2, p3;
		float h0 = headingPrev;
		float h3 = state.waypoints[i].heading;

		if (i == 0)
		{
			p0 = pixelPosPrev;
			p1 = state.robotState.handleOut;
		}
		else
		{
			p0 = state.waypoints[i - 1].pos;
			p1 = state.waypoints[i - 1].handleOut;
			h0 = state.waypoints[i - 1].heading;
		}
		p2 = state.waypoints[i].handleIn;
		p3 = state.waypoints[i].pos;

		Segment seg(p0, p1, p2, p3);

		int N = state.bezierSubdivisions;
		if (N < 1)
			N = 1;

		for (int s = 1; s <= N; ++s)
		{
			float t = (float) s / (float) N;
			Vector2 posPixels = seg.evaluate(t);

			float heading;
			if (s == N)
			{
				heading = h3;
			}
			else
			{
				Vector2 pBeforeField =
					pixelsToWpilibCoords(pixelPosPrev, FIELD_WIDTH, FIELD_HEIGHT,
										 state.fieldImageWidth, state.fieldImageHeight);
				Vector2 pAfterField =
					pixelsToWpilibCoords(posPixels, FIELD_WIDTH, FIELD_HEIGHT,
										 state.fieldImageWidth, state.fieldImageHeight);

				float dy = pAfterField.y - pBeforeField.y;
				float dx = pAfterField.x - pBeforeField.x;

				// Standard math: 0 is East (Right), 90 is North (Up)
				float angleRad = atan2f(dy, dx);
				float angleDeg = angleRad * 180.0f / PI;

				// Robot Heading: 0 is North (Up), 90 is East (Right) (if matching compass)
				// Or does it match standard math?
				// Let's check Robot::draw:
				// float angleRad = (r - 90) * ...
				// If r=90, angleRad=0 (Right).
				// So Heading 90 is East.
				// If math angle is 0 (East), Heading should be 90.
				// If math angle is 90 (North), Heading should be 0.
				// Heading = 90 - MathAngle.

				heading = 90.0f - angleDeg;

				// Normalize to 0-360 if desired, but not strictly needed for codegen contexts
				// unless specified
			}

			// Create Context
			WaypointContext ctx;
			Vector2 startField =
				pixelsToWpilibCoords(pixelPosPrev, FIELD_WIDTH, FIELD_HEIGHT, state.fieldImageWidth,
									 state.fieldImageHeight);
			Vector2 endField = pixelsToWpilibCoords(posPixels, FIELD_WIDTH, FIELD_HEIGHT,
													state.fieldImageWidth, state.fieldImageHeight);

			ctx.dx = endField.x - startField.x;
			ctx.dy = endField.y - startField.y;
			ctx.dist = sqrtf(ctx.dx * ctx.dx + ctx.dy * ctx.dy);

			ctx.heading = heading;
			ctx.deltaHeading = heading - headingPrev;

			ctx.xCurrent = endField.x;
			ctx.yCurrent = endField.y;

			ctx.distBlue = sqrtf(powf(endField.x - 4.6f, 2) + powf(endField.y - 4.0f, 2));
			ctx.distRed = sqrtf(powf(endField.x - 12.0f, 2) + powf(endField.y - 4.0f, 2));

			// Lookahead
			Vector2 nextPosPixels;
			if (s < N)
				nextPosPixels = seg.evaluate((float) (s + 1) / N);
			else if (i + 1 < state.waypoints.size())
				nextPosPixels = state.waypoints[i + 1].pos;
			else
				nextPosPixels = posPixels;

			Vector2 nextField = pixelsToWpilibCoords(nextPosPixels, FIELD_WIDTH, FIELD_HEIGHT,
													 state.fieldImageWidth, state.fieldImageHeight);
			ctx.xNext = nextField.x;

			// Add Drive Action
			if (!state.driveActions.empty())
			{
				Action driveAct = state.driveActions.back();
				driveAct.codegen = applyContext(driveAct.originalTemplate, ctx, currentVariables);
				state.autoRoutine.push_back(driveAct);
			}
			pixelPosPrev = posPixels;
			headingPrev = heading;
		}

		for (const auto& change : state.waypoints[i].variableChanges)
		{
			for (auto& cv : currentVariables)
			{
				if (cv.name == change.name)
				{
					TraceLog(LOG_INFO, "WP %d: Var %s %s -> %s", (int) i, cv.name.c_str(),
							 cv.value.c_str(), change.newValue.c_str());
					cv.value = change.newValue;
					break;
				}
			}
		}

		WaypointContext finalCtx;
		Vector2 currentField = pixelsToWpilibCoords(pixelPosPrev, FIELD_WIDTH, FIELD_HEIGHT,
													state.fieldImageWidth, state.fieldImageHeight);
		finalCtx.xCurrent = currentField.x;
		finalCtx.yCurrent = currentField.y;
		finalCtx.heading = headingPrev;
		finalCtx.dx = 0;
		finalCtx.dy = 0;
		finalCtx.dist = 0;
		finalCtx.deltaHeading = 0;
		finalCtx.distBlue = sqrtf(powf(currentField.x - 4.6f, 2) + powf(currentField.y - 4.0f, 2));
		finalCtx.distRed = sqrtf(powf(currentField.x - 12.0f, 2) + powf(currentField.y - 4.0f, 2));
		finalCtx.xNext = 0;	 // Unknown or irrelevant

		for (const auto& actionTemplate : state.waypoints[i].boundActions)
		{
			Action instance = actionTemplate;
			instance.codegen = applyContext(instance.originalTemplate, finalCtx, currentVariables);
			state.autoRoutine.push_back(instance);
			TraceLog(LOG_INFO, "Added bound action %s at WP %d", instance.name.c_str(), (int) i);
		}
	}
}
