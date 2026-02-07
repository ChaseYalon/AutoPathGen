#include "popups.h"
#include "app_state.h"
#include "codegen.h"
#include "constants.h"
#include "raygui_config.h"
#include "sidebar.h"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

void drawStartingPositionPopup(AppState& state)
{
	const float popupWidth = 600;
	const float popupHeight = 300;
	const float popupX = (state.fieldImageWidth - popupWidth) / 2.0f;
	const float popupY = ((state.fieldImageHeight - popupHeight) / 2.0f) + state.topBar.height;

	GuiSetStyle(DEFAULT, TEXT_SIZE, 48);
	GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
	GuiWindowBox((Rectangle) {popupX, popupY, popupWidth, popupHeight},
				 "Enter Robot Starting Position");
	GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
	GuiSetStyle(DEFAULT, TEXT_SIZE, 32);

	// Input fields for X and Y
	GuiLabel((Rectangle) {popupX + 20, popupY + 80, 50, 40}, "X:");
	if (GuiTextBox((Rectangle) {popupX + 100, popupY + 80, 200, 40}, state.robotStartXText, 32,
				   state.startXEditMode))
		state.startXEditMode = !state.startXEditMode;

	GuiLabel((Rectangle) {popupX + 20, popupY + 150, 50, 40}, "Y:");
	if (GuiTextBox((Rectangle) {popupX + 100, popupY + 150, 200, 40}, state.robotStartYText, 32,
				   state.startYEditMode))
		state.startYEditMode = !state.startYEditMode;

	if (GuiButton((Rectangle) {popupX + popupWidth / 2 - 75, popupY + popupHeight - 80, 150, 50},
				  "OK"))
	{
		state.robotStartX = atof(state.robotStartXText);
		state.robotStartY = atof(state.robotStartYText);
		state.askForStartingPosition = false;

		initControlPanel(state);
		updateSidebar(state);
	}
}

void drawAddActionPopup(AppState& state)
{
	const float popupWidth = 800;
	int totalVars = (int) state.providedVariables.size() + (int) state.customVariables.size();
	int varRows = (totalVars + 1) / 2;
	float varSectionHeight = varRows * 30.0f;
	const float popupHeight = 520.0f + varSectionHeight;
	const float popupX = (state.fieldImageWidth - popupWidth) / 2.0f;
	const float popupY = ((state.fieldImageHeight - popupHeight) / 2.0f) + state.topBar.height;

	GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
	GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
	GuiWindowBox((Rectangle) {popupX, popupY, popupWidth, popupHeight}, "Add Action");
	GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

	GuiLabel((Rectangle) {popupX + 20, popupY + 50, 200, 40}, "Action Name:");
	if (GuiTextBox((Rectangle) {popupX + 20, popupY + 90, popupWidth - 40, 40}, state.actionName,
				   128, state.actionNameEditMode))
	{
		state.actionNameEditMode = !state.actionNameEditMode;
	}

	GuiLabel((Rectangle) {popupX + 20, popupY + 140, 200, 40}, "Code Snippet:");
	if (GuiTextBox((Rectangle) {popupX + 20, popupY + 180, popupWidth - 40, 200}, state.actionCode,
				   1024, state.actionCodeEditMode))
	{
		state.actionCodeEditMode = !state.actionCodeEditMode;
	}

	GuiLabel((Rectangle) {popupX + 20, popupY + 390, 400, 40}, "Available Built-in Variables:");
	GuiSetStyle(DEFAULT, TEXT_SIZE, 24);
	float varY = popupY + 440;
	int varCount = 0;

	// Show built-ins
	for (size_t i = 0; i < state.providedVariables.size(); i++)
	{
		float xPos = (varCount % 2 == 0) ? (popupX + 20) : (popupX + 410);
		GuiLabel((Rectangle) {xPos, varY, 380, 30}, state.providedVariables[i].name.c_str());
		if (varCount % 2 == 1)
			varY += 30;
		varCount++;
	}
	// Show custom vars
	for (size_t i = 0; i < state.customVariables.size(); i++)
	{
		float xPos = (varCount % 2 == 0) ? (popupX + 20) : (popupX + 410);
		GuiLabel((Rectangle) {xPos, varY, 380, 30},
				 TextFormat("%s (%s)", state.customVariables[i].name.c_str(),
							state.customVariables[i].value.c_str()));
		if (varCount % 2 == 1)
			varY += 30;
		varCount++;
	}

	GuiSetStyle(DEFAULT, TEXT_SIZE, 32);

	auto detectVariables = [&state](const std::string& code) -> std::vector<ProvidedVariable>
	{
		std::vector<ProvidedVariable> used;
		for (const auto& var : state.providedVariables)
		{
			if (code.find(var.name) != std::string::npos)
			{
				used.push_back(var);
			}
		}
		for (const auto& var : state.customVariables)
		{
			if (code.find(var.name) != std::string::npos)
			{
				used.push_back(var);
			}
		}
		return used;
	};

	if (GuiButton((Rectangle) {popupX + 20, popupY + popupHeight - 60, 200, 40}, "Save Action"))
	{
		state.actions.push_back(
			Action(state.actionName, state.actionCode, detectVariables(state.actionCode)));
		state.showAddActionForm = false;
		state.actionName[0] = '\0';
		state.actionCode[0] = '\0';
	}

	if (GuiButton((Rectangle) {popupX + 240, popupY + popupHeight - 60, 260, 40},
				  "Save Drive Action"))
	{
		state.driveActions.push_back(
			Action(state.actionName, state.actionCode, detectVariables(state.actionCode)));
		state.showAddActionForm = false;
		state.actionName[0] = '\0';
		state.actionCode[0] = '\0';
	}

	if (GuiButton((Rectangle) {popupX + 520, popupY + popupHeight - 60, 100, 40}, "Cancel"))
	{
		state.showAddActionForm = false;
	}
}

void drawAddActionToWaypointPopup(AppState& state)
{
	const float popupWidth = 800;
	const float popupHeight = 600;
	const float popupX = (state.fieldImageWidth - popupWidth) / 2.0f;
	const float popupY = ((state.fieldImageHeight - popupHeight) / 2.0f) + state.topBar.height;

	GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
	GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
	GuiWindowBox((Rectangle) {popupX, popupY, popupWidth, popupHeight}, "Add Action to Waypoint");
	GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

	// --- CONTENT SECTION (Checkboxes) ---
	GuiLabel((Rectangle) {popupX + 20, popupY + 60, 400, 30}, "Select actions to bind:");
	float actionY = popupY + 100;
	int actionCount = 0;

	// Loop available actions
	for (size_t i = 0; i < state.actions.size(); i++)
	{
		bool isBound = false;
		for (size_t k = 0; k < state.waypoints[state.selectedWaypointIndex].boundActions.size();
			 k++)
		{
			if (state.waypoints[state.selectedWaypointIndex].boundActions[k].name ==
				state.actions[i].name)
			{
				isBound = true;
				break;
			}
		}

		bool params = isBound;
		float xPos = (actionCount % 2 == 0) ? (popupX + 20) : (popupX + 410);

		bool wasChecked = isBound;
		GuiSetStyle(DEFAULT, TEXT_SIZE, 24);
		std::string displayName = state.actions[i].name;
		// Simple truncation
		if (displayName.length() > 22)
		{
			displayName = displayName.substr(0, 19) + "...";
		}
		GuiLabel((Rectangle) {xPos, actionY, 250, 30}, displayName.c_str());
		if (GuiCheckBox((Rectangle) {xPos + 260, actionY, 30, 30}, "", &params))
		{
			// toggled
		}
		GuiSetStyle(DEFAULT, TEXT_SIZE, 32);

		// Check if it changed
		if (params != wasChecked)
		{
			if (params)
			{
				// Add
				Action newInstance = state.actions[i];
				state.waypoints[state.selectedWaypointIndex].boundActions.push_back(newInstance);
				LOG_DEBUG("Added action %s", state.actions[i].name.c_str());
			}
			else
			{
				// Remove
				for (size_t k = 0;
					 k < state.waypoints[state.selectedWaypointIndex].boundActions.size(); k++)
				{
					if (state.waypoints[state.selectedWaypointIndex].boundActions[k].name ==
						state.actions[i].name)
					{
						state.waypoints[state.selectedWaypointIndex].boundActions.erase(
							state.waypoints[state.selectedWaypointIndex].boundActions.begin() + k);
						LOG_DEBUG("Removed action %s", state.actions[i].name.c_str());
						break;
					}
				}
			}
			rebuildAutoRoutine(state);
			updateSidebar(state);
		}

		if (actionCount % 2 == 1)
			actionY += 40;
		actionCount++;
	}

	if (GuiButton((Rectangle) {popupX + 300, popupY + popupHeight - 60, 150, 40}, "Done"))
	{
		state.showAddActionToWaypointForm = false;
	}
}

void drawAddVariablePopup(AppState& state)
{
	const float popupWidth = 600;
	const float popupHeight = 400;
	const float popupX = (state.fieldImageWidth - popupWidth) / 2.0f;
	const float popupY = ((state.fieldImageHeight - popupHeight) / 2.0f) + state.topBar.height;

	GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
	GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
	GuiWindowBox((Rectangle) {popupX, popupY, popupWidth, popupHeight}, "Add Variable");
	GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

	GuiLabel((Rectangle) {popupX + 20, popupY + 50, 200, 40}, "Variable Name:");
	if (GuiTextBox((Rectangle) {popupX + 20, popupY + 90, popupWidth - 40, 40}, state.variableName,
				   128, state.variableNameEditMode))
	{
		state.variableNameEditMode = !state.variableNameEditMode;
	}

	GuiLabel((Rectangle) {popupX + 20, popupY + 140, 200, 40}, "Value:");
	if (GuiTextBox((Rectangle) {popupX + 20, popupY + 180, popupWidth - 40, 40},
				   state.variableValue, 32, state.variableValueEditMode))
	{
		state.variableValueEditMode = !state.variableValueEditMode;
	}

	if (GuiButton((Rectangle) {popupX + 20, popupY + popupHeight - 60, 200, 40}, "Save Variable"))
	{
		state.customVariables.push_back(ProvidedVariable(state.variableName, state.variableValue));
		state.showAddVariableForm = false;
		state.variableName[0] = '\0';
		state.variableValue[0] = '\0';
	}

	if (GuiButton((Rectangle) {popupX + 380, popupY + popupHeight - 60, 200, 40}, "Cancel"))
	{
		state.showAddVariableForm = false;
	}
}

void drawAddVarChangePopup(AppState& state)
{
	const float popupWidth = 600;
	const float popupHeight = 400;
	const float popupX = (state.fieldImageWidth - popupWidth) / 2.0f;
	const float popupY = ((state.fieldImageHeight - popupHeight) / 2.0f) + state.topBar.height;

	GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
	GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
	GuiWindowBox((Rectangle) {popupX, popupY, popupWidth, popupHeight}, "Set Variable Value");
	GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

	static int selectedVarIndex = 0;
	static bool dropdownEdit = false;
	static char valBuf[64] = "0.0";
	static bool valEdit = false;

	std::string comboText = "";
	if (state.customVariables.empty())
	{
		comboText = "No Variables";
	}
	else
	{
		for (size_t i = 0; i < state.customVariables.size(); i++)
		{
			comboText += state.customVariables[i].name;
			if (i < state.customVariables.size() - 1)
				comboText += ";";
		}
	}

	GuiLabel((Rectangle) {popupX + 20, popupY + 50, 200, 30}, "Select Variable:");

	if (state.customVariables.empty())
	{
		GuiLabel((Rectangle) {popupX + 20, popupY + 90, 300, 40}, "No Custom Variables Created.");
	}
	else
	{
		// We'll draw inputs first
		GuiLabel((Rectangle) {popupX + 20, popupY + 150, 200, 30}, "New Value:");
		if (GuiTextBox((Rectangle) {popupX + 20, popupY + 190, 300, 40}, valBuf, 64, valEdit))
		{
			valEdit = !valEdit;
		}

		if (GuiButton((Rectangle) {popupX + 20, popupY + popupHeight - 60, 200, 40},
					  "Set Variable"))
		{
			if (selectedVarIndex >= 0 && selectedVarIndex < (int) state.customVariables.size())
			{
				std::string val = std::string(valBuf);
				std::string varName = state.customVariables[selectedVarIndex].name;

				bool found = false;
				for (auto& change : state.waypoints[state.selectedWaypointIndex].variableChanges)
				{
					if (change.name == varName)
					{
						change.newValue = val;
						found = true;
						break;
					}
				}
				if (!found)
				{
					state.waypoints[state.selectedWaypointIndex].variableChanges.push_back(
						VariableChange(varName, val));
				}

				rebuildAutoRoutine(state);
				updateSidebar(state);
				state.showAddVarChangeForm = false;
				dropdownEdit = false;
				valEdit = false;
			}
		}

		// Draw Dropdown last
		int oldIdx = selectedVarIndex;
		if (GuiDropdownBox((Rectangle) {popupX + 20, popupY + 90, 300, 40}, comboText.c_str(),
						   &selectedVarIndex, dropdownEdit))
		{
			dropdownEdit = !dropdownEdit;
		}
		if (oldIdx != selectedVarIndex && selectedVarIndex >= 0 &&
			selectedVarIndex < (int) state.customVariables.size())
		{
			strncpy(valBuf, state.customVariables[selectedVarIndex].value.c_str(), sizeof(valBuf));
		}
	}

	if (GuiButton((Rectangle) {popupX + 300, popupY + popupHeight - 60, 150, 40}, "Cancel"))
	{
		state.showAddVarChangeForm = false;
		dropdownEdit = false;
	}
}

void drawExportPopup(AppState& state)
{
	const float popupWidth = 600;
	const float popupHeight = 250;
	const float popupX = (state.fieldImageWidth - popupWidth) / 2.0f;
	const float popupY = ((state.fieldImageHeight - popupHeight) / 2.0f) + state.topBar.height;

	GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
	GuiWindowBox((Rectangle) {popupX, popupY, popupWidth, popupHeight}, "Export Code to File");

	GuiLabel((Rectangle) {popupX + 20, popupY + 60, 560, 30}, "File Path:");
	if (GuiTextBox((Rectangle) {popupX + 20, popupY + 100, 560, 40}, state.exportFilePath, 256,
				   state.exportPathEditMode))
	{
		state.exportPathEditMode = !state.exportPathEditMode;
	}

	if (GuiButton((Rectangle) {popupX + 20, popupY + 180, 200, 50}, "Save"))
	{
		std::ofstream outfile(state.exportFilePath);
		if (outfile.is_open())
		{
			for (const auto& action : state.autoRoutine)
			{
				outfile << action.codegen << "\n";
			}
			outfile.close();
			state.showExportPopup = false;
			std::cout << "Saved code to " << state.exportFilePath << std::endl;
		}
		else
		{
			std::cerr << "Failed to open file: " << state.exportFilePath << std::endl;
		}
	}

	if (GuiButton((Rectangle) {popupX + 380, popupY + 180, 200, 50}, "Cancel"))
	{
		state.showExportPopup = false;
	}
}
