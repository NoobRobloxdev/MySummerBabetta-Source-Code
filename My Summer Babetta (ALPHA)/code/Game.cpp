#include "raylib.h"
#include "rlgl.h"
#include <math.h>
#include <vector>

struct GamePart {
    Vector3 pos;
    Vector3 offset;
    bool installed;
    Color color;
    bool isHeld;
    const char* name;
};

void DrawBabettaFrame(Vector3 pos) {
    DrawCube((Vector3){ pos.x, pos.y + 0.6f, pos.z }, 0.1f, 0.1f, 1.2f, LIGHTGRAY); // Main tube
    DrawCube((Vector3){ pos.x, pos.y + 0.8f, pos.z - 0.4f }, 0.1f, 0.4f, 0.1f, LIGHTGRAY); // Seat post
    DrawCube((Vector3){ pos.x, pos.y + 1.0f, pos.z - 0.6f }, 0.3f, 0.05f, 0.5f, RED); // Rear rack
    DrawLine3D((Vector3){ pos.x, pos.y + 0.6f, pos.z + 0.6f }, (Vector3){ pos.x, pos.y + 1.1f, pos.z + 0.6f }, GRAY); // Forks
}

void DrawTexturedCube(Texture2D texture, Vector3 position, float width, float height, float length, Color color) {
    float x = position.x, y = position.y, z = position.z;
    rlSetTexture(texture.id);
    rlBegin(RL_QUADS);
    rlColor4ub(color.r, color.g, color.b, color.a);
    // Face drawing logic here (shortened for brevity, use previous full version for all sides)
    rlNormal3f(0.0f, 1.0f, 0.0f);
    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);
    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);
    rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);
    rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);
    rlEnd();
    rlSetTexture(0);
}

int main() {
    InitWindow(1280, 720, "My Summer Babetta - Building");
    SetExitKey(KEY_NULL);
    Texture2D wallTex = LoadTexture("Textures/wallbuild1.png");
    Texture2D floorTex = LoadTexture("Textures/background1.png.jpeg");
    Camera3D camera = { { 5.0f, 2.0f, 5.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 75.0f, CAMERA_PERSPECTIVE };

    std::vector<GamePart> parts = {
        {{ -2.0f, 0.1f, -2.0f }, { 0.0f, 0.4f, 0.0f }, false, GRAY, false, "Engine"},
        {{ -2.5f, 0.1f, -1.0f }, { 0.0f, 0.4f, 0.6f }, false, DARKGRAY, false, "Front Wheel"},
        {{ -2.5f, 0.1f, -1.5f }, { 0.0f, 0.4f, -0.6f }, false, DARKGRAY, false, "Rear Wheel"}
    };

    DisableCursor();
    while (!WindowShouldClose()) {
        if (!IsKeyDown(KEY_ESCAPE)) {
            UpdateCameraPro(&camera, {(IsKeyDown(KEY_W)-IsKeyDown(KEY_S))*0.1f, (IsKeyDown(KEY_D)-IsKeyDown(KEY_A))*0.1f, 0}, {GetMouseDelta().x*0.1f, GetMouseDelta().y*0.1f, 0}, 0);
            camera.position.y = 1.8f;
        }
        
        // Interaction logic
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            for (auto &p : parts) {
                if (p.isHeld) { p.isHeld = false; break; }
                if (CheckCollisionSpheres(camera.position, 1.5f, p.pos, 0.5f)) { p.isHeld = true; break; }
            }
        }

        BeginDrawing();
            ClearBackground(SKYBLUE);
            BeginMode3D(camera);
                DrawTexturedCube(floorTex, {0,-0.1f,0}, 20, 0.1f, 20, WHITE);
                DrawBabettaFrame({0,0,0});
                for (auto &p : parts) {
                    if (p.isHeld) p.pos = (Vector3){camera.position.x + (camera.target.x - camera.position.x)*0.7f, camera.position.y - 0.3f, camera.position.z + (camera.target.z - camera.position.z)*0.7f};
                    DrawCube(p.pos, 0.3f, 0.3f, 0.3f, p.color);
                }
            EndMode3D();
            DrawCircle(640, 360, 2, WHITE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}