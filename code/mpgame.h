#pragma once
// Multiplayer cast pre Game.cpp. Vklada sa tesne PRED "int main()" (po funkcii DrawMapEnvironment),
// takze vidi DrawDetailedBabetta, DrawPlayerModel, struct Part a mod premenne.
#include "net.h"
#include <cstring>
#include <string>
#include <vector>

static const unsigned short MP_PORT = 7777;

static bool mpEnabled = false;
static std::string mpError;
static std::vector<Part> mpRemoteParts;   // vsetky diely nasadene (vzdialena Babetta je kompletna)
static NetPlayer mpOthers[8];
static int mpOtherCount = 0;
static float mpWalkAnim[8];
static Vector3 mpLastPos[8];

static void MP_Init(int argc, char** argv) {
    mpRemoteParts = {
        {{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, true, GRAY, false},
        {{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, true, BLACK, true},
        {{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, true, BLACK, true}
    };
    memset(mpWalkAnim, 0, sizeof(mpWalkAnim));
    memset(mpLastPos, 0, sizeof(mpLastPos));

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--host") == 0) {
            mpEnabled = Net::StartHost(MP_PORT);
            if (!mpEnabled) mpError = Net::Status();
        } else if (strcmp(argv[i], "--join") == 0 && i + 1 < argc) {
            mpEnabled = Net::StartClient(argv[i + 1], MP_PORT);
            if (!mpEnabled) mpError = Net::Status();
            i++;
        }
    }
}

static void MP_Update(const Camera3D& cam, bool riding, Vector3 bikePos, float bikeY, float bikeRot, float bikeLean) {
    if (!mpEnabled) return;

    NetPlayer me;
    memset(&me, 0, sizeof(me));
    me.riding = riding ? 1 : 0;
    me.bx = bikePos.x; me.by = bikePos.y + bikeY; me.bz = bikePos.z;
    me.brot = bikeRot; me.blean = bikeLean;
    if (riding) {
        me.px = me.bx; me.py = me.by; me.pz = me.bz; me.pyaw = bikeRot;
    } else {
        me.px = cam.position.x; me.py = 0.0f; me.pz = cam.position.z;
        me.pyaw = atan2f(cam.target.x - cam.position.x, cam.target.z - cam.position.z) * RAD2DEG;
    }
    Color fc = modHasCustomColor ? modBabettaColor : (Color){204, 34, 34, 255};
    me.r = fc.r; me.g = fc.g; me.b = fc.b;

    Net::Update(me);
    mpOtherCount = Net::GetRemotePlayers(mpOthers, 8);

    float dt = GetFrameTime();
    for (int i = 0; i < mpOtherCount; i++) {
        int id = mpOthers[i].id & 7;
        float dx = mpOthers[i].px - mpLastPos[id].x;
        float dz = mpOthers[i].pz - mpLastPos[id].z;
        if (dx * dx + dz * dz > 0.00005f) mpWalkAnim[id] += dt * 1.5f;
        mpLastPos[id].x = mpOthers[i].px;
        mpLastPos[id].z = mpOthers[i].pz;
    }
}

// Volat vo vnutri BeginMode3D ... EndMode3D
static void MP_Draw3D() {
    if (!mpEnabled) return;
    bool savedCustom = modHasCustomColor;
    Color savedColor = modBabettaColor;

    for (int i = 0; i < mpOtherCount; i++) {
        const NetPlayer& p = mpOthers[i];
        modHasCustomColor = true;
        modBabettaColor = (Color){p.r, p.g, p.b, 255};
        DrawDetailedBabetta({p.bx, p.by, p.bz}, p.brot, p.blean, mpRemoteParts,
                            false, false, false, false, false,
                            p.riding != 0, false, false, false);
        if (!p.riding) DrawPlayerModel({p.px, 0.0f, p.pz}, p.pyaw, false, mpWalkAnim[p.id & 7]);
    }

    modHasCustomColor = savedCustom;
    modBabettaColor = savedColor;
}

// Volat po EndMode3D (2D HUD)
static void MP_DrawHUD(const Camera3D& cam) {
    if (!mpEnabled) {
        if (!mpError.empty()) {
            DrawRectangle(840, 10, 430, 30, ColorAlpha(BLACK, 0.75f));
            DrawText(mpError.c_str(), 848, 16, 16, RED);
        }
        return;
    }

    DrawRectangle(840, 10, 430, 78, ColorAlpha(BLACK, 0.75f));
    DrawRectangleLines(840, 10, 430, 78, SKYBLUE);
    DrawText("MULTIPLAYER (TEST)", 850, 16, 16, YELLOW);
    DrawText(Net::Status(), 850, 38, 16, WHITE);
    if (Net::IsHost()) DrawText(TextFormat("Tvoja IP pre kamaratov: %s", Net::LocalIP()), 850, 60, 16, GREEN);
    else DrawText(TextFormat("Dalsich hracov: %i", mpOtherCount), 850, 60, 16, LIGHTGRAY);

    Vector3 fwd = Vector3Normalize(Vector3Subtract(cam.target, cam.position));
    for (int i = 0; i < mpOtherCount; i++) {
        const NetPlayer& p = mpOthers[i];
        Vector3 wp = {p.px, p.riding ? p.by + 2.3f : 2.3f, p.pz};
        Vector3 d = Vector3Subtract(wp, cam.position);
        if (Vector3DotProduct(d, fwd) > 0.1f && Vector3Length(d) < 200.0f) {
            Vector2 sp = GetWorldToScreen(wp, cam);
            const char* label = (p.id == 0) ? "HOST" : TextFormat("Hrac %d", (int)p.id);
            int w = MeasureText(label, 18);
            DrawText(label, (int)sp.x - w / 2, (int)sp.y, 18, YELLOW);
        }
    }
}

static void MP_Shutdown() {
    Net::Stop();
}