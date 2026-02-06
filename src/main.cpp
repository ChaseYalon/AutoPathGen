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
  void draw(void) {
    int screenWidth = GetScreenWidth();
    DrawRectangle(screenWidth - this->width, 0, 10, GetScreenHeight(),
                  LIGHTGRAY);
    int totalHeight = 10;
    for (int i = 0; i < this->elems.size(); i++) {
      this->elems[i].execute(
          (Vector2){static_cast<float>(screenWidth - this->width),
                    static_cast<float>(totalHeight)});
      totalHeight += this->elems[i].height;
    }
  }
  SidePanel() {
    this->width = SIDE_PANEL_WIDTH;
    this->elems = {};
  }
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
    Vector2 endPos = {
        this->x + lineLen * cos(angleRad),
        this->y + lineLen * sin(angleRad)
    };
    DrawLineEx((Vector2){(float)this->x, (float)this->y}, endPos, 7.0f, MAGENTA);
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
  SidePanel s = SidePanel();
  Image field = LoadImage("../assets/field_v1.png");
  const int fieldImageHeight = field.height;
  const int fieldImageWidth = field.width;
  InitWindow(fieldImageWidth + s.width, fieldImageHeight,
             "Combustible lemons 2026 path gen");

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

  GuiSetStyle(DEFAULT, TEXT_SIZE, 32); // bigger text
  GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(BLACK));
  GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, ColorToInt(BLACK));
  GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, ColorToInt(WHITE));

  Robot robotState;

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    DrawTexture(texture, 0, 0, WHITE);
    s.draw();

    if (askForStartingPosition) {
      const float popupWidth = 600;
      const float popupHeight = 300;
      const float popupX = (fieldImageWidth - popupWidth) / 2.0f;
      const float popupY = (fieldImageHeight - popupHeight) / 2.0f;

      GuiSetStyle(DEFAULT, TEXT_SIZE, 48);
      GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
      GuiWindowBox((Rectangle){popupX, popupY, popupWidth, popupHeight},
                   "Enter Starting Position");
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

        s.elems.push_back(SidePanelElement(
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
        s.elems.push_back(SidePanelElement(
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
        s.elems.push_back(SidePanelElement(
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
      }
    } else {
      Vector2 pixelPos =
          wpilibCoordsToPixels(robotStartX, robotStartY, FIELD_WIDTH,
                               FIELD_HEIGHT, fieldImageWidth, fieldImageHeight);
      robotState.x = (int)pixelPos.x;
      robotState.y = (int)pixelPos.y;
      robotState.draw();
    }

    EndDrawing();
  }

  UnloadFont(roboto);
  UnloadTexture(texture);
  CloseWindow(); // Clean up
  return 0;
}
