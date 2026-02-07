#include "codegen.h"
#include "app_state.h"
#include "constants.h"
#include "utils.h"
#include <cmath>

void rebuildAutoRoutine(AppState &state) {
  state.autoRoutine.clear();
  if (state.waypoints.empty())
    return;

  auto currentVariables = state.customVariables;

  for (size_t i = 0; i < state.waypoints.size(); ++i) {
    // 1. Add Drive Action (to reach this waypoint)
    if (!state.driveActions.empty()) {
      Vector2 prevPixelPos;
      if (i == 0) {
        prevPixelPos = wpilibCoordsToPixels(
            state.robotStartX, state.robotStartY, FIELD_WIDTH, FIELD_HEIGHT,
            state.fieldImageWidth, state.fieldImageHeight);
      } else {
        prevPixelPos = {state.waypoints[i - 1].pos.x,
                        state.waypoints[i - 1].pos.y - state.topBar.height};
      }
      Vector2 currentPixelPos = {
          state.waypoints[i].pos.x,
          state.waypoints[i].pos.y - state.topBar.height};

      Vector2 startField =
          pixelsToWpilibCoords(prevPixelPos, FIELD_WIDTH, FIELD_HEIGHT,
                               state.fieldImageWidth, state.fieldImageHeight);
      Vector2 endField =
          pixelsToWpilibCoords(currentPixelPos, FIELD_WIDTH, FIELD_HEIGHT,
                               state.fieldImageWidth, state.fieldImageHeight);

      float dx = endField.x - startField.x;
      float dy = endField.y - startField.y;
      float dist = sqrtf(dx * dx + dy * dy);

      float prevHeading =
          (i == 0) ? state.robotState.r : state.waypoints[i - 1].heading;
      float currentHeading = state.waypoints[i].heading;
      float deltaHeading = currentHeading - prevHeading;

      float nextWaypointX = 0.0f;
      if (i + 1 < state.waypoints.size()) {
        Vector2 nextPixel = {state.waypoints[i + 1].pos.x,
                             state.waypoints[i + 1].pos.y - state.topBar.height};
        nextWaypointX =
            pixelsToWpilibCoords(nextPixel, FIELD_WIDTH, FIELD_HEIGHT,
                                 state.fieldImageWidth, state.fieldImageHeight)
                .x;
      }

      Action driveAct = state.driveActions.back();
      std::string code = driveAct.originalTemplate;
      code = replaceAll(code, "xDistanceSinceLastWaypoint", getCoordFmt(dx));
      code = replaceAll(code, "yDistanceSinceLastWaypoint", getCoordFmt(dy));
      code =
          replaceAll(code, "distanceSinceLastWaypoint", getCoordFmt(dist));
      code = replaceAll(code, "rotationsSinceLastWaypoint",
                        getCoordFmt(deltaHeading));
      code =
          replaceAll(code, "absoluteRotations", getCoordFmt(currentHeading));
      code = replaceAll(code, "xCordOfNextWaypoint",
                        getCoordFmt(nextWaypointX));
      code = replaceAll(code, "xCordOfCurrentWaypoint",
                        getCoordFmt(endField.x));
      code = replaceAll(code, "yCordOfCurrentWaypoint",
                        getCoordFmt(endField.y));

      // Global custom vars
      for (const auto &cv : currentVariables) {
        code = replaceAll(code, cv.name, cv.value);
      }

      driveAct.codegen = code;
      state.autoRoutine.push_back(driveAct);
    }

    // Apply Variable Changes
    for (const auto &change : state.waypoints[i].variableChanges) {
      for (auto &cv : currentVariables) {
        if (cv.name == change.name) {
          LOG_DEBUG("WP %d: Var %s %s -> %s", (int)i, cv.name.c_str(),
                    cv.value.c_str(), change.newValue.c_str());
          cv.value = change.newValue;
          break;
        }
      }
    }

    // 2. Add Bound Actions for this waypoint
    for (const auto &actionTemplate : state.waypoints[i].boundActions) {
      Action instance = actionTemplate; // Copy
      std::string code = instance.originalTemplate;

      // Recalculate context for bound actions at the waypoint
      Vector2 prevPixelPos;
      if (i == 0) {
        prevPixelPos = wpilibCoordsToPixels(
            state.robotStartX, state.robotStartY, FIELD_WIDTH, FIELD_HEIGHT,
            state.fieldImageWidth, state.fieldImageHeight);
      } else {
        prevPixelPos = {state.waypoints[i - 1].pos.x,
                        state.waypoints[i - 1].pos.y - state.topBar.height};
      }
      Vector2 currentPixelPos = {
          state.waypoints[i].pos.x,
          state.waypoints[i].pos.y - state.topBar.height};
      Vector2 startField =
          pixelsToWpilibCoords(prevPixelPos, FIELD_WIDTH, FIELD_HEIGHT,
                               state.fieldImageWidth, state.fieldImageHeight);
      Vector2 endField =
          pixelsToWpilibCoords(currentPixelPos, FIELD_WIDTH, FIELD_HEIGHT,
                               state.fieldImageWidth, state.fieldImageHeight);
      float dx = endField.x - startField.x;
      float dy = endField.y - startField.y;
      float dist = sqrtf(dx * dx + dy * dy);
      float prevHeading =
          (i == 0) ? state.robotState.r : state.waypoints[i - 1].heading;
      float currentHeading = state.waypoints[i].heading;
      float deltaHeading = currentHeading - prevHeading;
      float nextWaypointX = 0.0f;
      if (i + 1 < state.waypoints.size()) {
        Vector2 nextPixel = {state.waypoints[i + 1].pos.x,
                             state.waypoints[i + 1].pos.y - state.topBar.height};
        nextWaypointX =
            pixelsToWpilibCoords(nextPixel, FIELD_WIDTH, FIELD_HEIGHT,
                                 state.fieldImageWidth, state.fieldImageHeight)
                .x;
      }

      code = replaceAll(code, "xDistanceSinceLastWaypoint", getCoordFmt(dx));
      code = replaceAll(code, "yDistanceSinceLastWaypoint", getCoordFmt(dy));
      code =
          replaceAll(code, "distanceSinceLastWaypoint", getCoordFmt(dist));
      code = replaceAll(code, "rotationsSinceLastWaypoint",
                        getCoordFmt(deltaHeading));
      code =
          replaceAll(code, "absoluteRotations", getCoordFmt(currentHeading));
      code = replaceAll(code, "xCordOfNextWaypoint",
                        getCoordFmt(nextWaypointX));
      code = replaceAll(code, "xCordOfCurrentWaypoint",
                        getCoordFmt(endField.x));
      code = replaceAll(code, "yCordOfCurrentWaypoint",
                        getCoordFmt(endField.y));

      // Replace Custom Vars (using currentVariables context)
      for (const auto &cv : currentVariables) {
        code = replaceAll(code, cv.name, cv.value);
      }

      instance.codegen = code;
      state.autoRoutine.push_back(instance);
      LOG_DEBUG("Added bound action %s at WP %d", instance.name.c_str(),
                (int)i);
    }
  }
}
