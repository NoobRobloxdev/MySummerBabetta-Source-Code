#include "raylib.h"
#include <vector>
#include <string>
#include <filesystem>


extern "C" __declspec(dllimport) void* __stdcall ShellExecuteA(void*, const char*, const char*, const char*, const char*, int);

namespace fs = std::filesystem;


void OpenLink(const char* url) {
    ShellExecuteA(0, "open", url, 0, 0, 5);
}

void StartGame() {
    if (fs::exists("Game.exe")) {
        OpenLink("Game.exe");
        exit(0);
    }
}


void DrawClickableLink(const char* text, Vector2 pos, int fontSize, Color color, const char* url) {
   
    int width = MeasureText(text, fontSize);
    Rectangle linkRec = { pos.x, pos.y, (float)width, (float)fontSize };
    Vector2 mousePos = GetMousePosition();

    bool isHover = CheckCollisionPointRec(mousePos, linkRec);

    if (isHover) {
        
        DrawRectangle((int)linkRec.x, (int)linkRec.y + fontSize, width, 1, color);
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            OpenLink(url);
        }
    }

    DrawText(text, (int)pos.x, (int)pos.y, fontSize, color);
}

int main() {
    const int screenWidth = 800;
    const int screenHeight = 500;

    InitWindow(screenWidth, screenHeight, "My Summer Babetta - Launcher 0.4.5A");
    SetTargetFPS(120);

    int activeTab = 0; 
    std::vector<std::string> foundMods;

    auto RefreshModList = [&]() {
        foundMods.clear();
        if (!fs::exists("mods")) fs::create_directory("mods");
        for (const auto& entry : fs::directory_iterator("mods")) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                if (ext == ".txt" || ext == ".cfg" || ext == ".dll" || ext == ".c") foundMods.push_back(entry.path().filename().string());
            }
        }
    };

    RefreshModList();

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();
        SetMouseCursor(MOUSE_CURSOR_DEFAULT); 

        BeginDrawing();
            ClearBackground(DARKGRAY);

            DrawRectangle(0, 0, screenWidth, 45, BLACK);
    
            Rectangle tabMainRec = { 10, 5, 110, 35 };
            Rectangle tabModsRec = { 130, 5, 110, 35 };
            Rectangle tabCredRec = { 250, 5, 110, 35 };

            if (CheckCollisionPointRec(mousePos, tabMainRec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) activeTab = 0;
            if (CheckCollisionPointRec(mousePos, tabModsRec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) { activeTab = 1; RefreshModList(); }
            if (CheckCollisionPointRec(mousePos, tabCredRec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) activeTab = 2;

            DrawRectangleRec(tabMainRec, activeTab == 0 ? GRAY : BLACK);
            DrawText("HLAVNE", 25, 12, 20, WHITE);
            DrawRectangleRec(tabModsRec, activeTab == 1 ? GRAY : BLACK);
            DrawText("MODY", 155, 12, 20, WHITE);
            DrawRectangleRec(tabCredRec, activeTab == 2 ? GRAY : BLACK);
            DrawText("CREDITY", 265, 12, 20, WHITE);

      
            if (activeTab == 0) {
                DrawText("MY SUMMER BABETTA", 50, 80, 40, YELLOW);
                DrawText("Verzia: 0.4.5A", 50, 130, 20, LIGHTGRAY);
                DrawText("ModLoader Verzia: 0.1.5A", 50, 150, 20, LIGHTGRAY);
                DrawText("Programované podla LOM_Noob, Jurmat", 50, 170, 20, GOLD);
                
                Rectangle playBtnRec = { 50, 200, 250, 60 };
                bool hoverPlay = CheckCollisionPointRec(mousePos, playBtnRec);
                DrawRectangleRec(playBtnRec, hoverPlay ? RED : MAROON);
                DrawText("SPUSTIT HRU", 90, 220, 25, WHITE);
                if (hoverPlay) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

                if (hoverPlay && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) StartGame();
                if (!fs::exists("Game.exe")) DrawText("Chyba: Game.exe nenajdeny!", 50, 270, 18, RED);
            } 
            else if (activeTab == 1) {
                DrawText("Najdene mody v priecinku /build/mods:", 50, 70, 22, GREEN);
                DrawLine(50, 95, 400, 95, GREEN);
                if (foundMods.empty()) DrawText("Ziadne mody, v MODS (TESTING) su niektore mody.", 50, 120, 20, RED);
                else {
                    for (int i = 0; i < (int)foundMods.size(); i++) 
                        DrawText(TextFormat("- %s", foundMods[i].c_str()), 60, 120 + (i * 30), 20, RAYWHITE);
                }
            }
            else if (activeTab == 2) {
                DrawText("AUTORI A PODPORA", 50, 70, 25, GOLD);
                DrawLine(50, 100, 300, 100, GOLD);

                DrawText("Jurmat", 60, 120, 22, GOLD);
                DrawText("Lead Texture Helper- Owner", 60, 145, 16, GRAY);
                DrawClickableLink("tiktok.com/@jurajkyselica", {60, 162}, 16, SKYBLUE, "https://www.tiktok.com/@jurajkyselica/");

                DrawText("LOM_Noob", 60, 200, 22, YELLOW);
                DrawText("Lead Programmer,Programmed Every thing, Co-Owner", 60, 225, 16, GRAY);
                DrawClickableLink("github.com/NoobRobloxdev/", {60, 242}, 16, SKYBLUE, "https://github.com/NoobRobloxdev");

                DrawText("Katka Cista", 60, 280, 22, BLUE);
                DrawText("Ideas for Launcher and Game and Helper, (Its a girl) -LOM_Noob ", 60, 305, 16, GRAY);
                DrawClickableLink("Link stlac (JK)", {60, 322}, 16, SKYBLUE, "https://youtu.be/-aZ9knDohXw?si=dUk4YF6wgdTqGMyO");

                DrawText("Dobry Autori btw, aj tak juro neviem programovat - LOM_Noob", 50, 450, 18, LIGHTGRAY);
            }

        EndDrawing();
    }

    CloseWindow();
}