// Simple Vulkan shapes test — Phase 2 verification
#include "raylib.h"

int main(void)
{
    InitWindow(800, 450, "raylib [vulkan] - shapes test");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(DARKGRAY);

            DrawRectangle(50, 50, 200, 100, RED);
            DrawRectangle(300, 50, 200, 100, GREEN);
            DrawRectangle(550, 50, 200, 100, BLUE);

            DrawRectangle(50, 200, 700, 200, YELLOW);

            DrawTriangle(
                (Vector2){ 400, 180 },
                (Vector2){ 300, 350 },
                (Vector2){ 500, 350 },
                PURPLE
            );

            DrawCircle(400, 300, 80, (Color){ 255, 128, 0, 180 });

            DrawLine(0, 225, 800, 225, WHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
