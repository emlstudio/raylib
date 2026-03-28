// Simple Vulkan shapes test — Phase 2 verification
#include "raylib.h"

int main(void)
{
    InitWindow(800, 450, "raylib [vulkan] - shapes test");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawText("raylib on Vulkan!", 200, 50, 40, DARKGRAY);
            DrawText("Shapes, textures, and text working!", 150, 100, 20, LIGHTGRAY);

            DrawRectangle(50, 160, 200, 80, RED);
            DrawRectangle(300, 160, 200, 80, GREEN);
            DrawRectangle(550, 160, 200, 80, BLUE);

            DrawCircle(400, 340, 60, ORANGE);

            DrawTriangle(
                (Vector2){ 400, 260 },
                (Vector2){ 340, 380 },
                (Vector2){ 460, 380 },
                PURPLE
            );

            DrawText("Press ESC to exit", 300, 420, 15, GRAY);

            DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
