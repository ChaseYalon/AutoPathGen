// ALL DISTANCES IN METERS
// ALL TIMES IN SECONDS
// UNLESS OTHERWISE SPECIFIED

#define RAYGUI_IMPLEMENTATION
#define RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT 54
#include "../extern/raygui/src/raygui.h"
#include "raylib.h"
#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

#ifdef NDEBUG
#define LOG_DEBUG(...)
#else
#define LOG_DEBUG(...)                                                         \
  do {                                                                         \
    printf("[DEBUG] ");                                                        \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  } while (0)
#endif

#define FIELD_WIDTH 16.540988f
#define FIELD_HEIGHT 8.069326f
#define SIDE_PANEL_WIDTH 500

// these coordinates are compatible w/ WPILIB, so (0,0) is at the bottom corner
// of the blue driver station (1, 0) is one meter up, along the wall of the blue
// driver station (0, 1) is one unit closer to the neutral zone along the bottom
// wall
Vector2 wpilibCoordsToPixels(float fieldX, float fieldY, float fieldWidth,
                             float fieldHeight, float screenWidth,
                             float screenHeight) {
  Vector2 pixel;
  pixel.x = screenWidth - (fieldX / fieldWidth) * screenWidth;    // flip X
  pixel.y = screenHeight - (fieldY / fieldHeight) * screenHeight; // flip Y
  return pixel;
}

Vector2 pixelsToWpilibCoords(Vector2 pixel, float fieldWidth, float fieldHeight,
                             float screenWidth, float screenHeight) {
  Vector2 field;
  field.x = fieldWidth * (1.0f - (pixel.x / screenWidth));
  field.y = fieldHeight * (1.0f - (pixel.y / screenHeight));
  return field;
}

std::string replaceAll(std::string str, const std::string &from,
                       const std::string &to) {
  if (from.empty())
    return str;
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
  return str;
}

struct TopBarElement {
  std::string label;
  int width;
  std::function<void(Vector2)> execute;
  TopBarElement(std::string label, std::function<void(Vector2)> execute,
                int width) {
    this->label = label;
    this->execute = execute;
    this->width = width;
  }
};

struct TopBar {
  int height;
  std::vector<TopBarElement> elems;
  void draw(void) {
    DrawRectangle(0, 0, GetScreenWidth(), this->height, LIGHTGRAY);
    int currentX = 10;
    for (int i = 0; i < this->elems.size(); i++) {
      //this assumes button height is 40, maybe that should be checked or hardcoded???
      this->elems[i].execute((Vector2){static_cast<float>(currentX),
                                       (float)(this->height - 40) / 2});
      currentX += this->elems[i].width;
    }
  }
  TopBar() {
    this->height = 50;
    this->elems = {};
  }
};
struct SidePanelElement {
  std::string label;
  int height;
  std::function<void(Vector2)> execute;
  SidePanelElement(std::string label, std::function<void(Vector2)> execute,
                   int height) {
    this->label = label;
    this->execute = execute;
    this->height = height;
  }
};
struct SidePanel {
  int width;
  std::vector<SidePanelElement> elems;
  void draw(int yOffset) {
    int screenWidth = GetScreenWidth();
    DrawRectangle(screenWidth - this->width, yOffset, 10,
                  GetScreenHeight() - yOffset, LIGHTGRAY);
    int totalHeight = 10 + yOffset;
    for (int i = 0; i < this->elems.size(); i++) {
      // Cache height before execution to avoid crash if elems is
      // modified/cleared during execute
      int elemHeight = this->elems[i].height;
      this->elems[i].execute(
          (Vector2){static_cast<float>(screenWidth - this->width),
                    static_cast<float>(totalHeight)});
      if (i >= this->elems.size())
        break; // Stop if vector was cleared/shrank
      totalHeight += elemHeight;
    }
  }
  SidePanel() {
    this->width = SIDE_PANEL_WIDTH;
    this->elems = {};
  }
};
struct ProvidedVariable {
  std::string name;
  std::string value;
  ProvidedVariable(std::string name, std::string value) {
    this->name = name;
    this->value = value;
  }
};
struct Action {
  std::string name;
  std::string codegen;
  std::string originalTemplate;
  std::vector<ProvidedVariable> usedVariables;
  Action(std::string name, std::string codegen,
         std::vector<ProvidedVariable> usedVariables) {
    this->name = name;
    this->codegen = codegen;
    this->originalTemplate = codegen;
    this->usedVariables = usedVariables;
  }
};

struct VariableChange {
  std::string name;
  std::string newValue;
  VariableChange(std::string n, std::string v) : name(n), newValue(v) {}
};

struct Waypoint {
  Vector2 pos;
  float heading;
  std::vector<Action> boundActions;
  std::vector<VariableChange> variableChanges;
  Waypoint(Vector2 p, float h = 0.0f) : pos(p), heading(h) {}
};

struct Robot {
  int x;
  int y;
  float r;
  int radius = 30;
  void draw() {
    DrawCircle(this->x, this->y, this->radius, BLACK);
    float angleRad = (this->r - 90) * (PI / 180.0f);
    float lineLen = this->radius + 10;
    Vector2 endPos = {this->x + lineLen * cos(angleRad),
                      this->y + lineLen * sin(angleRad)};
    DrawLineEx((Vector2){(float)this->x, (float)this->y}, endPos, 7.0f,
               MAGENTA);
  }
  Robot(int x, int y, float r) {
    this->x = x;
    this->y = y;
    this->r = r;
  }
  Robot() {
    this->x = 0;
    this->y = 0;
    this->r = 0;
  }
};
int main() {
  TopBar t = TopBar();
  SidePanel s = SidePanel();
  Image field = LoadImage("../assets/field_v1.png");
  const int fieldImageHeight = field.height;
  const int fieldImageWidth = field.width;
  InitWindow(fieldImageWidth + s.width, fieldImageHeight + t.height,
             "Combustible lemons 2026 path gen");
  std::vector<ProvidedVariable> providedVariables = {
      ProvidedVariable("distanceSinceLastWaypoint", "0.0"),
      ProvidedVariable("xDistanceSinceLastWaypoint", "0.0"),
      ProvidedVariable("yDistanceSinceLastWaypoint", "0.0"),
      ProvidedVariable("rotationsSinceLastWaypoint", "0.0"),
      ProvidedVariable("absoluteRotations", "0.0"),
      ProvidedVariable("xCordOfNextWaypoint", "0.0"),
      ProvidedVariable("xCordOfCurrentWaypoint", "0.0"),
      ProvidedVariable("yCordOfCurrentWaypoint", "0.0")};

  SetTargetFPS(60);
  Texture2D texture = LoadTextureFromImage(field);
  UnloadImage(field);

  Font roboto = LoadFont("../assets/Roboto-Black.ttf");
  GuiSetFont(roboto);

  bool askForStartingPosition = true;
  bool startXEditMode = false;
  bool startYEditMode = false;
  char robotStartXText[32];
  char robotStartYText[32];
  float robotStartX;
  float robotStartY;
  snprintf(robotStartXText, sizeof(robotStartXText), "%.2f",
           FIELD_WIDTH / 2.0f);
  snprintf(robotStartYText, sizeof(robotStartYText), "%.2f",
           FIELD_HEIGHT / 2.0f);

  Robot robotState;

  GuiSetStyle(DEFAULT, TEXT_SIZE, 32); // bigger text
  GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(BLACK));
  GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, ColorToInt(BLACK));
  GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, ColorToInt(WHITE));

  std::vector<Action> actions;
  std::vector<Action> driveActions;

  std::vector<Action> autoRoutine;
  std::vector<Waypoint> waypoints;
  std::vector<SidePanelElement> controlPanelElems;

  bool showAddActionForm = false;
  bool showAddActionToWaypointForm = false;
  bool showAddVarChangeForm = false;
  int selectedActionTemplateIndex = -1;
  bool actionDropdownEditMode = false;
  // Temporary storage for variable values in the wizard
  // Key: variable name, Value: user input string
  std::vector<std::pair<std::string, std::string>> tempVarValues;

  bool placingWaypoint = false;
  bool viewingActions = false;
  int selectedWaypointIndex = -1;
  char actionName[128] = "";

  std::vector<ProvidedVariable> customVariables;
  bool showAddVariableForm = false;
  char variableName[128] = "";
  char variableValue[32] = "";
  bool variableNameEditMode = false;
  bool variableValueEditMode = false;

  auto getCoordFmt = [](float f) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.2f", f);
    return std::string(buf);
  };

  auto rebuildAutoRoutine = [&]() {
    autoRoutine.clear();
    if (waypoints.empty())
      return;

    auto currentVariables = customVariables;

    for (size_t i = 0; i < waypoints.size(); ++i) {
      // 1. Add Drive Action (to reach this waypoint)
      if (!driveActions.empty()) {
        Vector2 prevPixelPos;
        if (i == 0) {
          prevPixelPos = wpilibCoordsToPixels(
              robotStartX, robotStartY, FIELD_WIDTH, FIELD_HEIGHT,
              fieldImageWidth, fieldImageHeight);
        } else {
          prevPixelPos = {waypoints[i - 1].pos.x,
                          waypoints[i - 1].pos.y - t.height};
        }
        Vector2 currentPixelPos = {waypoints[i].pos.x,
                                   waypoints[i].pos.y - t.height};

        Vector2 startField =
            pixelsToWpilibCoords(prevPixelPos, FIELD_WIDTH, FIELD_HEIGHT,
                                 fieldImageWidth, fieldImageHeight);
        Vector2 endField =
            pixelsToWpilibCoords(currentPixelPos, FIELD_WIDTH, FIELD_HEIGHT,
                                 fieldImageWidth, fieldImageHeight);

        float dx = endField.x - startField.x;
        float dy = endField.y - startField.y;
        float dist = sqrt(dx * dx + dy * dy);

        float prevHeading = (i == 0) ? robotState.r : waypoints[i - 1].heading;
        float currentHeading = waypoints[i].heading;
        float deltaHeading = currentHeading - prevHeading;

        float nextWaypointX = 0.0f;
        if (i + 1 < waypoints.size()) {
          Vector2 nextPixel = {waypoints[i + 1].pos.x,
                               waypoints[i + 1].pos.y - t.height};
          nextWaypointX =
              pixelsToWpilibCoords(nextPixel, FIELD_WIDTH, FIELD_HEIGHT,
                                   fieldImageWidth, fieldImageHeight)
                  .x;
        }

        Action driveAct = driveActions.back();
        std::string code = driveAct.originalTemplate;
        code = replaceAll(code, "xDistanceSinceLastWaypoint", getCoordFmt(dx));
        code = replaceAll(code, "yDistanceSinceLastWaypoint", getCoordFmt(dy));
        code = replaceAll(code, "distanceSinceLastWaypoint", getCoordFmt(dist));
        code = replaceAll(code, "rotationsSinceLastWaypoint",
                          getCoordFmt(deltaHeading));
        code =
            replaceAll(code, "absoluteRotations", getCoordFmt(currentHeading));
        code =
            replaceAll(code, "xCordOfNextWaypoint", getCoordFmt(nextWaypointX));
        code =
            replaceAll(code, "xCordOfCurrentWaypoint", getCoordFmt(endField.x));
        code =
            replaceAll(code, "yCordOfCurrentWaypoint", getCoordFmt(endField.y));

        // Global custom vars
        for (const auto &cv : currentVariables) {
          code = replaceAll(code, cv.name, cv.value);
        }

        driveAct.codegen = code;
        autoRoutine.push_back(driveAct);
      }

      // Apply Variable Changes
      for (const auto &change : waypoints[i].variableChanges) {
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
      for (const auto &actionTemplate : waypoints[i].boundActions) {
        Action instance = actionTemplate; // Copy
        std::string code = instance.originalTemplate;

        // Replace System Vars (context sensitive)
        // Note: Context for bound actions is "at the waypoint"
        // So "current" is waypoint[i], "prev" is waypoint[i-1] (or start)
        // This context matches the drive action context above.
        // We should probably cache these values to reuse.
        // Re-calculating for simplicity/safety against shadowing.
        Vector2 prevPixelPos;
        if (i == 0) {
          prevPixelPos = wpilibCoordsToPixels(
              robotStartX, robotStartY, FIELD_WIDTH, FIELD_HEIGHT,
              fieldImageWidth, fieldImageHeight);
        } else {
          prevPixelPos = {waypoints[i - 1].pos.x,
                          waypoints[i - 1].pos.y - t.height};
        }
        Vector2 currentPixelPos = {waypoints[i].pos.x,
                                   waypoints[i].pos.y - t.height};
        Vector2 startField =
            pixelsToWpilibCoords(prevPixelPos, FIELD_WIDTH, FIELD_HEIGHT,
                                 fieldImageWidth, fieldImageHeight);
        Vector2 endField =
            pixelsToWpilibCoords(currentPixelPos, FIELD_WIDTH, FIELD_HEIGHT,
                                 fieldImageWidth, fieldImageHeight);
        float dx = endField.x - startField.x;
        float dy = endField.y - startField.y;
        float dist = sqrt(dx * dx + dy * dy);
        float prevHeading = (i == 0) ? robotState.r : waypoints[i - 1].heading;
        float currentHeading = waypoints[i].heading;
        float deltaHeading = currentHeading - prevHeading;
        float nextWaypointX = 0.0f;
        if (i + 1 < waypoints.size()) {
          Vector2 nextPixel = {waypoints[i + 1].pos.x,
                               waypoints[i + 1].pos.y - t.height};
          nextWaypointX =
              pixelsToWpilibCoords(nextPixel, FIELD_WIDTH, FIELD_HEIGHT,
                                   fieldImageWidth, fieldImageHeight)
                  .x;
        }

        code = replaceAll(code, "xDistanceSinceLastWaypoint", getCoordFmt(dx));
        code = replaceAll(code, "yDistanceSinceLastWaypoint", getCoordFmt(dy));
        code = replaceAll(code, "distanceSinceLastWaypoint", getCoordFmt(dist));
        code = replaceAll(code, "rotationsSinceLastWaypoint",
                          getCoordFmt(deltaHeading));
        code =
            replaceAll(code, "absoluteRotations", getCoordFmt(currentHeading));
        code =
            replaceAll(code, "xCordOfNextWaypoint", getCoordFmt(nextWaypointX));
        code =
            replaceAll(code, "xCordOfCurrentWaypoint", getCoordFmt(endField.x));
        code =
            replaceAll(code, "yCordOfCurrentWaypoint", getCoordFmt(endField.y));

        // Replace Custom Vars (using currentVariables context)
        for (const auto &cv : currentVariables) {
          code = replaceAll(code, cv.name, cv.value);
        }

        instance.codegen = code;
        autoRoutine.push_back(instance);
        LOG_DEBUG("Added bound action %s at WP %d", instance.name.c_str(),
                  (int)i);
      }
    }
  };

  std::function<void()> updateSidebar;
  updateSidebar = [&]() {
    s.elems.clear();
    if (selectedWaypointIndex != -1) {
      // WAYPOINT EDIT MODE
      s.elems.push_back(SidePanelElement(
          TextFormat("Waypoint #%d", selectedWaypointIndex),
          [](Vector2 topLeft) {
            GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
            GuiLabel((Rectangle){topLeft.x + 10, topLeft.y,
                                 SIDE_PANEL_WIDTH - 20, 40},
                     "Edit Waypoint");
          },
          50));

      s.elems.push_back(SidePanelElement(
          "Heading Slider",
          [ptr = &waypoints[selectedWaypointIndex].heading,
           captureIndex = selectedWaypointIndex,
           &rebuildAutoRoutine](Vector2 topLeft) {
            GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
            GuiLabel((Rectangle){topLeft.x + 10, topLeft.y,
                                 SIDE_PANEL_WIDTH - 20, 30},
                     "Heading at point:");

            float oldVal = *ptr;
            GuiSlider((Rectangle){topLeft.x + 10, topLeft.y + 35,
                                  SIDE_PANEL_WIDTH - 70, 30},
                      "", TextFormat("%.0f", *ptr), ptr, 0, 360);

            if (oldVal != *ptr) {
              rebuildAutoRoutine();
            }
          },
          80));

      // Variable Overrides
      s.elems.push_back(SidePanelElement(
          "Vars Header",
          [](Vector2 tl) {
            GuiSetStyle(DEFAULT, TEXT_SIZE, 24);
            GuiLabel(
                (Rectangle){tl.x + 10, tl.y + 5, SIDE_PANEL_WIDTH - 20, 25},
                "Variable Overrides:");
          },
          30));

      for (int i = 0;
           i < waypoints[selectedWaypointIndex].variableChanges.size(); i++) {
        s.elems.push_back(SidePanelElement(
            "Var Change Item",
            [&, idx = i, wpIndex = selectedWaypointIndex](Vector2 tl) {
              auto &chg = waypoints[wpIndex].variableChanges[idx];
              GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
              GuiLabel((Rectangle){tl.x + 10, tl.y, 250, 30},
                       TextFormat("%s = %s", chg.name.c_str(),
                                  chg.newValue.c_str()));
              if (GuiButton(
                      (Rectangle){tl.x + SIDE_PANEL_WIDTH - 90, tl.y, 80, 30},
                      "Remove")) {
                waypoints[wpIndex].variableChanges.erase(
                    waypoints[wpIndex].variableChanges.begin() + idx);
                rebuildAutoRoutine();
                updateSidebar();
              }
            },
            35));
      }

      s.elems.push_back(SidePanelElement(
          "Add Var Change Button",
          [&](Vector2 topLeft) {
            if (GuiButton((Rectangle){topLeft.x + 10, topLeft.y,
                                      SIDE_PANEL_WIDTH - 20, 40},
                          "Set Variable")) {
              showAddVarChangeForm = true;
            }
          },
          50));

      // ACTIONS LIST
      s.elems.push_back(SidePanelElement(
          "Actions Header",
          [](Vector2 tl) {
            GuiSetStyle(DEFAULT, TEXT_SIZE, 24);
            GuiLabel(
                (Rectangle){tl.x + 10, tl.y + 5, SIDE_PANEL_WIDTH - 20, 25},
                "Waypoint Actions:");
          },
          30));

      for (int i = 0; i < waypoints[selectedWaypointIndex].boundActions.size();
           i++) {
        s.elems.push_back(SidePanelElement(
            "Action Item",
            [&, actionIndex = i, wpIndex = selectedWaypointIndex](Vector2 tl) {
              Action &act = waypoints[wpIndex].boundActions[actionIndex];
              GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
              GuiLabel((Rectangle){tl.x + 10, tl.y, 200, 30}, act.name.c_str());
              if (GuiButton(
                      (Rectangle){tl.x + SIDE_PANEL_WIDTH - 90, tl.y, 80, 30},
                      "Remove")) {
                waypoints[wpIndex].boundActions.erase(
                    waypoints[wpIndex].boundActions.begin() + actionIndex);
                rebuildAutoRoutine();
                updateSidebar();
              }
            },
            35));
      }

      s.elems.push_back(SidePanelElement(
          "Add Action Button",
          [&](Vector2 topLeft) {
            if (GuiButton((Rectangle){topLeft.x + 10, topLeft.y,
                                      SIDE_PANEL_WIDTH - 20, 40},
                          "Add Custom Action")) {
              showAddActionToWaypointForm = true;
              tempVarValues.clear();
              selectedActionTemplateIndex = -1;
              actionDropdownEditMode = false;
            }
          },
          50));

      s.elems.push_back(SidePanelElement(
          "Delete Button",
          [&, captureIndex = selectedWaypointIndex](Vector2 topLeft) {
            if (GuiButton((Rectangle){topLeft.x + 10, topLeft.y,
                                      SIDE_PANEL_WIDTH - 20, 40},
                          "DELETE WAYPOINT")) {
              if (captureIndex >= 0 && captureIndex < waypoints.size()) {
                waypoints.erase(waypoints.begin() + captureIndex);
                selectedWaypointIndex = -1;
                rebuildAutoRoutine();
                updateSidebar();
              }
            }
          },
          60));

      s.elems.push_back(SidePanelElement(
          "Close",
          [&](Vector2 topLeft) {
            if (GuiButton((Rectangle){topLeft.x + 10, topLeft.y,
                                      SIDE_PANEL_WIDTH - 20, 40},
                          "Close / Deselect")) {
              selectedWaypointIndex = -1;
              updateSidebar();
            }
          },
          60));

    } else if (viewingActions) {
      for (auto &action : autoRoutine) {
        s.elems.push_back(SidePanelElement(
            action.name,
            [action](Vector2 topLeft) {
              GuiSetStyle(DEFAULT, TEXT_SIZE, 48);
              GuiLabel((Rectangle){topLeft.x + 10, topLeft.y,
                                   SIDE_PANEL_WIDTH - 20, 50},
                       action.name.c_str());
              GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
              // Simple visual clipping for code snippet
              std::string codePreview =
                  action.codegen.length() > 28
                      ? action.codegen.substr(0, 25) + "..."
                      : action.codegen;
              GuiLabel((Rectangle){topLeft.x + 10, topLeft.y + 60,
                                   SIDE_PANEL_WIDTH - 20, 40},
                       codePreview.c_str());

              // Draw a separator
              DrawLine(topLeft.x, topLeft.y + 115, topLeft.x + SIDE_PANEL_WIDTH,
                       topLeft.y + 115, BLACK);
            },
            120));
      }
    } else {
      s.elems = controlPanelElems;
    }
  };
  bool actionNameEditMode = false;
  char actionCode[1024] = "";
  bool actionCodeEditMode = false;

  t.elems.push_back(TopBarElement(
      "Add Action",
      [&](Vector2 vec) {
        GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
        if (GuiButton((Rectangle){vec.x, vec.y, 250, 40}, "Add Action")) {
          showAddActionForm = true;
          showAddVariableForm = false;
          placingWaypoint = false;
        }
        GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
      },
      260));

  t.elems.push_back(TopBarElement(
      "Add Variable",
      [&](Vector2 vec) {
        GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
        if (GuiButton((Rectangle){vec.x, vec.y, 250, 40}, "Add Variable")) {
          showAddVariableForm = true;
          showAddActionForm = false;
          placingWaypoint = false;
        }
        GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
      },
      260));

  t.elems.push_back(TopBarElement(
      "Add Waypoint",
      [&](Vector2 vec) {
        GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
        if (GuiButton((Rectangle){vec.x, vec.y, 310, 40},
                      placingWaypoint ? "Cancel Waypoint" : "Add Waypoint")) {
          placingWaypoint = !placingWaypoint;
          showAddActionForm = false;
          showAddVariableForm = false;
        }
        GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
      },
      320));

  t.elems.push_back(TopBarElement(
      "Show Actions",
      [&](Vector2 vec) {
        GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
        if (GuiButton((Rectangle){vec.x, vec.y, 310, 40},
                      viewingActions ? "Show Controls" : "Show Actions")) {
          viewingActions = !viewingActions;
          if (viewingActions) {
            rebuildAutoRoutine();
          }
          updateSidebar();
        }
        GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
      },
      320));

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    t.draw();
    DrawTexture(texture, 0, t.height, WHITE);

    Vector2 prevPos = {
        (float)robotState.x,
        (float)robotState.y -
            t.height}; // Adjust for stored y being in screen space w/ offset
    for (size_t i = 0; i < waypoints.size(); ++i) {
      Vector2 currentWp = {waypoints[i].pos.x, waypoints[i].pos.y - t.height};

      Vector2 drawPrev =
          (i == 0) ? (Vector2){(float)robotState.x, (float)robotState.y}
                   : waypoints[i - 1].pos;
      Vector2 drawCurr = waypoints[i].pos;

      DrawLineEx(drawPrev, drawCurr, 5.0f, BLACK);

      bool isHovered = CheckCollisionPointCircle(GetMousePosition(),
                                                 waypoints[i].pos, 45.0f);
      Color circleColor = PURPLE;
      if (selectedWaypointIndex == (int)i)
        circleColor = GREEN;
      else if (isHovered && !placingWaypoint)
        circleColor = (Color){200, 100, 255, 255}; // Lighter purple on hover

      DrawCircleV(drawCurr, isHovered ? 35.0f : 30.0f, circleColor);

      // Draw heading line
      float angleRad = (waypoints[i].heading - 90) * (PI / 180.0f);
      float lineLen = (isHovered ? 35.0f : 30.0f) + 10;
      Vector2 endPos = {drawCurr.x + lineLen * cos(angleRad),
                        drawCurr.y + lineLen * sin(angleRad)};
      DrawLineEx(drawCurr, endPos, 4.0f, BLACK);
    }

    // Check for waypoint selection (mouse click near waypoint)
    if (!placingWaypoint && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        !showAddActionForm && !showAddVariableForm &&
        !showAddActionToWaypointForm && !askForStartingPosition) {
      Vector2 mousePos = GetMousePosition();
      for (size_t i = 0; i < waypoints.size(); ++i) {
        // Increased tolerance radius from 30.0f to 45.0f for easier clicking
        if (CheckCollisionPointCircle(mousePos, waypoints[i].pos, 45.0f)) {
          selectedWaypointIndex = (int)i;
          updateSidebar();
          break;
        }
      }
    }

    // Handle placement
    if (placingWaypoint && !showAddActionForm && !showAddVariableForm &&
        !showAddActionToWaypointForm && !askForStartingPosition) {
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {

        if (driveActions.empty()) {
          TraceLog(LOG_FATAL, "NO DRIVE ACTION TEMPLATE DEFINED. ABORTING.");
          exit(1);
        }

        Vector2 mousePos = GetMousePosition();
        if (mousePos.y > t.height && mousePos.x < fieldImageWidth) {
          // Determine previous position in Field Coordinates
          Vector2 prevPixelPos;
          if (waypoints.empty()) {
            prevPixelPos = {(float)robotState.x,
                            (float)robotState.y - t.height};
          } else {
            prevPixelPos = {waypoints.back().pos.x,
                            waypoints.back().pos.y - t.height};
          }
          Vector2 currentPixelPos = {mousePos.x, mousePos.y - t.height};

          float newHeading = 0.0f;
          if (waypoints.empty())
            newHeading = robotState.r;
          else
            newHeading = waypoints.back().heading;

          waypoints.push_back(Waypoint(mousePos, newHeading));

          if (!driveActions.empty()) {
            // Just trigger rebuild
            rebuildAutoRoutine();
          }

          placingWaypoint = false;
        }
      }
    }

    s.draw(t.height);

    if (askForStartingPosition) {
      const float popupWidth = 600;
      const float popupHeight = 300;
      const float popupX = (fieldImageWidth - popupWidth) / 2.0f;
      const float popupY = ((fieldImageHeight - popupHeight) / 2.0f) + t.height;

      GuiSetStyle(DEFAULT, TEXT_SIZE, 48);
      GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
      GuiWindowBox((Rectangle){popupX, popupY, popupWidth, popupHeight},
                   "Enter Robot Starting Position");
      GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
      GuiSetStyle(DEFAULT, TEXT_SIZE, 32);

      // Input fields for X and Y
      GuiLabel((Rectangle){popupX + 20, popupY + 80, 50, 40}, "X:");
      if (GuiTextBox((Rectangle){popupX + 100, popupY + 80, 200, 40},
                     robotStartXText, 32, startXEditMode))
        startXEditMode = !startXEditMode;

      GuiLabel((Rectangle){popupX + 20, popupY + 150, 50, 40}, "Y:");
      if (GuiTextBox((Rectangle){popupX + 100, popupY + 150, 200, 40},
                     robotStartYText, 32, startYEditMode))
        startYEditMode = !startYEditMode;

      if (GuiButton((Rectangle){popupX + popupWidth / 2 - 75,
                                popupY + popupHeight - 80, 150, 50},
                    "OK")) {
        robotStartX = atof(robotStartXText);
        robotStartY = atof(robotStartYText);
        askForStartingPosition = false;

        controlPanelElems.push_back(SidePanelElement(
            "Robot X",
            [ptr = &robotStartX](Vector2 topLeft) {
              GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(BLACK));
              GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

              GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
              GuiLabel((Rectangle){topLeft.x + 10, topLeft.y,
                                   SIDE_PANEL_WIDTH - 20, 40},
                       "Robot X (meters)");

              GuiSetStyle(DEFAULT, TEXT_SIZE, 34);
              GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
              GuiLabel((Rectangle){topLeft.x + 10, topLeft.y + 45, 90, 40},
                       TextFormat("%.2f", FIELD_WIDTH));
              GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
              GuiLabel((Rectangle){topLeft.x + SIDE_PANEL_WIDTH - 100,
                                   topLeft.y + 45, 90, 40},
                       "0");

              float sliderVal = FIELD_WIDTH - *ptr;
              GuiSlider((Rectangle){topLeft.x + 110, topLeft.y + 50,
                                    SIDE_PANEL_WIDTH - 220, 30},
                        "", "", &sliderVal, 0.0f, FIELD_WIDTH);
              *ptr = FIELD_WIDTH - sliderVal;
            },
            110));

        // Add Robot Y Slider
        controlPanelElems.push_back(SidePanelElement(
            "Robot Y",
            [ptr = &robotStartY](Vector2 topLeft) {
              GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(BLACK));
              GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

              GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
              GuiLabel((Rectangle){topLeft.x + 10, topLeft.y,
                                   SIDE_PANEL_WIDTH - 20, 40},
                       "Robot Y (meters)");

              GuiSetStyle(DEFAULT, TEXT_SIZE, 34);
              GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
              GuiLabel((Rectangle){topLeft.x + 10, topLeft.y + 45, 90, 40},
                       "0");
              GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
              GuiLabel((Rectangle){topLeft.x + SIDE_PANEL_WIDTH - 100,
                                   topLeft.y + 45, 90, 40},
                       TextFormat("%.2f", FIELD_HEIGHT));

              GuiSlider((Rectangle){topLeft.x + 110, topLeft.y + 50,
                                    SIDE_PANEL_WIDTH - 220, 30},
                        "", "", ptr, 0.0f, FIELD_HEIGHT);
            },
            110));

        // Add Robot Rotation Slider
        controlPanelElems.push_back(SidePanelElement(
            "Robot Heading",
            [ptr = &robotState.r](Vector2 topLeft) {
              GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(BLACK));
              GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

              GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
              GuiLabel((Rectangle){topLeft.x + 10, topLeft.y,
                                   SIDE_PANEL_WIDTH - 20, 40},
                       "Robot Heading (deg)");

              GuiSetStyle(DEFAULT, TEXT_SIZE, 34);
              GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
              GuiLabel((Rectangle){topLeft.x + 10, topLeft.y + 45, 90, 40},
                       "0");
              GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
              GuiLabel((Rectangle){topLeft.x + SIDE_PANEL_WIDTH - 100,
                                   topLeft.y + 45, 90, 40},
                       "360");

              GuiSlider((Rectangle){topLeft.x + 110, topLeft.y + 50,
                                    SIDE_PANEL_WIDTH - 220, 30},
                        "", "", ptr, 0.0f, 360.0f);
            },
            110));

        updateSidebar();
      }
    } else {
      Vector2 pixelPos =
          wpilibCoordsToPixels(robotStartX, robotStartY, FIELD_WIDTH,
                               FIELD_HEIGHT, fieldImageWidth, fieldImageHeight);
      robotState.x = (int)pixelPos.x;
      robotState.y = (int)pixelPos.y + t.height;
      robotState.draw();
    }

    if (showAddActionForm) {
      const float popupWidth = 800;
      int totalVars = providedVariables.size() + customVariables.size();
      int varRows = (totalVars + 1) / 2;
      float varSectionHeight = varRows * 30.0f;
      const float popupHeight = 520.0f + varSectionHeight;
      const float popupX = (fieldImageWidth - popupWidth) / 2.0f;
      const float popupY = ((fieldImageHeight - popupHeight) / 2.0f) + t.height;

      GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
      GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
      GuiWindowBox((Rectangle){popupX, popupY, popupWidth, popupHeight},
                   "Add Action");
      GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

      GuiLabel((Rectangle){popupX + 20, popupY + 50, 200, 40}, "Action Name:");
      if (GuiTextBox((Rectangle){popupX + 20, popupY + 90, popupWidth - 40, 40},
                     actionName, 128, actionNameEditMode)) {
        actionNameEditMode = !actionNameEditMode;
      }

      GuiLabel((Rectangle){popupX + 20, popupY + 140, 200, 40},
               "Code Snippet:");
      if (GuiTextBox(
              (Rectangle){popupX + 20, popupY + 180, popupWidth - 40, 200},
              actionCode, 1024, actionCodeEditMode)) {
        actionCodeEditMode = !actionCodeEditMode;
      }

      GuiLabel((Rectangle){popupX + 20, popupY + 390, 400, 40},
               "Available Built-in Variables:");
      GuiSetStyle(DEFAULT, TEXT_SIZE, 24);
      float varY = popupY + 440;
      int varCount = 0;

      // Show built-ins
      for (size_t i = 0; i < providedVariables.size(); i++) {
        float xPos = (varCount % 2 == 0) ? (popupX + 20) : (popupX + 410);
        GuiLabel((Rectangle){xPos, varY, 380, 30},
                 providedVariables[i].name.c_str());
        if (varCount % 2 == 1)
          varY += 30;
        varCount++;
      }
      // Show custom vars
      for (size_t i = 0; i < customVariables.size(); i++) {
        float xPos = (varCount % 2 == 0) ? (popupX + 20) : (popupX + 410);
        GuiLabel((Rectangle){xPos, varY, 380, 30},
                 TextFormat("%s (%s)", customVariables[i].name.c_str(),
                            customVariables[i].value.c_str()));
        if (varCount % 2 == 1)
          varY += 30;
        varCount++;
      }

      GuiSetStyle(DEFAULT, TEXT_SIZE, 32);

      auto detectVariables =
          [&](const std::string &code) -> std::vector<ProvidedVariable> {
        std::vector<ProvidedVariable> used;
        for (const auto &var : providedVariables) {
          if (code.find(var.name) != std::string::npos) {
            used.push_back(var);
          }
        }
        for (const auto &var : customVariables) {
          if (code.find(var.name) != std::string::npos) {
            used.push_back(var);
          }
        }
        return used;
      };

      if (GuiButton(
              (Rectangle){popupX + 20, popupY + popupHeight - 60, 200, 40},
              "Save Action")) {
        actions.push_back(
            Action(actionName, actionCode, detectVariables(actionCode)));
        showAddActionForm = false;
        actionName[0] = '\0';
        actionCode[0] = '\0';
      }

      if (GuiButton(
              (Rectangle){popupX + 240, popupY + popupHeight - 60, 260, 40},
              "Save Drive Action")) {
        driveActions.push_back(
            Action(actionName, actionCode, detectVariables(actionCode)));
        showAddActionForm = false;
        actionName[0] = '\0';
        actionCode[0] = '\0';
      }

      if (GuiButton(
              (Rectangle){popupX + 520, popupY + popupHeight - 60, 100, 40},
              "Cancel")) {
        showAddActionForm = false;
      }
    }

    if (showAddActionToWaypointForm && selectedWaypointIndex != -1) {
      const float popupWidth = 800;
      const float popupHeight = 600;
      const float popupX = (fieldImageWidth - popupWidth) / 2.0f;
      const float popupY = ((fieldImageHeight - popupHeight) / 2.0f) + t.height;

      GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
      GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
      GuiWindowBox((Rectangle){popupX, popupY, popupWidth, popupHeight},
                   "Add Action to Waypoint");
      GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

      // --- CONTENT SECTION (Checkboxes) ---
      GuiLabel((Rectangle){popupX + 20, popupY + 60, 400, 30},
               "Select actions to bind:");
      float actionY = popupY + 100;
      int actionCount = 0;

      // Loop available actions
      for (size_t i = 0; i < actions.size(); i++) {
        bool isBound = false;
        int boundIndex = -1;
        for (size_t k = 0;
             k < waypoints[selectedWaypointIndex].boundActions.size(); k++) {
          // Compare by name or original template? Assuming Name is unique
          // enough for identification
          if (waypoints[selectedWaypointIndex].boundActions[k].name ==
              actions[i].name) {
            isBound = true;
            boundIndex = (int)k;
            break;
          }
        }

        bool params = isBound;
        float xPos = (actionCount % 2 == 0) ? (popupX + 20) : (popupX + 410);

        // GuiCheckBox returns true on click, and updates the bool pointer if
        // provided? RayGui's GuiCheckBox: "Check Box control, returns true when
        // active" ... wait. bool GuiCheckBox(Rectangle bounds, const char
        // *text, bool *checked); It updates the state internally if *checked is
        // passed.

        bool wasChecked = isBound;
        GuiSetStyle(DEFAULT, TEXT_SIZE, 24);
        std::string displayName = actions[i].name;
        // Simple truncation
        if (displayName.length() > 22) {
          displayName = displayName.substr(0, 19) + "...";
        }
        GuiLabel((Rectangle){xPos, actionY, 250, 30}, displayName.c_str());
        if (GuiCheckBox((Rectangle){xPos + 260, actionY, 30, 30}, "",
                        &params)) {
          // toggled
        }
        GuiSetStyle(DEFAULT, TEXT_SIZE, 32);

        // Check if it changed
        if (params != wasChecked) {
          if (params) {
            // Add
            Action newInstance = actions[i];
            // Reset used variables to global defaults (just in case)
            // Although we ignore them in rebuildAutoRoutine, it's good
            // practice. But we don't have the logic here. It's fine.
            waypoints[selectedWaypointIndex].boundActions.push_back(
                newInstance);
            LOG_DEBUG("Added action %s", actions[i].name.c_str());
          } else {
            // Remove
            // We need to find it again because vector might have shifted if we
            // did multiple ops (unlikely in one frame) But saftey:
            for (size_t k = 0;
                 k < waypoints[selectedWaypointIndex].boundActions.size();
                 k++) {
              if (waypoints[selectedWaypointIndex].boundActions[k].name ==
                  actions[i].name) {
                waypoints[selectedWaypointIndex].boundActions.erase(
                    waypoints[selectedWaypointIndex].boundActions.begin() + k);
                LOG_DEBUG("Removed action %s", actions[i].name.c_str());
                break;
              }
            }
          }
          rebuildAutoRoutine();
          updateSidebar();
        }

        if (actionCount % 2 == 1)
          actionY += 40;
        actionCount++;
      }

      if (GuiButton(
              (Rectangle){popupX + 300, popupY + popupHeight - 60, 150, 40},
              "Done")) {
        showAddActionToWaypointForm = false;
      }
    }

    if (showAddVariableForm) {
      const float popupWidth = 600;
      const float popupHeight = 400;
      const float popupX = (fieldImageWidth - popupWidth) / 2.0f;
      const float popupY = ((fieldImageHeight - popupHeight) / 2.0f) + t.height;

      GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
      GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
      GuiWindowBox((Rectangle){popupX, popupY, popupWidth, popupHeight},
                   "Add Variable");
      GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

      GuiLabel((Rectangle){popupX + 20, popupY + 50, 200, 40},
               "Variable Name:");
      if (GuiTextBox((Rectangle){popupX + 20, popupY + 90, popupWidth - 40, 40},
                     variableName, 128, variableNameEditMode)) {
        variableNameEditMode = !variableNameEditMode;
      }

      GuiLabel((Rectangle){popupX + 20, popupY + 140, 200, 40}, "Value:");
      if (GuiTextBox(
              (Rectangle){popupX + 20, popupY + 180, popupWidth - 40, 40},
              variableValue, 32, variableValueEditMode)) {
        variableValueEditMode = !variableValueEditMode;
      }

      if (GuiButton(
              (Rectangle){popupX + 20, popupY + popupHeight - 60, 200, 40},
              "Save Variable")) {
        customVariables.push_back(
            ProvidedVariable(variableName, variableValue));
        showAddVariableForm = false;
        variableName[0] = '\0';
        variableValue[0] = '\0';
      }

      if (GuiButton(
              (Rectangle){popupX + 380, popupY + popupHeight - 60, 200, 40},
              "Cancel")) {
        showAddVariableForm = false;
      }
    }

    if (showAddVarChangeForm && selectedWaypointIndex != -1) {
      const float popupWidth = 600;
      const float popupHeight = 400;
      const float popupX = (fieldImageWidth - popupWidth) / 2.0f;
      const float popupY = ((fieldImageHeight - popupHeight) / 2.0f) + t.height;

      GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
      GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
      GuiWindowBox((Rectangle){popupX, popupY, popupWidth, popupHeight},
                   "Set Variable Value");
      GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

      static int selectedVarIndex = 0;
      static bool dropdownEdit = false;
      static char valBuf[64] = "0.0";
      static bool valEdit = false;

      std::string comboText = "";
      if (customVariables.empty()) {
        comboText = "No Variables";
      } else {
        for (size_t i = 0; i < customVariables.size(); i++) {
          comboText += customVariables[i].name;
          if (i < customVariables.size() - 1)
            comboText += ";";
        }
      }

      GuiLabel((Rectangle){popupX + 20, popupY + 50, 200, 30},
               "Select Variable:");

      if (customVariables.empty()) {
        GuiLabel((Rectangle){popupX + 20, popupY + 90, 300, 40},
                 "No Custom Variables Created.");
      } else {
        // We'll draw inputs first
        GuiLabel((Rectangle){popupX + 20, popupY + 150, 200, 30}, "New Value:");
        if (GuiTextBox((Rectangle){popupX + 20, popupY + 190, 300, 40}, valBuf,
                       64, valEdit)) {
          valEdit = !valEdit;
        }

        if (GuiButton(
                (Rectangle){popupX + 20, popupY + popupHeight - 60, 200, 40},
                "Set Variable")) {
          if (selectedVarIndex >= 0 &&
              selectedVarIndex < customVariables.size()) {
            std::string val = std::string(valBuf);
            std::string varName = customVariables[selectedVarIndex].name;

            bool found = false;
            for (auto &change :
                 waypoints[selectedWaypointIndex].variableChanges) {
              if (change.name == varName) {
                change.newValue = val;
                found = true;
                break;
              }
            }
            if (!found) {
              waypoints[selectedWaypointIndex].variableChanges.push_back(
                  VariableChange(varName, val));
            }

            rebuildAutoRoutine();
            updateSidebar();
            showAddVarChangeForm = false;
            dropdownEdit = false;
            valEdit = false;
          }
        }

        // Draw Dropdown last
        int oldIdx = selectedVarIndex;
        if (GuiDropdownBox((Rectangle){popupX + 20, popupY + 90, 300, 40},
                           comboText.c_str(), &selectedVarIndex,
                           dropdownEdit)) {
          dropdownEdit = !dropdownEdit;
        }
        if (oldIdx != selectedVarIndex && selectedVarIndex >= 0 &&
            selectedVarIndex < customVariables.size()) {
          strncpy(valBuf, customVariables[selectedVarIndex].value.c_str(),
                  sizeof(valBuf));
        }
      }

      if (GuiButton(
              (Rectangle){popupX + 300, popupY + popupHeight - 60, 150, 40},
              "Cancel")) {
        showAddVarChangeForm = false;
        dropdownEdit = false;
      }
    }

    EndDrawing();
  }

  UnloadFont(roboto);
  UnloadTexture(texture);
  CloseWindow();
  return 0;
}

