#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>  

namespace fs = std::filesystem;


float modSpeedMultiplier = 1.0f;
float modMoneyBonus = 0.0f;

struct Mod {
    std::string name;
};


std::vector<Mod> LoadAndApplyMods() {
    std::vector<Mod> loadedMods;
    if (!fs::exists("mods")) fs::create_directory("mods");

    for (const auto& entry : fs::directory_iterator("mods")) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            if (ext == ".txt" || ext == ".cfg" || ext == ".dll" || ext == ".c") {
                loadedMods.push_back({entry.path().filename().string()});

                
                std::ifstream file(entry.path());
                std::string line;
                while (std::getline(file, line)) {
                  
                    if (line.find("speed:") != std::string::npos) {
                        try {
                            std::string val = line.substr(line.find(":") + 1);
                            modSpeedMultiplier = std::stof(val);
                        } catch (...) {}
                    }
                   
                    if (line.find("money:") != std::string::npos) {
                        try {
                            std::string val = line.substr(line.find(":") + 1);
                            modMoneyBonus = std::stof(val);
                        } catch (...) {}
                    }
                }
            }
        }
    }
    return loadedMods;
}

struct Part { Vector3 pos; Vector3 snapPos; bool installed; Color color; bool isSphere; };

bool IsPlayerColliding(Vector3 p, Vector3 gPos) {
    BoundingBox player = {{p.x-0.5f, 0, p.z-0.5f}, {p.x+0.5f, 2, p.z+0.5f}};
    if (CheckCollisionBoxes(player, (BoundingBox){{gPos.x-7.5f, 0, gPos.z-7.7f}, {gPos.x+7.5f, 5, gPos.z-7.3f}})) return true;
    if (CheckCollisionBoxes(player, (BoundingBox){{gPos.x-7.7f, 0, gPos.z-7.5f}, {gPos.x-7.3f, 5, gPos.z+7.5f}})) return true;
    if (CheckCollisionBoxes(player, (BoundingBox){{gPos.x+7.3f, 0, gPos.z-7.5f}, {gPos.x+7.7f, 5, gPos.z+7.5f}})) return true;
    return false;
}

void DrawFullGarage(Vector3 pos) {
    DrawCube({pos.x, pos.y + 2.5f, pos.z - 7.5f}, 15, 5, 0.2f, BEIGE);  
    DrawCube({pos.x - 7.5f, pos.y + 2.5f, pos.z}, 0.2f, 5, 15, BEIGE); 
    DrawCube({pos.x + 7.5f, pos.y + 2.5f, pos.z}, 0.2f, 5, 15, BEIGE); 
    DrawCube({pos.x, pos.y + 5.0f, pos.z}, 15, 0.2f, 15, GRAY);        
    DrawCube({pos.x - 7.0f, pos.y + 1.5f, pos.z}, 1, 0.1f, 10, BROWN); 
}

int main() {
    InitWindow(1280, 720, "My Summer Babetta - Version: 0.4.5A  || ModLoader Version: 0.1.5A");
    
    InitAudioDevice();
    Music ambient = { 0 }; 
    if (fs::exists("Sounds/birds_background.mp3")) {
        ambient = LoadMusicStream("Sounds/birds_background.mp3");
        PlayMusicStream(ambient);
    }
    
    std::vector<Mod> activeMods = LoadAndApplyMods();
    
    bool radioOn = true;
    SetExitKey(0); 

    Camera3D camera = { { 20.0f, 1.8f, 25.0f }, { 20.0f, 1.8f, 20.0f }, { 0.0f, 1.0f, 0.0f }, 75.0f, CAMERA_PERSPECTIVE };
    
    std::vector<Part> parts = {
        {{ 16.0f, 0.5f, 16.0f }, { 0.0f, 0.5f, 0.0f }, false, GRAY, false}, 
        {{ 16.0f, 0.5f, 17.0f }, { 0.0f, 0.4f, 0.7f }, false, BLACK, true}, 
        {{ 16.0f, 0.5f, 18.0f }, { 0.0f, 0.4f, -0.7f }, false, BLACK, true} 
    };

    float money = 2500.0f + modMoneyBonus; 
    float hunger = 0.0f, thirst = 0.0f, fuel = 5.0f;
    int holdingIdx = -1;
    bool isRiding = false, isPaused = false, showFPS = true;
    Vector3 babettaPos = {20, 0, 20};
    Vector3 radioPos = {13.5f, 1.7f, 20.0f}; 
    float babettaRot = 0.0f, speed = 0.0f;

    DisableCursor();
    SetTargetFPS(120);

    while (!WindowShouldClose()) {
        if (radioOn && ambient.stream.buffer != nullptr) UpdateMusicStream(ambient);

        if (IsKeyPressed(KEY_ESCAPE)) { 
            isPaused = !isPaused; 
            if (isPaused) EnableCursor(); else DisableCursor(); 
        }
        if (IsKeyPressed(KEY_F6)) showFPS = !showFPS;
        if (IsKeyPressed(KEY_F9)) money = 999999.0f;

        if (!isPaused) {                
            hunger += 0.00001f;
            thirst += 0.00005f;

            if (!isRiding) {
                Vector3 oldPos = camera.position;
                UpdateCameraPro(&camera, 
                    {(IsKeyDown(KEY_W)-IsKeyDown(KEY_S))*0.15f, (IsKeyDown(KEY_D)-IsKeyDown(KEY_A))*0.15f, 0}, 
                    {GetMouseDelta().x*0.1f, GetMouseDelta().y*0.1f, 0}, 0);
                
                if (IsPlayerColliding(camera.position, {20,0,20})) camera.position = oldPos; 
                camera.position.y = 1.8f;
                
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionSpheres(camera.position, 1.5f, radioPos, 0.5f)) radioOn = !radioOn;

                    if (holdingIdx == -1) {
                        for (int i = 0; i < (int)parts.size(); i++) 
                            if (!parts[i].installed && CheckCollisionSpheres(camera.position, 2.0f, parts[i].pos, 0.6f)) { holdingIdx = i; break; }
                        
                        if (holdingIdx == -1 && CheckCollisionSpheres(camera.position, 1.5f, babettaPos, 1.0f)) {
                            for (auto& p : parts) if (p.installed) { p.installed = false; p.pos = Vector3Add(camera.position, {1,0,0}); break; }
                        }
                    } else holdingIdx = -1;
                }

                if (holdingIdx != -1) {
                    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
                    parts[holdingIdx].pos = Vector3Add(camera.position, Vector3Scale(forward, 1.5f));
                    if (CheckCollisionSpheres(parts[holdingIdx].pos, 0.6f, babettaPos, 1.0f)) { parts[holdingIdx].installed = true; holdingIdx = -1; }
                }

                bool ready = true; for(auto& p : parts) if(!p.installed) ready = false;
                if (ready && IsKeyPressed(KEY_E)) isRiding = true; 
            } else {
                if (IsKeyDown(KEY_W) && fuel > 0) { speed += (0.003f * modSpeedMultiplier); fuel -= 0.0001f; } else speed *= 0.98f;
                if (IsKeyDown(KEY_A)) babettaRot += 2.0f; if (IsKeyDown(KEY_D)) babettaRot -= 2.0f;
                
                babettaPos.x += sinf(babettaRot * DEG2RAD) * speed;
                babettaPos.z += cosf(babettaRot * DEG2RAD) * speed;
                
                camera.position = Vector3Add(babettaPos, (Vector3){-sinf(babettaRot * DEG2RAD)*5, 2.5f, -cosf(babettaRot * DEG2RAD)*5});
                camera.target = babettaPos;
                if (IsKeyPressed(KEY_E)) isRiding = false;
            }
        }

        BeginDrawing();
            ClearBackground(SKYBLUE);
            BeginMode3D(camera);
                DrawPlane({0, 0, 0}, {4000, 4000}, DARKGREEN);
                DrawCube({0, 0.01f, 0}, 12, 0.01f, 2000, GRAY);
                DrawFullGarage({20, 0, 20});
                
                DrawCube(radioPos, 0.4f, 0.3f, 0.6f, radioOn ? BLUE : DARKGRAY);
                DrawSphere({radioPos.x, radioPos.y + 0.2f, radioPos.z}, 0.05f, radioOn ? GREEN : RED);

                DrawCube({-50, 3, 100}, 20, 6, 20, ORANGE); 
                DrawCube({50, 3, 100}, 20, 6, 20, BLUE);   
                DrawCube({120, 3, 150}, 20, 6, 20, YELLOW); 
                DrawCube({-120, 2.5f, 200}, 10, 5, 10, RED);   

                DrawCube(babettaPos, 0.2f, 0.1f, 1.5f, RED); 
                for (auto& p : parts) {
                    Vector3 dPos = p.installed ? Vector3Add(babettaPos, Vector3RotateByQuaternion(p.snapPos, QuaternionFromEuler(0, babettaRot * DEG2RAD, 0))) : p.pos;
                    if (p.isSphere) DrawSphere(dPos, 0.35f, p.color); else DrawCube(dPos, 0.4f, 0.4f, 0.4f, p.color);
                }
            EndMode3D();

            if (showFPS) DrawFPS(10, 10);
            DrawText(TextFormat("Peniaze: %.0f EUR", money), 10, 40, 20, GREEN);
            DrawText(TextFormat("Hlad: %.1f%%", hunger*100), 10, 70, 20, ORANGE);
            DrawText(radioOn ? "Radio: ON" : "Radio: OFF", 10, 100, 20, radioOn ? BLUE : RED);
            

            DrawText(TextFormat("MODS LOADED: %i", (int)activeMods.size()), 10, 130, 20, GOLD);
            if(modSpeedMultiplier != 1.0f) DrawText(TextFormat("SPEED BOOST: %.1fx", modSpeedMultiplier), 10, 160, 20, YELLOW);


            if (isRiding) {
                DrawText(TextFormat("Rychlost: %i KM/H", (int)(speed*100)), 1000, 650, 20, WHITE);
                DrawText(TextFormat("Palivo: %.1f L", fuel), 1000, 680, 20, YELLOW);
            }

            if (isPaused) {
                DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.7f));
                DrawText("PAUZA", 580, 320, 40, WHITE);
                DrawText("Version: 0.4.5A || ModLoader: 0.1.5A", 530, 100, 30, RED);
            }
        EndDrawing();
    }
    
    if (ambient.stream.buffer != nullptr) UnloadMusicStream(ambient);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}