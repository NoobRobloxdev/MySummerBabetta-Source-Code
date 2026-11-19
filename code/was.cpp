#include "raylib.h"
#include "raymath.h"
#include <math.h>

int main() {
    // 1. Nastavenie okna
    const int screenWidth = 1280;
    const int screenHeight = 720;
    SetConfigFlags(FLAG_MSAA_4X_HINT); // Antialiasing
    InitWindow(screenWidth, screenHeight, "Babetta Simulator 2026 - C++");

    // 2. Kamera
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // 3. Fyzika a pozícia Babetty
    Vector3 position = { 0.0f, 0.0f, 0.0f };
    float rotation = 0.0f;
    float speed = 0.0f;
    const float acceleration = 0.5f;
    const float friction = 0.98f;
    const float maxSpeed = 1.0f;

    SetTargetFPS(60);

    // --- HLAVNÁ SMYČKA ---
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // Ovládanie plynu
        if (IsKeyDown(KEY_W)) speed += acceleration * dt;
        else if (IsKeyDown(KEY_S)) speed -= (acceleration * 2) * dt;
        else speed *= friction;

        // Obmedzenie rýchlosti
        if (speed > maxSpeed) speed = maxSpeed;
        if (speed < -0.2f) speed = -0.2f;

        // Otáčanie (iba ak sa hýbeme)
        if (fabs(speed) > 0.01f) {
            if (IsKeyDown(KEY_A)) rotation += 120.0f * dt;
            if (IsKeyDown(KEY_D)) rotation -= 120.0f * dt;
        }

        // Výpočet pohybu
        position.x += sinf(rotation * DEG2RAD) * speed;
        position.z += cosf(rotation * DEG2RAD) * speed;

        // Kamera sleduje hráča
        camera.target = position;
        camera.position.x = position.x - sinf(rotation * DEG2RAD) * 7.0f;
        camera.position.z = position.z - cosf(rotation * DEG2RAD) * 7.0f;
        camera.position.y = position.y + 3.0f;

        // --- VYKRESLOVANIE ---
        BeginDrawing();
            ClearBackground(SKYBLUE);

            BeginMode3D(camera);
                // Mapa / Terén
                DrawGrid(100, 5.0f);
                DrawPlane((Vector3){0, 0, 0}, (Vector2){500, 500}, DARKGREEN);

                // Model Babetty (ako náhrada kocka)
                DrawCubeV(position, (Vector3){0.5f, 1.0f, 1.5f}, ORANGE);
                DrawCubeWiresV(position, (Vector3){0.5f, 1.0f, 1.5f}, BLACK);
                
                // Prekážky v otvorenom svete
                DrawCube((Vector3){10, 1, 10}, 2, 2, 2, GRAY);
                DrawSphere((Vector3){-15, 0, 20}, 3, BROWN);
            EndMode3D();

            // HUD / Tachometer
            DrawRectangle(20, 20, 240, 90, ColorAlpha(BLACK, 0.5f));
            DrawText("BABETTA SIM", 30, 30, 20, YELLOW);
            DrawText(TextFormat("Rychlost: %.1f km/h", speed * 60), 30, 55, 18, WHITE);
            DrawText("W,S - Plyn | A,D - Smer", 30, 80, 15, LIGHTGRAY);

        EndDrawing();
    }

    // 4. Ukončenie
    CloseWindow();
    return 0;
}