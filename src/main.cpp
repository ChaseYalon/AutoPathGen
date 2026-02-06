//ALL DISTANCES IN METERS
//ALL TIMES IN SECONDS
//UNLESS OTHERWISE SPECIFIED

#include "raylib.h"
#include "raygui.h"

#define FIELD_WITH 16.540988
struct SidePanel {
    int width;
    void draw(void) {
        int screenWidth = GetScreenWidth();
        DrawRectangle(screenWidth - this->width, 0, 10, GetScreenHeight(), LIGHTGRAY);
    }
    SidePanel() {
        this->width = 500;
    }
};
int main() {
    SidePanel s = SidePanel();
    Image field = LoadImage("../assets/field_v1.png");
    InitWindow(field.width + s.width, field.height, "Combustible lemons 2026 path gen");
    
    SetTargetFPS(60);
    Texture2D texture = LoadTextureFromImage(field);
    UnloadImage(field);


    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(DARKGRAY);

        DrawTexture(texture, 0, 0, WHITE);
        s.draw();

        EndDrawing();
    }

    CloseWindow(); // Clean up
    return 0;
}
