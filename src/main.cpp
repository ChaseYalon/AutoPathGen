// ALL DISTANCES IN METERS
// ALL TIMES IN SECONDS
// UNLESS OTHERWISE SPECIFIED

#include "app_state.h"
#include "codegen.h"
#include "constants.h"
#include "field_renderer.h"
#include "popups.h"
#include "raygui_config.h"
#include "sidebar.h"
#include "utils.h"
#include <cstring>

int main()
{
	AppState state;

	Image field = LoadImage("../assets/field_v1.png");
	state.fieldImageWidth = field.width;
	state.fieldImageHeight = field.height;

	InitWindow(state.fieldImageWidth + state.sidePanel.width,
			   state.fieldImageHeight + state.topBar.height, "Combustible lemons 2026 path gen");

	SetTargetFPS(60);
	Texture2D texture = LoadTextureFromImage(field);
	UnloadImage(field);

	Font roboto = LoadFont("../assets/Roboto-Black.ttf");
	GuiSetFont(roboto);

	GuiSetStyle(DEFAULT, TEXT_SIZE, 32);  // bigger text
	GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(BLACK));
	GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, ColorToInt(BLACK));
	GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, ColorToInt(WHITE));

	// --- Top Bar Buttons ---
	state.topBar.elems.push_back(TopBarElement(
		"Add Action",
		[&state](Vector2 vec)
		{
			GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
			if (GuiButton((Rectangle) {vec.x, vec.y, 250, 40}, "Add Action"))
			{
				state.showAddActionForm = true;
				state.showAddVariableForm = false;
				state.placingWaypoint = false;
			}
			GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
		},
		260));

	state.topBar.elems.push_back(TopBarElement(
		"Add Variable",
		[&state](Vector2 vec)
		{
			GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
			if (GuiButton((Rectangle) {vec.x, vec.y, 250, 40}, "Add Variable"))
			{
				state.showAddVariableForm = true;
				state.showAddActionForm = false;
				state.placingWaypoint = false;
			}
			GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
		},
		260));

	state.topBar.elems.push_back(TopBarElement(
		"Add Waypoint",
		[&state](Vector2 vec)
		{
			GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
			if (GuiButton((Rectangle) {vec.x, vec.y, 310, 40},
						  state.placingWaypoint ? "Cancel Waypoint" : "Add Waypoint"))
			{
				state.placingWaypoint = !state.placingWaypoint;
				state.showAddActionForm = false;
				state.showAddVariableForm = false;
			}
			GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
		},
		320));

	state.topBar.elems.push_back(TopBarElement(
		"Show Actions",
		[&state](Vector2 vec)
		{
			GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
			if (GuiButton((Rectangle) {vec.x, vec.y, 310, 40},
						  state.viewingActions ? "Show Controls" : "Show Actions"))
			{
				state.viewingActions = !state.viewingActions;
				if (state.viewingActions)
				{
					rebuildAutoRoutine(state);
				}
				updateSidebar(state);
			}
			GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
		},
		320));
	state.topBar.elems.push_back(TopBarElement(
		"Export Code",
		[&state](Vector2 vec)
		{
			GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
			if (GuiButton((Rectangle) {vec.x, vec.y, 250, 40}, "Export Code"))
			{
				state.showExportPopup = true;
				state.exportPathEditMode = false;
				if (strlen(state.exportFilePath) == 0)
				{
					strncpy(state.exportFilePath, "auto.py", sizeof(state.exportFilePath) - 1);
				}
			}
			GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
			GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
		},
		260));

	// --- Main Loop ---
	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(DARKGRAY);

		state.topBar.draw();

		Vector2 mousePos = GetMousePosition();
		if (CheckCollisionPointRec(mousePos, (Rectangle) {0, (float) state.topBar.height,
														  (float) state.fieldImageWidth,
														  (float) state.fieldImageHeight}))
		{
			// Adjust mouse pos to be relative to the field image
			Vector2 relativePos = {mousePos.x, mousePos.y - state.topBar.height};
			Vector2 wpilibCoords =
				pixelsToWpilibCoords(relativePos, FIELD_WIDTH, FIELD_HEIGHT,
									 (float) state.fieldImageWidth, (float) state.fieldImageHeight);

			const char* text = TextFormat("X: %.2f  Y: %.2f", wpilibCoords.x, wpilibCoords.y);
			float textFontSize = 30.0f;
			float spacing = 1.0f;
			Vector2 textSize = MeasureTextEx(roboto, text, textFontSize, spacing);

			// Right align in top bar
			DrawTextEx(roboto, text,
					   (Vector2) {(float) GetScreenWidth() - textSize.x - 20,
								  (state.topBar.height - textSize.y) / 2},
					   textFontSize, spacing, BLACK);
		}

		DrawTexture(texture, 0, state.topBar.height, WHITE);

		drawWaypoints(state);
		handleWaypointSelection(state);
		handleWaypointPlacement(state);

		state.sidePanel.draw(state.topBar.height);

		if (state.askForStartingPosition)
		{
			drawStartingPositionPopup(state);
		}
		else
		{
			Vector2 pixelPos =
				wpilibCoordsToPixels(state.robotStartX, state.robotStartY, FIELD_WIDTH,
									 FIELD_HEIGHT, state.fieldImageWidth, state.fieldImageHeight);
			state.robotState.x = (int) pixelPos.x;
			state.robotState.y = (int) pixelPos.y + state.topBar.height;
			state.robotState.draw();
		}

		if (state.showAddActionForm)
		{
			drawAddActionPopup(state);
		}

		if (state.showExportPopup)
		{
			drawExportPopup(state);
		}

		if (state.showAddActionToWaypointForm && state.selectedWaypointIndex != -1)
		{
			drawAddActionToWaypointPopup(state);
		}

		if (state.showAddVariableForm)
		{
			drawAddVariablePopup(state);
		}

		if (state.showAddVarChangeForm && state.selectedWaypointIndex != -1)
		{
			drawAddVarChangePopup(state);
		}

		EndDrawing();
	}

	UnloadFont(roboto);
	UnloadTexture(texture);
	CloseWindow();
	return 0;
}
