#include "app_state.h"
#include "constants.h"
#include <cstdio>
#include <cstring>

AppState::AppState()
	: fieldImageWidth(0),
	  fieldImageHeight(0),
	  bezierSubdivisions(7),
	  robotStartX(0),
	  robotStartY(0),
	  startXEditMode(false),
	  startYEditMode(false),
	  askForStartingPosition(true),
	  showAddActionForm(false),
	  showAddActionToWaypointForm(false),
	  showAddVarChangeForm(false),
	  selectedActionTemplateIndex(-1),
	  actionDropdownEditMode(false),
	  placingWaypoint(false),
	  viewingActions(false),
	  selectedWaypointIndex(-1),
	  draggingHandleMode(0),
	  actionNameEditMode(false),
	  actionCodeEditMode(false),
	  showAddVariableForm(false),
	  variableNameEditMode(false),
	  variableValueEditMode(false),
	  showExportPopup(false),
	  exportPathEditMode(false) {
	std::memset(actionName, 0, sizeof(actionName));
	std::memset(actionCode, 0, sizeof(actionCode));
	std::memset(variableName, 0, sizeof(variableName));
	std::memset(variableValue, 0, sizeof(variableValue));
	std::memset(exportFilePath, 0, sizeof(exportFilePath));

	snprintf(robotStartXText, sizeof(robotStartXText), "%.2f", FIELD_WIDTH / 2.0f);
	snprintf(robotStartYText, sizeof(robotStartYText), "%.2f", FIELD_HEIGHT / 2.0f);

	providedVariables =
		{ProvidedVariable("distanceSinceLastWaypoint", "0.0"),
		 ProvidedVariable("xDistanceSinceLastWaypoint", "0.0"),
		 ProvidedVariable("yDistanceSinceLastWaypoint", "0.0"),
		 ProvidedVariable("rotationsSinceLastWaypoint", "0.0"),
		 ProvidedVariable("absoluteRotations", "0.0"),
		 ProvidedVariable("xCordOfNextWaypoint", "0.0"),
		 ProvidedVariable("xCordOfCurrentWaypoint", "0.0"),
		 ProvidedVariable("yCordOfCurrentWaypoint", "0.0"),
		 ProvidedVariable("distanceFromBlueNet", "0.0"),
		 ProvidedVariable("distanceFromRedNet", "0.0")};
}

void AppState::addAlert(std::string msg, AlertType type, float duration) {
	alerts.emplace_back(msg, type, duration);
}
