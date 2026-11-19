#include "raylib.h"
#include "net.h"
#include <vector>
#include <string>
#include <cstdlib>
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

void StartSim() {
    if (fs::exists("babettasim.exe")) {
        OpenLink("babettasim.exe");
        exit(0);
    }
}

// Spusti Game.exe s parametrami (--host alebo --join IP). Vrati false ak Game.exe chyba.
bool LaunchGameWithArgs(const char* params) {
    if (!fs::exists("Game.exe")) return false;
    ShellExecuteA(0, "open", "Game.exe", params, 0, 5);
    exit(0);
    return true;
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
    const int screenHeight = 520;

    InitWindow(screenWidth, screenHeight, "My Summer Babetta - Launcher 0.4.8DEV2");
    SetTargetFPS(120);

    int activeTab = 0; 
    std::vector<std::string> foundMods;
    std::string mpStatusMsg = "";
    float mpStatusTimer = 0.0f;

    // LAN servery (automaticke hladanie)
    bool discOk = Net::DiscoveryStart();
    Net::LanServer servers[16];
    int serverCount = 0;
    std::string selectedIp = "";

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

        if (mpStatusTimer > 0.0f) {
            mpStatusTimer -= GetFrameTime();
            if (mpStatusTimer <= 0.0f) mpStatusMsg = "";
        }

        // Obnov zoznam LAN serverov
        serverCount = Net::DiscoveryPoll(servers, 16);
        if (!selectedIp.empty()) {
            bool stillThere = false;
            for (int i = 0; i < serverCount; i++) if (selectedIp == servers[i].ip) stillThere = true;
            if (!stillThere) selectedIp.clear();
        }

        BeginDrawing();
            ClearBackground(DARKGRAY);

            DrawRectangle(0, 0, screenWidth, 45, BLACK);
    
            Rectangle tabMainRec = { 10, 5, 100, 35 };
            Rectangle tabModsRec = { 115, 5, 95, 35 };
            Rectangle tabMultiRec = { 215, 5, 145, 35 };
            Rectangle tabCredRec = { 365, 5, 105, 35 };
            Rectangle tabSimRec = { 475, 5, 70, 35 };

            if (CheckCollisionPointRec(mousePos, tabMainRec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) activeTab = 0;
            if (CheckCollisionPointRec(mousePos, tabModsRec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) { activeTab = 1; RefreshModList(); }
            if (CheckCollisionPointRec(mousePos, tabMultiRec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) activeTab = 2;
            if (CheckCollisionPointRec(mousePos, tabCredRec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) activeTab = 3;
            if (CheckCollisionPointRec(mousePos, tabSimRec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) activeTab = 4;

            DrawRectangleRec(tabMainRec, activeTab == 0 ? GRAY : BLACK);
            DrawText("HLAVNE", 25, 12, 20, WHITE);

            DrawRectangleRec(tabModsRec, activeTab == 1 ? GRAY : BLACK);
            DrawText("MODY", 135, 12, 20, WHITE);

            DrawRectangleRec(tabMultiRec, activeTab == 2 ? (Color){40, 70, 130, 255} : (Color){15, 25, 45, 255});
            DrawText("MULTIPLAYER", 225, 12, 18, (activeTab == 2) ? YELLOW : SKYBLUE);

            DrawRectangleRec(tabCredRec, activeTab == 3 ? GRAY : BLACK);
            DrawText("CREDITY", 380, 12, 20, WHITE);

            DrawRectangleRec(tabSimRec, activeTab == 4 ? (Color){140, 35, 35, 255} : (Color){45, 12, 12, 255});
            DrawText("SIM", 492, 12, 18, (activeTab == 4) ? YELLOW : (Color){255, 110, 110, 255});

            // --- TAB 0: HLAVNA STRANKA ---
            if (activeTab == 0) {
                DrawText("MY SUMMER BABETTA", 50, 75, 40, YELLOW);
                DrawText("Verzia: 0.4.8DEV2", 50, 125, 20, LIGHTGRAY);
                DrawText("ModLoader Verzia: 0.1.8ADEV", 50, 148, 20, LIGHTGRAY);
                DrawText("Programovane podla LOM_Noob, Jurmat", 50, 172, 20, GOLD);
                
                Rectangle playBtnRec = { 50, 215, 260, 60 };
                bool hoverPlay = CheckCollisionPointRec(mousePos, playBtnRec);
                DrawRectangleRec(playBtnRec, hoverPlay ? RED : MAROON);
                DrawRectangleLinesEx(playBtnRec, 2, hoverPlay ? YELLOW : RED);
                DrawText("SPUSTIT HRU", 95, 232, 25, WHITE);
                if (hoverPlay) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

                if (hoverPlay && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) StartGame();
                if (!fs::exists("Game.exe")) DrawText("Chyba: Game.exe nenajdeny!", 50, 290, 18, RED);

                // Informacie o novinkach
                DrawRectangle(50, 320, 700, 152, (Color){35, 38, 45, 255});
                DrawRectangleLines(50, 320, 700, 152, GRAY);
                DrawText("NOVINKY VO VERZII 0.4.8DEV2:", 65, 332, 18, YELLOW);
                DrawText("- Restauracia pri pumpe + nova Opravovna Babett", 65, 358, 16, YELLOW);
                DrawText("- Minihra Zloz Babetu v opravovni (odmena 60 EUR)", 65, 380, 16, YELLOW);
                DrawText("- 14 novych domov podel existujucich ulic", 65, 402, 16, YELLOW);
                DrawText("- Dealer showroom: 2x Jawa a 2x Babetta", 65, 424, 16, YELLOW);
                DrawText("- Sound Effekty - Mnauko", 65, 446, 16, YELLOW);
            } 
            // --- TAB 1: MODY ---
            else if (activeTab == 1) {
                DrawText("Najdene mody v priecinku /build/mods:", 50, 70, 22, GREEN);
                DrawLine(50, 95, 450, 95, GREEN);
                if (foundMods.empty()) {
                    DrawText("Ziadne mody. V priecinku 'mods (TESTING)' najdete pripravene ukazky!", 50, 120, 18, RED);
                    DrawText("Podporovane prikazy v modoch:", 50, 160, 20, GOLD);
                    DrawText("  color: BLUE / GREEN / GOLD / BLACK / PURPLE", 50, 190, 16, RAYWHITE);
                    DrawText("  nitro: true (Aktivuje Nitro Boost cez LSHIFT!)", 50, 215, 16, RAYWHITE);
                    DrawText("  jump: true (Umoznuje skakat s Babettou cez SPACE!)", 50, 240, 16, RAYWHITE);
                    DrawText("  gravity: 0.3 (Mesacna nizka gravitacia)", 50, 265, 16, RAYWHITE);
                    DrawText("  speed: 2.0 (Nasobitel vykonu motora)", 50, 290, 16, RAYWHITE);
                    DrawText("  max_speed: 90 (Zmena maximalnej rychlosti v KM/H)", 50, 315, 16, RAYWHITE);
                    DrawText("  money: 50000 (Pociatocny bonus penazi)", 50, 340, 16, RAYWHITE);
                    DrawText("  infinite_fuel: true (Nekonecny benzin)", 50, 365, 16, RAYWHITE);
                    DrawText("  fast_grow: true (PSENICA vyrastie za 0.1 ms)", 50, 390, 16, RAYWHITE);
                    DrawText("  wheat_regrow_time: 5 (cas rastu PSENICA v sekundach)", 50, 415, 16, RAYWHITE);
                } else {
                    for (int i = 0; i < (int)foundMods.size() && i < 10; i++) 
                        DrawText(TextFormat("- %s", foundMods[i].c_str()), 60, 120 + (i * 28), 20, RAYWHITE);
                }
            }
            // --- TAB 2: MULTIPLAYER (LAN zoznam serverov) ---
            else if (activeTab == 2) {
                DrawText("MY SUMMER BABETTA - MULTIPLAYER (ONLINE / LAN)", 50, 65, 24, YELLOW);
                if (discOk) DrawText("STAV: TESTOVACIA VERZIA - LAN servery sa hladaju automaticky", 50, 95, 16, (Color){120, 220, 120, 255});
                else DrawText("STAV: LAN hladanie nefunguje (port 7778 sa nepodarilo otvorit)", 50, 95, 16, (Color){255, 140, 40, 255});
                DrawLine(50, 115, 750, 115, (Color){40, 80, 140, 255});

                DrawRectangle(50, 130, 700, 170, (Color){25, 30, 42, 255});
                DrawRectangleLines(50, 130, 700, 170, (Color){50, 70, 100, 255});
                DrawText("NAZOV SERVERA", 65, 140, 15, GRAY);
                DrawText("IP ADRESA", 390, 140, 15, GRAY);
                DrawText("SIET", 540, 140, 15, GRAY);
                DrawText("HRACI", 640, 140, 15, GRAY);
                DrawLine(50, 160, 750, 160, (Color){40, 60, 90, 255});

                if (serverCount == 0) {
                    DrawText("Hladam servery v LAN sieti... (zatial nikto nehostuje)", 65, 175, 16, GRAY);
                    DrawText("Kamarat musi kliknut VYTVORIT SERVER a byt v tej istej sieti.", 65, 205, 15, GRAY);
                }
                for (int i = 0; i < serverCount && i < 4; i++) {
                    int rowY = 175 + i * 30;
                    Rectangle rowRec = { 55.0f, (float)rowY - 5, 690.0f, 28.0f };
                    bool hovRow = CheckCollisionPointRec(mousePos, rowRec);
                    bool isSel = (selectedIp == servers[i].ip);
                    if (isSel) {
                        DrawRectangleRec(rowRec, (Color){40, 70, 130, 255});
                        DrawRectangleLinesEx(rowRec, 1, SKYBLUE);
                    } else if (hovRow) {
                        DrawRectangleRec(rowRec, (Color){35, 45, 65, 255});
                    }
                    if (hovRow) {
                        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selectedIp = servers[i].ip;
                    }
                    DrawText(TextFormat("[LAN] %s", servers[i].name), 65, rowY, 16, WHITE);
                    DrawText(servers[i].ip, 390, rowY, 15, LIGHTGRAY);
                    DrawText("LAN", 540, rowY, 15, GREEN);
                    bool full = servers[i].players >= servers[i].maxPlayers;
                    DrawText(TextFormat("%d / %d", (int)servers[i].players, (int)servers[i].maxPlayers), 640, rowY, 15, full ? RED : YELLOW);
                }

                Rectangle joinBtn = { 50, 320, 220, 48 };
                Rectangle hostBtn = { 290, 320, 230, 48 };
                Rectangle refreshBtn = { 540, 320, 210, 48 };

                bool hovJoin = CheckCollisionPointRec(mousePos, joinBtn);
                bool hovHost = CheckCollisionPointRec(mousePos, hostBtn);
                bool hovRef = CheckCollisionPointRec(mousePos, refreshBtn);

                DrawRectangleRec(joinBtn, hovJoin ? (Color){30, 100, 200, 255} : (Color){20, 60, 140, 255});
                DrawRectangleLinesEx(joinBtn, 1, SKYBLUE);
                DrawText("PRIPOJIT SA", joinBtn.x + 45, joinBtn.y + 14, 18, WHITE);

                DrawRectangleRec(hostBtn, hovHost ? (Color){40, 140, 60, 255} : (Color){25, 90, 40, 255});
                DrawRectangleLinesEx(hostBtn, 1, GREEN);
                DrawText("VYTVORIT SERVER", hostBtn.x + 30, hostBtn.y + 14, 18, WHITE);

                DrawRectangleRec(refreshBtn, hovRef ? GRAY : (Color){50, 50, 55, 255});
                DrawRectangleLinesEx(refreshBtn, 1, LIGHTGRAY);
                DrawText("OBNOVIT ZOZNAM", refreshBtn.x + 25, refreshBtn.y + 14, 18, WHITE);

                if (hovJoin || hovHost || hovRef) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

                if (hovJoin && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (selectedIp.empty()) {
                        mpStatusMsg = "Najprv klikni na server v zozname!";
                        mpStatusTimer = 3.0f;
                    } else {
                        std::string args = "--join " + selectedIp;
                        if (!LaunchGameWithArgs(args.c_str())) {
                            mpStatusMsg = "Chyba: Game.exe nenajdeny!";
                            mpStatusTimer = 3.0f;
                        }
                    }
                }
                if (hovHost && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (!LaunchGameWithArgs("--host")) {
                        mpStatusMsg = "Chyba: Game.exe nenajdeny!";
                        mpStatusTimer = 3.0f;
                    }
                }
                if (hovRef && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Net::DiscoveryClear();
                    selectedIp.clear();
                    mpStatusMsg = "Zoznam serverov bol obnoveny. Hlada sa spojenie...";
                    mpStatusTimer = 2.0f;
                }

                if (!mpStatusMsg.empty()) {
                    DrawRectangle(50, 390, 700, 45, ColorAlpha((Color){10, 40, 80, 255}, 0.9f));
                    DrawRectangleLines(50, 390, 700, 45, SKYBLUE);
                    DrawText(mpStatusMsg.c_str(), 65, 402, 17, YELLOW);
                }

                DrawText("Tip: V Multiplayeri budete moct pretekat s kamaratmi po dedine a vymienat diely!", 50, 460, 15, LIGHTGRAY);
            }
            // --- TAB 3: CREDITY ---
            else if (activeTab == 3) {
                DrawText("AUTORI A PODPORA", 50, 70, 25, GOLD);
                DrawLine(50, 100, 300, 100, GOLD);

                DrawText("Jurmat", 60, 120, 22, GOLD);
                DrawText("Lead Texture Helper- Owner", 60, 145, 16, GRAY);
                DrawClickableLink("tiktok.com/@jurajkyselica", {60, 162}, 16, SKYBLUE, "https://www.tiktok.com/@jurajkyselica/");

                DrawText("LOM_Noob", 60, 200, 22, YELLOW);
                DrawText("Lead Programmer,Programmed Every thing, Co-Owner", 60, 225, 16, GRAY);
                DrawClickableLink("github.com/NoobRobloxdev/", {60, 242}, 16, SKYBLUE, "https://github.com/NoobRobloxdev");

                DrawText("Mnauko", 60, 280, 22, BLUE);
                DrawText("Sound Effects and Music Producer", 60, 305, 16, GRAY);
                DrawClickableLink("Link", {60, 322}, 16, SKYBLUE, "https://www.instagram.com/dj_servi.hip.pop?mdxt=Mmpranl4cHFsZ2tv");

                DrawText("Dobry Autori btw, aj tak juro neviem programovat - LOM_Noob", 50, 450, 18, LIGHTGRAY);
            }
            // --- TAB 4: SIM ---
            else if (activeTab == 4) {
                DrawText("BABETTA SIMULATOR", 50, 75, 40, YELLOW);
                DrawText("Verzia: 1.0.0", 50, 125, 20, LIGHTGRAY);
                DrawText("Programovane podla Jurmat", 50, 172, 20, GOLD);

                Rectangle simBtnRec = { 50, 215, 260, 60 };
                bool hoverSim = CheckCollisionPointRec(mousePos, simBtnRec);
                DrawRectangleRec(simBtnRec, hoverSim ? RED : MAROON);
                DrawRectangleLinesEx(simBtnRec, 2, hoverSim ? YELLOW : RED);
                DrawText("SPUSTIT SIM", 95, 232, 25, WHITE);
                if (hoverSim) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

                if (hoverSim && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) StartSim();
                if (!fs::exists("babettasim.exe")) DrawText("Chyba: babettasim.exe nenajdeny!", 50, 290, 18, RED);

                DrawRectangle(50, 320, 700, 150, (Color){35, 38, 45, 255});
                DrawRectangleLines(50, 320, 700, 150, GRAY);
                DrawText("O BABETTA SIM:", 65, 332, 18, YELLOW);
                DrawText("- Samostatna simulacna verzia hry My Summer Babetta", 65, 358, 16, WHITE);
                DrawText("- Spusta sa zo suboru babettasim.exe v priecinku launchera", 65, 380, 16, WHITE);
            }

        EndDrawing();
    }

    Net::DiscoveryStop();
    CloseWindow();
}