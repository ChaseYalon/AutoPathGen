#pragma once

#include "types.h"
#include "ui_elements.h"
#include <utility>
#include <vector>

struct AppState {
	// Field dimensions (set from loaded image)
	int fieldImageWidth;
	int fieldImageHeight;

	// Global Curve Settings
	int bezierSubdivisions;

	// UI elements
	TopBar topBar;
	SidePanel sidePanel;

	// Robot state
	Robot robotState;
	float robotStartX;
	float robotStartY;
	char robotStartXText[32];
	char robotStartYText[32];
	bool startXEditMode;
	bool startYEditMode;
	bool askForStartingPosition;

	// Actions & routine
	std::vector<Action> actions;
	std::vector<Action> driveActions;
	std::vector<Action> autoRoutine;

	// Waypoints
	std::vector<Waypoint> waypoints;
	std::vector<SidePanelElement> controlPanelElems;

	// Popup states
	bool showAddActionForm;
	bool showAddActionToWaypointForm;
	bool showAddVarChangeForm;
	int selectedActionTemplateIndex;
	bool actionDropdownEditMode;
	std::vector<std::pair<std::string, std::string>> tempVarValues;

	// Selection & placement
	bool placingWaypoint;
	bool viewingActions;
	int selectedWaypointIndex;
	int draggingHandleMode;	 // 0: None, 1: Waypoint, 2: HandleIn, 3: HandleOut, 4:
							 // RobotHandleOut

	// Action form buffers
	char actionName[128];
	char actionCode[1024];
	bool actionNameEditMode;
	bool actionCodeEditMode;

	// Variables
	std::vector<ProvidedVariable> providedVariables;
	std::vector<ProvidedVariable> customVariables;
	bool showAddVariableForm;
	char variableName[128];
	char variableValue[32];
	bool variableNameEditMode;
	bool variableValueEditMode;

	// Alerts
	std::vector<Alert> alerts;
	void addAlert(std::string msg, AlertType type, float duration = 5.0f);

	// Export Popup
	bool showExportPopup;
	char exportFilePath[256];
	bool exportPathEditMode;

	// UI Resources
	Font uiFont;

	AppState();
};
