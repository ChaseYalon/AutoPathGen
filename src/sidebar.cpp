#include "sidebar.h"
#include "app_state.h"
#include "codegen.h"
#include "constants.h"
#include "raygui_config.h"
#include "utils.h"

void initControlPanel(AppState& state) {
	state.controlPanelElems.clear();

	// Robot X Slider
	state.controlPanelElems.push_back(SidePanelElement(
		"Robot X",
		[&state](Vector2 topLeft) {
			GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(BLACK));
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

			GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
			GuiLabel(
				(Rectangle) {topLeft.x + 10, topLeft.y, SIDE_PANEL_WIDTH - 20, 40},
				"Robot X (meters)"
			);

			GuiSetStyle(DEFAULT, TEXT_SIZE, 34);
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
			GuiLabel(
				(Rectangle) {topLeft.x + 10, topLeft.y + 45, 90, 40},
				TextFormat("%.2f", FIELD_WIDTH)
			);
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
			GuiLabel(
				(Rectangle) {topLeft.x + SIDE_PANEL_WIDTH - 100, topLeft.y + 45, 90, 40},
				"0"
			);

			float sliderVal = FIELD_WIDTH - state.robotStartX;
			GuiSlider(
				(Rectangle) {topLeft.x + 110, topLeft.y + 50, SIDE_PANEL_WIDTH - 220, 30},
				"",
				"",
				&sliderVal,
				0.0f,
				FIELD_WIDTH
			);
			state.robotStartX = FIELD_WIDTH - sliderVal;
		},
		110
	));

	// Robot Y Slider
	state.controlPanelElems.push_back(SidePanelElement(
		"Robot Y",
		[&state](Vector2 topLeft) {
			GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(BLACK));
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

			GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
			GuiLabel(
				(Rectangle) {topLeft.x + 10, topLeft.y, SIDE_PANEL_WIDTH - 20, 40},
				"Robot Y (meters)"
			);

			GuiSetStyle(DEFAULT, TEXT_SIZE, 34);
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
			GuiLabel((Rectangle) {topLeft.x + 10, topLeft.y + 45, 90, 40}, "0");
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
			GuiLabel(
				(Rectangle) {topLeft.x + SIDE_PANEL_WIDTH - 100, topLeft.y + 45, 90, 40},
				TextFormat("%.2f", FIELD_HEIGHT)
			);

			GuiSlider(
				(Rectangle) {topLeft.x + 110, topLeft.y + 50, SIDE_PANEL_WIDTH - 220, 30},
				"",
				"",
				&state.robotStartY,
				0.0f,
				FIELD_HEIGHT
			);
		},
		110
	));

	// Robot Heading Slider
	state.controlPanelElems.push_back(SidePanelElement(
		"Robot Heading",
		[&state](Vector2 topLeft) {
			GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(BLACK));
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

			GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
			GuiLabel(
				(Rectangle) {topLeft.x + 10, topLeft.y, SIDE_PANEL_WIDTH - 20, 40},
				"Robot Heading (deg)"
			);

			GuiSetStyle(DEFAULT, TEXT_SIZE, 34);
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
			GuiLabel((Rectangle) {topLeft.x + 10, topLeft.y + 45, 90, 40}, "0");
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
			GuiLabel(
				(Rectangle) {topLeft.x + SIDE_PANEL_WIDTH - 100, topLeft.y + 45, 90, 40},
				"360"
			);

			GuiSlider(
				(Rectangle) {topLeft.x + 110, topLeft.y + 50, SIDE_PANEL_WIDTH - 220, 30},
				"",
				"",
				&state.robotState.r,
				0.0f,
				360.0f
			);
		},
		110
	));

	state.controlPanelElems.push_back(SidePanelElement(
		"Curve Quality",
		[&state](Vector2 topLeft) {
			GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(BLACK));
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

			GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
			GuiLabel(
				(Rectangle) {topLeft.x + 10, topLeft.y, SIDE_PANEL_WIDTH - 20, 40},
				TextFormat("Curve Steps (N: %d)", state.bezierSubdivisions)
			);

			GuiSetStyle(DEFAULT, TEXT_SIZE, 34);
			float val = (float) state.bezierSubdivisions;
			GuiSlider(
				(Rectangle) {topLeft.x + 40, topLeft.y + 50, SIDE_PANEL_WIDTH - 80, 30},
				"1",
				"50",
				&val,
				1.0f,
				50.0f
			);
			state.bezierSubdivisions = (int) val;
		},
		110
	));
}

void updateSidebar(AppState& state) {
	state.sidePanel.elems.clear();

	if (state.selectedWaypointIndex != -1) {
		// WAYPOINT EDIT MODE
		int wpIdx = state.selectedWaypointIndex;

		state.sidePanel.elems.push_back(SidePanelElement(
			TextFormat("Waypoint #%d", wpIdx),
			[](Vector2 topLeft) {
				GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
				GuiLabel(
					(Rectangle) {topLeft.x + 10, topLeft.y, SIDE_PANEL_WIDTH - 20, 40},
					"Edit Waypoint"
				);
			},
			50
		));

		state.sidePanel.elems.push_back(SidePanelElement(
			"Waypoint X Slider",
			[&state, wpIdx](Vector2 topLeft) {
				Vector2 currentPixel = state.waypoints[wpIdx].pos;
				Vector2 pixelForCalc =
					{state.waypoints[wpIdx].pos.x,
					 state.waypoints[wpIdx].pos.y - state.topBar.height};

				Vector2 fieldPos = pixelsToWpilibCoords(
					pixelForCalc,
					FIELD_WIDTH,
					FIELD_HEIGHT,
					state.fieldImageWidth,
					state.fieldImageHeight
				);

				GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
				GuiLabel((Rectangle) {topLeft.x + 10, topLeft.y, 60, 30}, "X:");
				GuiLabel(
					(Rectangle) {topLeft.x + SIDE_PANEL_WIDTH - 60, topLeft.y, 50, 30},
					TextFormat("%.2f", fieldPos.x)
				);

				float val = fieldPos.x;
				float oldVal = val;
				GuiSlider(
					(Rectangle) {topLeft.x + 70, topLeft.y, SIDE_PANEL_WIDTH - 140, 30},
					"",
					"",
					&val,
					0.0f,
					FIELD_WIDTH
				);

				if (val != oldVal) {
					// Update X
					Vector2 newPixelBase = wpilibCoordsToPixels(
						val,
						fieldPos.y,
						FIELD_WIDTH,
						FIELD_HEIGHT,
						state.fieldImageWidth,
						state.fieldImageHeight
					);
					// This returns pixel relative to field image (0,0)
					// Add top bar
					Vector2 newPos =
						{newPixelBase.x, newPixelBase.y + state.topBar.height};

					Vector2 delta =
						{newPos.x - state.waypoints[wpIdx].pos.x,
						 newPos.y - state.waypoints[wpIdx].pos.y};

					state.waypoints[wpIdx].pos = newPos;
					state.waypoints[wpIdx].handleIn.x += delta.x;
					state.waypoints[wpIdx].handleIn.y += delta.y;
					state.waypoints[wpIdx].handleOut.x += delta.x;
					state.waypoints[wpIdx].handleOut.y += delta.y;

					rebuildAutoRoutine(state);
				}
			},
			40
		));

		state.sidePanel.elems.push_back(SidePanelElement(
			"Waypoint Y Slider",
			[&state, wpIdx](Vector2 topLeft) {
				Vector2 pixelForCalc =
					{state.waypoints[wpIdx].pos.x,
					 state.waypoints[wpIdx].pos.y - state.topBar.height};
				Vector2 fieldPos = pixelsToWpilibCoords(
					pixelForCalc,
					FIELD_WIDTH,
					FIELD_HEIGHT,
					state.fieldImageWidth,
					state.fieldImageHeight
				);

				GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
				GuiLabel((Rectangle) {topLeft.x + 10, topLeft.y, 60, 30}, "Y:");
				GuiLabel(
					(Rectangle) {topLeft.x + SIDE_PANEL_WIDTH - 60, topLeft.y, 50, 30},
					TextFormat("%.2f", fieldPos.y)
				);

				float val = fieldPos.y;
				float oldVal = val;
				GuiSlider(
					(Rectangle) {topLeft.x + 70, topLeft.y, SIDE_PANEL_WIDTH - 140, 30},
					"",
					"",
					&val,
					0.0f,
					FIELD_HEIGHT
				);

				if (val != oldVal) {
					// Update Y
					Vector2 newPixelBase = wpilibCoordsToPixels(
						fieldPos.x,
						val,
						FIELD_WIDTH,
						FIELD_HEIGHT,
						state.fieldImageWidth,
						state.fieldImageHeight
					);
					Vector2 newPos =
						{newPixelBase.x, newPixelBase.y + state.topBar.height};

					Vector2 delta =
						{newPos.x - state.waypoints[wpIdx].pos.x,
						 newPos.y - state.waypoints[wpIdx].pos.y};

					state.waypoints[wpIdx].pos = newPos;
					state.waypoints[wpIdx].handleIn.x += delta.x;
					state.waypoints[wpIdx].handleIn.y += delta.y;
					state.waypoints[wpIdx].handleOut.x += delta.x;
					state.waypoints[wpIdx].handleOut.y += delta.y;

					rebuildAutoRoutine(state);
				}
			},
			40
		));

		state.sidePanel.elems.push_back(SidePanelElement(
			"Heading Slider",
			[&state, wpIdx](Vector2 topLeft) {
				GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
				GuiLabel(
					(Rectangle) {topLeft.x + 10, topLeft.y, SIDE_PANEL_WIDTH - 20, 30},
					"Heading at point:"
				);

				float oldVal = state.waypoints[wpIdx].heading;
				GuiSlider(
					(
						Rectangle
					) {topLeft.x + 10, topLeft.y + 35, SIDE_PANEL_WIDTH - 70, 30},
					"",
					TextFormat("%.0f", state.waypoints[wpIdx].heading),
					&state.waypoints[wpIdx].heading,
					0,
					360
				);

				if (oldVal != state.waypoints[wpIdx].heading) {
					rebuildAutoRoutine(state);
				}
			},
			80
		));

		// Variable Overrides
		state.sidePanel.elems.push_back(SidePanelElement(
			"Vars Header",
			[](Vector2 tl) {
				GuiSetStyle(DEFAULT, TEXT_SIZE, 24);
				GuiLabel(
					(Rectangle) {tl.x + 10, tl.y + 5, SIDE_PANEL_WIDTH - 20, 25},
					"Variable Overrides:"
				);
			},
			30
		));

		for (int i = 0; i < (int) state.waypoints[wpIdx].variableChanges.size(); i++) {
			state.sidePanel.elems.push_back(SidePanelElement(
				"Var Change Item",
				[&state, idx = i, wpIdx](Vector2 tl) {
					auto& chg = state.waypoints[wpIdx].variableChanges[idx];
					GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
					GuiLabel(
						(Rectangle) {tl.x + 10, tl.y, 250, 30},
						TextFormat("%s = %s", chg.name.c_str(), chg.newValue.c_str())
					);
					if (GuiButton(
							(Rectangle) {tl.x + SIDE_PANEL_WIDTH - 90, tl.y, 80, 30},
							"Remove"
						)) {
						state.waypoints[wpIdx].variableChanges.erase(
							state.waypoints[wpIdx].variableChanges.begin() + idx
						);
						rebuildAutoRoutine(state);
						updateSidebar(state);
					}
				},
				35
			));
		}

		state.sidePanel.elems.push_back(SidePanelElement(
			"Add Var Change Button",
			[&state](Vector2 topLeft) {
				if (GuiButton(
						(
							Rectangle
						) {topLeft.x + 10, topLeft.y, SIDE_PANEL_WIDTH - 20, 40},
						"Set Variable"
					)) {
					state.showAddVarChangeForm = true;
				}
			},
			50
		));

		// ACTIONS LIST
		state.sidePanel.elems.push_back(SidePanelElement(
			"Actions Header",
			[](Vector2 tl) {
				GuiSetStyle(DEFAULT, TEXT_SIZE, 24);
				GuiLabel(
					(Rectangle) {tl.x + 10, tl.y + 5, SIDE_PANEL_WIDTH - 20, 25},
					"Waypoint Actions:"
				);
			},
			30
		));

		for (int i = 0; i < (int) state.waypoints[wpIdx].boundActions.size(); i++) {
			state.sidePanel.elems.push_back(SidePanelElement(
				"Action Item",
				[&state, actionIndex = i, wpIdx](Vector2 tl) {
					Action& act = state.waypoints[wpIdx].boundActions[actionIndex];
					GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
					GuiLabel((Rectangle) {tl.x + 10, tl.y, 200, 30}, act.name.c_str());
					if (GuiButton(
							(Rectangle) {tl.x + SIDE_PANEL_WIDTH - 90, tl.y, 80, 30},
							"Remove"
						)) {
						state.waypoints[wpIdx].boundActions.erase(
							state.waypoints[wpIdx].boundActions.begin() + actionIndex
						);
						rebuildAutoRoutine(state);
						updateSidebar(state);
					}
				},
				35
			));
		}

		state.sidePanel.elems.push_back(SidePanelElement(
			"Add Action Button",
			[&state](Vector2 topLeft) {
				if (GuiButton(
						(
							Rectangle
						) {topLeft.x + 10, topLeft.y, SIDE_PANEL_WIDTH - 20, 40},
						"Add Custom Action"
					)) {
					state.showAddActionToWaypointForm = true;
					state.tempVarValues.clear();
					state.selectedActionTemplateIndex = -1;
					state.actionDropdownEditMode = false;
				}
			},
			50
		));

		state.sidePanel.elems.push_back(SidePanelElement(
			"Delete Button",
			[&state, captureIndex = wpIdx](Vector2 topLeft) {
				if (GuiButton(
						(
							Rectangle
						) {topLeft.x + 10, topLeft.y, SIDE_PANEL_WIDTH - 20, 40},
						"DELETE WAYPOINT"
					)) {
					if (captureIndex >= 0 &&
						captureIndex < (int) state.waypoints.size()) {
						state.waypoints.erase(state.waypoints.begin() + captureIndex);
						state.selectedWaypointIndex = -1;
						rebuildAutoRoutine(state);
						updateSidebar(state);
					}
				}
			},
			60
		));

		state.sidePanel.elems.push_back(SidePanelElement(
			"Close",
			[&state](Vector2 topLeft) {
				if (GuiButton(
						(
							Rectangle
						) {topLeft.x + 10, topLeft.y, SIDE_PANEL_WIDTH - 20, 40},
						"Close / Deselect"
					)) {
					state.selectedWaypointIndex = -1;
					updateSidebar(state);
				}
			},
			60
		));
	} else if (state.viewingActions) {
		for (auto& action : state.autoRoutine) {
			state.sidePanel.elems.push_back(SidePanelElement(
				action.name,
				[action](Vector2 topLeft) {
					GuiSetStyle(DEFAULT, TEXT_SIZE, 48);
					GuiLabel(
						(
							Rectangle
						) {topLeft.x + 10, topLeft.y, SIDE_PANEL_WIDTH - 20, 50},
						action.name.c_str()
					);
					GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
					// Simple visual clipping for code snippet
					std::string codePreview = action.codegen.length() > 28
												  ? action.codegen.substr(0, 25) + "..."
												  : action.codegen;
					GuiLabel(
						(
							Rectangle
						) {topLeft.x + 10, topLeft.y + 60, SIDE_PANEL_WIDTH - 20, 40},
						codePreview.c_str()
					);

					// Draw a separator
					DrawLine(
						topLeft.x,
						topLeft.y + 115,
						topLeft.x + SIDE_PANEL_WIDTH,
						topLeft.y + 115,
						BLACK
					);
				},
				120
			));
		}
	} else {
		state.sidePanel.elems = state.controlPanelElems;
	}
}
