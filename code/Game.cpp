#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>
namespace fs = std::filesystem;
// --- GLOBALNE PREMENNE PRE MODY A TUNING ---
float modSpeedMultiplier = 1.0f;
float modMoneyBonus = 0.0f;
float modMaxSpeed = 42.0f;
// Fabricka maximalna rychlost Babetty 210 je ~40-42 KM/H
Color modBabettaColor = (Color){ 204, 34, 34, 255 };
// Predvolena cervena
bool modHasCustomColor = false;
float modGravity = 1.0f;
bool modEnableNitro = false;
bool modEnableJump = false;
bool modInfiniteFuel = false;
bool modNightMode = false;
bool modFastGrowing = false;
float modFarmRegrowTime = 300.0f;
// normalne 5 minut
bool modDisableWheatRegrow = false;
// Novy mod flag: zakaze rast psenice aj pri startovani
bool modUnlockEverything = false;
// Novy mod flag: odomkne vsetky tuningy a premium diele zadarmo
bool modEnableCustomMusic = false;
// Novy mod flag: povolena vlastna hudba
std::string modCustomMusicPath = "Sounds/birds_background.mp3";
// Cesta k vlastnej hudbe
// --- PREMYSLNE PRACIA / JOBS ---
bool modEnableJobs = false;
// Povolene jobs na zarobok
float modJobPay = 50.0f;
// Výda za jedno dokončené jobu
float modJobTimer = 10.0f;
// Čas na dokončenie jobu (sekundy)
float modRepairPay = 60.0f;
// Odmena za jednu opravenu Babetu v opravovni
float modPizzaPay = 55.0f;
// Odmena za jednu dovezenu pizzu (Pizzeria Lucia)
float jobRegrowTimer = 0.0f;
// Timer pre opakovanie jobu
bool jobIsActive = false;
// Či je momentálne job aktívny
Vector3 jobPos = { 0.0f, 0.0f, 0.0f };
// Pozícia jobu na mape
bool farmWheatReady = true;
float farmWheatRegrowTimer = 0.0f;
bool hasTuningMotor = false;
// 70cc dvojrychlostny tuning motor (+23 KM/H)
bool hasSportCarb = false;
// Sportovy karburator Dellorto (+10 KM/H)
bool hasSportExhaust = false;
// Ladeni vyfuk Giannelli (+5 KM/H)
bool hasTurboKit = false;
// Turbo kit (+10 KM/H + boost pri rozjazde)
bool hasSportSuspension = false;
// Sportova suspenzia (lepsi handling)
bool hasSportWheels = false;
// Sportove kolesa Pirelli 17" (+3 KM/H)
bool hasHeadlightKit = false;
// Vylepsene LED svetla (lazsie riadenie v noci)
bool hasAlarmSystem = false;
// Alarm system (len estetika)
// PREMIUM UPGRADY - AUTOSERVIS (drahsie, ale vyrazne lepsie ako Moto-Shop)
bool hasBigBoreKit = false;
// 90cc Big Bore Kit (+20 KM/H)
bool hasRaceCDI = false;
// Race CDI bez obmedzovaca (+8 KM/H)
bool hasTitaniumExhaust = false;
// Titanium racing vyfuk (+10 KM/H)
bool hasRaceClutch = false;
// Race spojka - rychlejsi rozjazd
bool hasBremboBrakes = false;
// Brembo racing brzdy - silnejsie brzdenie
bool hasOhlinsSuspension = false;
// Ohlins racing podvozok - lepsie zatacanie
bool ownsJawa = false;
// Vlastni Jawu 350 (rychlejsi motor)
bool isRidingJawa = false;
// Momentalne riadi Jawu namiesto Babetty
// Farby Babetty zakupene v predajni
int babettaPaintIndex = 0;
// 0=Cervena(default) 1=Modra 2=Zlata 3=Cierna 4=Zelena 5=Biela
struct Mod { std::string name; };
// Pomocne funkcie pre flexibilne citanie textov
std::string StringToUpper(std::string s) {
    for (char &c : s) c = (char)toupper((unsigned char)c);
    return s;
}
std::string StringTrim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}
std::vector<Mod> LoadAndApplyMods() {
    std::vector<Mod> loadedMods;
    if (!fs::exists("mods")) fs::create_directory("mods");
    for (const auto& entry : fs::directory_iterator("mods")) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            std::string fn = StringToUpper(entry.path().filename().string());
            if (ext == ".txt" || ext == ".cfg" || ext == ".dll" || ext == ".c") {
                loadedMods.push_back({
                    entry.path().filename().string()
                });
                // Automaticka detekcia lowgrav z nazvu suboru
                if (fn.find("LOWGRAV") != std::string::npos) {
                    modGravity = 0.35f;
                    modEnableJump = true;
                }
                if (fn.find("FASTGROW") != std::string::npos || fn.find("FAST_GROW") != std::string::npos) {
                    modFastGrowing = true;
                    modFarmRegrowTime = 0.0001f;
                    // 0.1 ms; obnovi sa v nasledujucom frame
                }
                if (fn.find("DISABLE_WHEAT") != std::string::npos || fn.find("NOWHEAT") != std::string::npos) {
                    modDisableWheatRegrow = true;
                }
                if (fn.find("UNLOCKALL") != std::string::npos || fn.find("ALL_PARTS") != std::string::npos) {
                    modUnlockEverything = true;
                }
                std::ifstream file(entry.path());
                std::string line;
                while (std::getline(file, line)) {
                    line = StringTrim(line);
                    if (line.empty() || line[0] == '#') continue;
                    std::string key, val;
                    size_t delim = line.find_first_of(":=");
                    if (delim != std::string::npos) {
                        key = StringToUpper(StringTrim(line.substr(0, delim)));
                        val = StringToUpper(StringTrim(line.substr(delim + 1)));
                    }
                    else {
                        key = StringToUpper(line);
                        val = "TRUE";
                    }
                    if (key == "SPEED" || key == "MULTIPLIER") {
                        try {
                            modSpeedMultiplier = std::stof(val);
                        }
                        catch (...) {

                        }
                    }
                    else if (key == "MAX_SPEED" || key == "MAXSPEED") {
                        try {
                            modMaxSpeed = std::stof(val);
                        }
                        catch (...) {

                        }
                    }
                    else if (key == "MONEY") {
                        try {
                            modMoneyBonus = std::stof(val);
                        }
                        catch (...) {

                        }
                    }
                    else if (key == "COLOR" || key == "BABETTA_COLOR") {
                        modHasCustomColor = true;
                        if (val.find("BLUE") != std::string::npos) modBabettaColor = (Color){ 30, 95, 230, 255 };
                        else if (val.find("GREEN") != std::string::npos) modBabettaColor = (Color){ 35, 175, 55, 255 };
                        else if (val.find("YELLOW") != std::string::npos) modBabettaColor = (Color){ 245, 215, 20, 255 };
                        else if (val.find("BLACK") != std::string::npos) modBabettaColor = (Color){ 25, 25, 28, 255 };
                        else if (val.find("GOLD") != std::string::npos) modBabettaColor = GOLD;
                        else if (val.find("PURPLE") != std::string::npos) modBabettaColor = PURPLE;
                        else if (val.find("PINK") != std::string::npos) modBabettaColor = (Color){ 245, 90, 160, 255 };
                        else if (val.find("ORANGE") != std::string::npos) modBabettaColor = ORANGE;
                        else if (val.find("WHITE") != std::string::npos) modBabettaColor = RAYWHITE;
                        else if (val.find("LIME") != std::string::npos) modBabettaColor = LIME;
                        else if (val.find("CYAN") != std::string::npos) modBabettaColor = (Color){ 20, 215, 235, 255 };
                        else modBabettaColor = RED;
                    }
                    else if (key == "NITRO" || key == "BOOST") {
                        modEnableNitro = (val != "0" && val != "FALSE");
                    }
                    else if (key == "JUMP" || key == "SUPERJUMP") {
                        modEnableJump = (val != "0" && val != "FALSE");
                    }
                    else if (key == "GRAVITY" || key == "LOWGRAV") {
                        try {
                            modGravity = std::stof(val);
                        }
                        catch (...) {
                            modGravity = 0.35f;
                        }
                    }
                    else if (key == "INFINITE_FUEL" || key == "INFINITEFUEL") {
                        modInfiniteFuel = (val != "0" && val != "FALSE");
                    }
                    else if (key == "NIGHT" || key == "NIGHTMODE") {
                        modNightMode = (val != "0" && val != "FALSE");
                    }
                    else if (key == "FAST_GROW" || key == "FASTGROW" || key == "FAST_GROWING") {
                        modFastGrowing = (val != "0" && val != "FALSE");
                        if (modFastGrowing) modFarmRegrowTime = 0.0001f;
                    }
                    else if (key == "DISABLE_WHEAT" || key == "NO_WHEAT" || key == "DISABLEWHEAT") {
                        modDisableWheatRegrow = (val != "0" && val != "FALSE");
                    }
                    else if (key == "UNLOCK_ALL" || key == "UNLOCKALL" || key == "ALL_PARTS") {
                        modUnlockEverything = (val != "0" && val != "FALSE");
                    }
                    else if (key == "WHEAT_REGROW_TIME" || key == "FARM_REGROW_TIME") {
                        try {
                            modFarmRegrowTime = std::max(0.0001f, std::stof(val));
                        }
                        catch (...) {

                        }
                    }
                    else if (key == "ENABLE_JOBS" || key == "JOBS") {
                        modEnableJobs = (val != "0" && val != "FALSE");
                    }
                    else if (key == "JOB_PAY" || key == "SALARY") {
                        try {
                            modJobPay = std::max(0.1f, std::stof(val));
                        }
                        catch (...) {

                        }
                    }
                    else if (key == "JOB_TIME" || key == "JOB_DURATION") {
                        try {
                            modJobTimer = std::max(1.0f, std::stof(val));
                        }
                        catch (...) {

                        }
                    }
                    else if (key == "REPAIR_PAY" || key == "SERVIS_PAY" || key == "OPRAVA_PAY") {
                        try {
                            modRepairPay = std::max(1.0f, std::stof(val));
                        }
                        catch (...) {

                        }
                    }
                    else if (key == "PIZZA_PAY" || key == "PIZZA_JOB_PAY" || key == "DOVOZ_PAY") {
                        try {
                            modPizzaPay = std::max(1.0f, std::stof(val));
                        }
                        catch (...) {

                        }
                    }
                    else if (key == "ENABLE_CUSTOM_MUSIC" || key == "CUSTOM_MUSIC") {
                        modEnableCustomMusic = (val != "0" && val != "FALSE");
                        if (modEnableCustomMusic && fs::exists(val)) {
                            modCustomMusicPath = val;
                        }
                    }
                    else if (key == "CUSTOM_MUSIC_PATH") {
                        modCustomMusicPath = val;
                        if (!fs::exists(modCustomMusicPath)) {
                            modCustomMusicPath = "Sounds/birds_background.mp3";
                        }
                    }
                    else if (key.find("CAMERA.POSITION.Y") != std::string::npos) {
                        modGravity = 0.35f;
                        modEnableJump = true;
                    }
                }
            }
        }
    }
    return loadedMods;
}
struct Part {
    Vector3 pos;
    Vector3 snapPos;
    bool installed;
    Color color;
    bool isSphere;
};
// Budovy na mape (vsetky umiestnene VEDLA ciest, ziadna neblokuje premavku)
const int NUM_BUILDINGS = 30;
const Vector3 buildingsPos[NUM_BUILDINGS] = {
    { -155.0f, 3.0f, 75.0f },  // 0: Dom zapad (juh od priecnej ulice)
    { 60.0f, 3.0f, 72.0f },    // 1: MOTO-TUNING SHOP (juh od priecnej ulice)
    { 155.0f, 3.0f, 75.0f },   // 2: Dom vychod (juh od priecnej ulice)
    { -155.0f, 2.75f, 130.0f },// 3: Dom severozapad (sever od priecnej ulice)
    { 155.0f, 2.75f, 130.0f }, // 4: Dom severovychod (sever od priecnej ulice)
    { 35.0f, 2.75f, 206.0f },  // 5: Dom sever (sever od severnej spojky)
    { 248.0f, 3.0f, 75.0f },   // 6: Novy dom pri Moto Dealerovi (juh od predlzenej ulice)
    { -60.0f, 3.0f, 280.0f },  // 7: Dom novej stvrte (juh od ulice Z=260)
    { 60.0f, 3.0f, 280.0f },   // 8: Dom novej stvrte (juh od ulice Z=260)
    { 140.0f, 3.0f, 280.0f },  // 9: Dom novej stvrte (juh od ulice Z=260)
    { 110.0f, 3.0f, 200.0f },  // 10: Dom pri severnej spojke (juh od Z=180)
    { -80.0f, 3.0f, 200.0f },  // 11: Dom pri severnej spojke (juh od Z=180)
    { -120.0f, 3.0f, 280.0f }, // 12: Dom novej stvrte (zapad)
    { -30.0f, 3.0f, 280.0f },  // 13: Dom novej stvrte (pri hlavnej ceste)
    { 215.0f, 2.75f, 130.0f }, // 14: Dom vychod (sever od priecnej ulice)
    { -110.0f, 2.75f, 130.0f },// 15: Dom zapad (sever od priecnej ulice)
    // NOVE DOMY POZDLZ EXISTUJUCICH ULIC - priecna ulica (Z = 100)
    { -132.0f, 3.0f, 75.0f },  // 16: zapadna cast, pri pumpe
    { -102.0f, 3.0f, 75.0f },  // 17: zapadna cast, pri pumpe
    { 90.0f, 3.0f, 75.0f },    // 18: vychodna cast
    { 120.0f, 3.0f, 75.0f },   // 19: vychodna cast
    { 180.0f, 3.0f, 75.0f },   // 20: pri dealerovi
    { -60.0f, 3.0f, 125.0f },  // 21: sever od priecnej ulice
    { -20.0f, 3.0f, 125.0f },  // 22: sever od priecnej ulice
    { 25.0f, 3.0f, 125.0f },   // 23: sever od priecnej ulice
    // Severna spojka (Z = 180)
    { -140.0f, 3.0f, 160.0f }, // 24: juh od spojky
    { 140.0f, 3.0f, 160.0f },  // 25: juh od spojky
    { -30.0f, 3.0f, 200.0f },  // 26: sever od spojky
    { 60.0f, 3.0f, 200.0f },   // 27: sever od spojky
    // Ulica novej stvrte (Z = 260)
    { -90.0f, 3.0f, 240.0f },  // 28: juh od ulice
    { 90.0f, 3.0f, 240.0f }  // 29: juh od ulice
};
const Vector3 buildingsSize[NUM_BUILDINGS] = {
    { 20.0f, 6.0f, 20.0f },
    { 24.0f, 6.5f, 20.0f },
    { 20.0f, 6.0f, 20.0f },
    { 18.0f, 5.5f, 18.0f },
    { 18.0f, 5.5f, 18.0f },
    { 18.0f, 5.5f, 18.0f },
    { 18.0f, 6.0f, 18.0f },
    { 18.0f, 6.0f, 18.0f },
    { 20.0f, 6.0f, 20.0f },
    { 18.0f, 6.0f, 18.0f },
    { 18.0f, 6.0f, 18.0f },
    { 18.0f, 6.0f, 18.0f },
    { 18.0f, 6.0f, 18.0f },
    { 18.0f, 6.0f, 18.0f },
    { 18.0f, 5.5f, 18.0f },
    { 18.0f, 5.5f, 18.0f },    // Juzna stvrt
    { 18.0f, 6.0f, 18.0f },
    { 18.0f, 6.0f, 18.0f },
    { 18.0f, 6.0f, 18.0f },
    { 18.0f, 6.0f, 18.0f },
    { 18.0f, 6.0f, 18.0f },
    { 18.0f, 6.0f, 18.0f },
    { 18.0f, 6.0f, 18.0f },
    { 18.0f, 6.0f, 18.0f },
    { 20.0f, 6.0f, 20.0f },
    { 18.0f, 6.0f, 18.0f },
    { 18.0f, 6.0f, 18.0f },
    { 20.0f, 6.0f, 20.0f },
    { 18.0f, 6.0f, 18.0f },
    { 18.0f, 6.0f, 18.0f }
};
const Color buildingWallColors[NUM_BUILDINGS] = {
    ORANGE,     (Color){
        30, 40, 60, 255
    },     YELLOW,     (Color){
        195, 75, 75, 255
    },     (Color){
        75, 115, 145, 255
    },    (Color){
        90, 140, 90, 255
    },    (Color){
        175, 182, 196, 255
    },    (Color){
        120, 150, 170, 255
    },    (Color){
        200, 160, 110, 255
    },    (Color){
        160, 120, 90, 255
    },    (Color){
        150, 130, 175, 255
    },    (Color){
        200, 120, 120, 255
    },    (Color){
        130, 165, 140, 255
    },    (Color){
        185, 185, 195, 255
    },    (Color){
        215, 200, 150, 255
    },    (Color){
        140, 110, 130, 255
    },    // Juzna stvrt    (Color)
    { 190, 120, 100, 255 },    (Color){
        105, 135, 165, 255
    },    (Color){
        205, 185, 140, 255
    },    (Color){
        135, 155, 125, 255
    },    (Color){
        175, 145, 190, 255
    },    (Color){
        210, 160, 130, 255
    },    (Color){
        120, 165, 170, 255
    },    (Color){
        195, 175, 110, 255
    },    (Color){
        160, 135, 115, 255
    },    (Color){
        130, 145, 180, 255
    },    (Color){
        190, 160, 170, 255
    },    (Color){
        145, 175, 150, 255
    },    (Color){
        200, 190, 160, 255
    },    (Color){
        155, 120, 140, 255
    }
};
const Color buildingRoofColors[NUM_BUILDINGS] = {
    DARKBROWN,     (Color){
        20, 25, 35, 255
    },     BROWN,     MAROON,     (Color){
        35, 45, 55, 255
    },    (Color){
        40, 50, 40, 255
    },    (Color){
        80, 90, 110, 255
    },    (Color){
        50, 60, 75, 255
    },    BROWN,    (Color){
        70, 45, 30, 255
    },    (Color){
        55, 45, 70, 255
    },    (Color){
        90, 50, 50, 255
    },    (Color){
        45, 65, 55, 255
    },    (Color){
        60, 60, 70, 255
    },    (Color){
        100, 85, 50, 255
    },    (Color){
        60, 40, 55, 255
    },    // Juzna stvrt    (Color)
    { 80, 45, 40, 255 },    (Color){
        45, 55, 75, 255
    },    (Color){
        95, 75, 40, 255
    },    (Color){
        50, 65, 50, 255
    },    (Color){
        65, 50, 80, 255
    },    (Color){
        85, 55, 45, 255
    },    (Color){
        40, 65, 70, 255
    },    (Color){
        90, 80, 35, 255
    },    (Color){
        60, 50, 45, 255
    },    (Color){
        45, 55, 85, 255
    },    (Color){
        80, 60, 70, 255
    },    (Color){
        50, 70, 55, 255
    },    (Color){
        95, 90, 70, 255
    },    (Color){
        65, 45, 60, 255
    }
};
// Smer, ktorym je dom otoceny (vjazd k najblizsej ulici): +1 = +Z, -1 = -Z
const int buildingFrontDir[NUM_BUILDINGS] = {
    1, 1, 1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1,    1, 1, 1, 1, 1,     // 16-20 domy pri priecnej ulici (Z=100), vjazd na sever    -1, -1, -1,
    // 21-23 domy severne od priecnej ulice, vjazd na juh    1, 1,
    // 24-25 domy pri spojke (Z=180), vjazd na sever    -1, -1,
    // 26-27 domy severne od spojky, vjazd na juh    1, 1
    // 28-29 domy pri ulici Z=260, vjazd na sever
};
// Pozicie specialnych objektov (mimo cesty so samostatnym vjazdom)
const Vector3 shopCounterPos = { 60.0f, 0.0f, 84.5f };
// Pult MotoShopu
const Vector3 gasStationPos = { -70.0f, 0.0f, 75.0f };
// Cerpacia stanica
const Vector3 gasPump1Pos = { -73.0f, 0.0f, 75.0f };
// Stojan MIX 2T
const Vector3 gasPump2Pos = { -67.0f, 0.0f, 75.0f };
// Stojan N95
// Delivery job location (vedla priecnej ulice, blizu czerpackej stanice)
const Vector3 jobDeliveryPos = { -72.0f, 0.0f, 78.0f };
// Misto pre dodanie balikov
// Jawa + Babetta DEALER (daleko vychod, vedla priecnej ulice, mimo domov)
const Vector3 dealerPos = { 210.0f, 0.0f, 75.0f };
// Budova dealera vedla cesty
const Vector3 dealerCounterPos = { 210.0f, 0.0f, 86.0f };
// Pult dealera
const Vector3 jawaShowroomPos = { 203.0f, 0.0f, 70.0f };
// Jawa v showroome
const Vector3 babettaShowroomPos = { 217.0f, 0.0f, 70.0f };
// Babetta farby v showroome
const Vector3 jawaShowroom2Pos = { 203.0f, 0.0f, 80.0f };
// druha Jawa v showroome
const Vector3 babettaShowroom2Pos = { 217.0f, 0.0f, 80.0f };
// druha Babetta v showroome
// Farma je na sever od dediny. Texty pouzivaju ASCII, aby fungovali aj bez UTF-8 nastavenia kompilatora.
const Vector3 farmWheatPickupPos = { 105.0f, 0.0f, 450.0f };
const Vector3 farmSellingStandPos = { 35.0f, 0.0f, 448.0f };
const int farmWheatLoad = 20;
const float farmWheatPrice = 10.0f;
// AUTOSERVIS (premium tuning) a RESTAURACIA (slovenske jedla)
const Vector3 mechanicCounterPos = { -18.0f, 0.0f, 324.0f };
// Restauracia na vychodnej strane hlavnej cesty (povodne miesto).
const Vector3 restaurantPos = { -30.0f, 3.0f, 75.0f };
const Vector3 restaurantCounterPos = { -30.0f, 0.0f, 87.0f };
// OPRAVOVNA BABETT - dielna na vychodnej strane hlavnej cesty (povodne miesto).
const Vector3 repairShopPos = { 100.0f, 3.0f, 290.0f };
const Vector3 repairCounterPos = { 100.0f, 0.0f, 272.0f };
// PIZZERIA LUCIA - predna cast (predaj pizze) a zadna cast (job dovozu pizze).
const Vector3 pizzeriaPos = { 90.0f, 3.0f, 125.0f };
const Vector3 pizzeriaCounterPos = { 90.0f, 0.0f, 113.5f };
const Vector3 pizzaJobPos = { 90.0f, 0.0f, 136.5f };
// Ciele rozvozu pizze (nazov + pozicia)
const int PIZZA_TARGET_COUNT = 4;
const Vector3 pizzaTargets[PIZZA_TARGET_COUNT] = {
    { 35.0f, 0.0f, 445.0f },   // Farma - predajny stanok
    { -70.0f, 0.0f, 78.0f },   // Cerpacia stanica
    { 210.0f, 0.0f, 97.0f },   // Moto Dealer
    { 100.0f, 0.0f, 266.0f }  // Opravovna Babett
};
const char* pizzaTargetNames[PIZZA_TARGET_COUNT] = { "Farma", "Cerpacia stanica", "Moto Dealer", "Opravovna" };
bool CheckCollisionWithWorld(Vector3 p, float radius) {
    BoundingBox entityBox = {
        { p.x - radius, 0, p.z - radius },
        { p.x + radius, 2.0f, p.z + radius }
    };
    Vector3 gPos = { 20.0f, 0.0f, 20.0f };
    // Steny garaze
    if (CheckCollisionBoxes(entityBox, (BoundingBox){
        { gPos.x - 7.5f, 0, gPos.z - 7.7f },
        {
            gPos.x + 7.5f, 5, gPos.z - 7.3f
        }
    })) return true;
    if (CheckCollisionBoxes(entityBox, (BoundingBox){
        { gPos.x - 7.7f, 0, gPos.z - 7.5f },
        {
            gPos.x - 7.3f, 5, gPos.z + 7.5f
        }
    })) return true;
    if (CheckCollisionBoxes(entityBox, (BoundingBox){
        { gPos.x + 7.3f, 0, gPos.z - 7.5f },
        {
            gPos.x + 7.7f, 5, gPos.z + 7.5f
        }
    })) return true;
    // Budovy v dedine
    for (int i = 0; i < NUM_BUILDINGS; i++) {
        BoundingBox bBox = {
            { buildingsPos[i].x - buildingsSize[i].x * 0.5f, 0, buildingsPos[i].z - buildingsSize[i].z * 0.5f },
            { buildingsPos[i].x + buildingsSize[i].x * 0.5f, buildingsSize[i].y, buildingsPos[i].z + buildingsSize[i].z * 0.5f }
        };
        if (CheckCollisionBoxes(entityBox, bBox)) return true;
    }
    // Predajny pult obchodu
    BoundingBox counterBox = {
        { shopCounterPos.x - 2.5f, 0, shopCounterPos.z - 0.8f },
        { shopCounterPos.x + 2.5f, 1.5f, shopCounterPos.z + 0.8f }
    };
    if (CheckCollisionBoxes(entityBox, counterBox)) return true;
    // Budova obchodu na pumpe
    BoundingBox gasShopBox = {
        { -78.0f, 0, 54.0f },
        { -62.0f, 5.0f, 66.0f }
    };
    if (CheckCollisionBoxes(entityBox, gasShopBox)) return true;
    // Stojany pumpy
    BoundingBox pump1Box = {
        { gasPump1Pos.x - 0.6f, 0, gasPump1Pos.z - 1.2f },
        { gasPump1Pos.x + 0.6f, 2.0f, gasPump1Pos.z + 1.2f }
    };
    BoundingBox pump2Box = {
        { gasPump2Pos.x - 0.6f, 0, gasPump2Pos.z - 1.2f },
        { gasPump2Pos.x + 0.6f, 2.0f, gasPump2Pos.z + 1.2f }
    };
    if (CheckCollisionBoxes(entityBox, pump1Box) || CheckCollisionBoxes(entityBox, pump2Box)) return true;
    // Piliere strechy pumpy
    float pX[4] = { -78.0f, -78.0f, -62.0f, -62.0f };
    float pZ[4] = { 69.0f, 81.0f, 69.0f, 81.0f };
    for (int i = 0; i < 4; i++) {
        BoundingBox pillarBox = {
            { pX[i] - 0.4f, 0, pZ[i] - 0.4f },
            { pX[i] + 0.4f, 5.0f, pZ[i] + 0.4f }
        };
        if (CheckCollisionBoxes(entityBox, pillarBox)) return true;
    }
    // Cenovy totem pumpy
    BoundingBox totemBox = {
        { -54.0f, 0, 87.0f },
        { -52.0f, 6.0f, 89.0f }
    };
    if (CheckCollisionBoxes(entityBox, totemBox)) return true;
    // Budova DEALERA (Jawa + Babetta predajna)
    BoundingBox dealerBuildingBox = {
        { dealerPos.x - 15.0f, 0, dealerPos.z - 12.0f },
        { dealerPos.x + 15.0f, 6.0f, dealerPos.z + 12.0f }
    };
    if (CheckCollisionBoxes(entityBox, dealerBuildingBox)) return true;
    // Pult dealera
    BoundingBox dealerCtrBox = {
        { dealerCounterPos.x - 3.0f, 0, dealerCounterPos.z - 0.8f },
        { dealerCounterPos.x + 3.0f, 1.5f, dealerCounterPos.z + 0.8f }
    };
    if (CheckCollisionBoxes(entityBox, dealerCtrBox)) return true;
    // Budova AUTOSERVISU (placeholder)
    BoundingBox autoServiceBox = {
        { -48.0f, 0, 320.0f },
        { -20.0f, 8.0f, 340.0f }
    };
    if (CheckCollisionBoxes(entityBox, autoServiceBox)) return true;
    // Pult autoservisu
    BoundingBox mechCtrBox = {
        { mechanicCounterPos.x - 2.5f, 0, mechanicCounterPos.z - 0.8f },
        { mechanicCounterPos.x + 2.5f, 1.5f, mechanicCounterPos.z + 0.8f }
    };
    if (CheckCollisionBoxes(entityBox, mechCtrBox)) return true;
    // Budova RESTAURACIE
    BoundingBox restaurantBox = {
        { restaurantPos.x - 12.0f, 0, restaurantPos.z - 10.0f },
        { restaurantPos.x + 12.0f, 6.0f, restaurantPos.z + 10.0f }
    };
    if (CheckCollisionBoxes(entityBox, restaurantBox)) return true;
    // Pult restauracie
    BoundingBox restCtrBox = {
        { restaurantCounterPos.x - 2.0f, 0, restaurantCounterPos.z - 0.8f },
        { restaurantCounterPos.x + 2.0f, 1.5f, restaurantCounterPos.z + 0.8f }
    };
    if (CheckCollisionBoxes(entityBox, restCtrBox)) return true;
    // Budova OPRAVOVNE BABETT
    BoundingBox repairShopBox = {
        { repairShopPos.x - 12.0f, 0, repairShopPos.z - 10.0f },
        { repairShopPos.x + 12.0f, 6.0f, repairShopPos.z + 10.0f }
    };
    if (CheckCollisionBoxes(entityBox, repairShopBox)) return true;
    // Pult opravovne
    BoundingBox repairCtrBox = {
        { repairCounterPos.x - 2.0f, 0, repairCounterPos.z - 0.8f },
        { repairCounterPos.x + 2.0f, 1.5f, repairCounterPos.z + 0.8f }
    };
    if (CheckCollisionBoxes(entityBox, repairCtrBox)) return true;
    // Budova PIZZERIE LUCIA (predaj pizze + zadny job dovozu)
    BoundingBox pizzeriaBox = {
        { pizzeriaPos.x - 9.0f, 0, pizzeriaPos.z - 8.0f },
        { pizzeriaPos.x + 9.0f, 6.0f, pizzeriaPos.z + 8.0f }
    };
    if (CheckCollisionBoxes(entityBox, pizzeriaBox)) return true;
    // Predny pult pizzerie
    BoundingBox pizzaCtrBox = {
        { pizzeriaCounterPos.x - 2.3f, 0, pizzeriaCounterPos.z - 0.8f },
        { pizzeriaCounterPos.x + 2.3f, 1.5f, pizzeriaCounterPos.z + 0.8f }
    };
    if (CheckCollisionBoxes(entityBox, pizzaCtrBox)) return true;
    // Zadny pult dovozu pizze
    BoundingBox pizzaJobBox = {
        { pizzaJobPos.x - 1.8f, 0, pizzaJobPos.z - 0.8f },
        { pizzaJobPos.x + 1.8f, 1.5f, pizzaJobPos.z + 0.8f }
    };
    if (CheckCollisionBoxes(entityBox, pizzaJobBox)) return true;
    // FARMA - stodola (DrawCube 150,2.5,520 | 18x5x14)
    BoundingBox barnBox = {
        { 141.0f, 0, 513.0f },
        { 159.0f, 5.0f, 527.0f }
    };
    if (CheckCollisionBoxes(entityBox, barnBox)) return true;
    // FARMA - silaz (DrawCylinder 137,0,520 | radius 3, vyska 8)
    BoundingBox siloBox = {
        { 134.0f, 0, 517.0f },
        { 140.0f, 8.0f, 523.0f }
    };
    if (CheckCollisionBoxes(entityBox, siloBox)) return true;
    // FARMA - predajny stanok (farmSellingStandPos, 5.5x2.5x3)
    BoundingBox farmStandBox = {
        { farmSellingStandPos.x - 2.75f, 0, farmSellingStandPos.z - 1.5f },
        { farmSellingStandPos.x + 2.75f, 2.5f, farmSellingStandPos.z + 1.5f }
    };
    if (CheckCollisionBoxes(entityBox, farmStandBox)) return true;
    return false;
}
void DrawFullGarage(Vector3 pos) {
    DrawCube({
        pos.x, pos.y + 0.04f, pos.z
    }, 15.0f, 0.08f, 15.0f, LIGHTGRAY);
    DrawCubeWires({
        pos.x, pos.y + 0.04f, pos.z
    }, 15.0f, 0.08f, 15.0f, GRAY);
    DrawCube({
        pos.x, pos.y + 2.5f, pos.z - 7.5f
    }, 15, 5, 0.2f, BEIGE);
    DrawCube({
        pos.x - 7.5f, pos.y + 2.5f, pos.z
    }, 0.2f, 5, 15, BEIGE);
    DrawCube({
        pos.x + 7.5f, pos.y + 2.5f, pos.z
    }, 0.2f, 5, 15, BEIGE);
    DrawCube({
        pos.x, pos.y + 5.0f, pos.z
    }, 15.2f, 0.3f, 15.2f, DARKGRAY);
    DrawCube({
        pos.x - 7.0f, pos.y + 1.5f, pos.z
    }, 1, 0.1f, 10, BROWN);
}
void DrawShopkeeperJozo(Vector3 pos) {
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);
    DrawCube({
        0.0f, 0.95f, 0.0f
    }, 0.5f, 0.7f, 0.3f, (Color){
        20, 60, 140, 255
    });
    DrawCube({
        0.0f, 1.25f, 0.0f
    }, 0.45f, 0.2f, 0.28f, RED);
    DrawCube({
        0.0f, 1.55f, 0.0f
    }, 0.28f, 0.28f, 0.28f, (Color){
        255, 205, 160, 255
    });
    DrawCube({
        0.0f, 1.50f, 0.15f
    }, 0.16f, 0.06f, 0.05f, (Color){
        60, 30, 20, 255
    });
    DrawCube({
        -0.06f, 1.58f, 0.15f
    }, 0.04f, 0.04f, 0.02f, BLACK);
    DrawCube({
        0.06f, 1.58f, 0.15f
    }, 0.04f, 0.04f, 0.02f, BLACK);
    DrawCube({
        0.0f, 1.72f, 0.0f
    }, 0.32f, 0.08f, 0.32f, RED);
    DrawCube({
        0.0f, 1.70f, 0.20f
    }, 0.30f, 0.03f, 0.18f, RED);
    DrawCube({
        -0.32f, 1.10f, 0.10f
    }, 0.12f, 0.45f, 0.12f, (Color){
        20, 60, 140, 255
    });
    DrawCube({
        0.32f, 1.10f, 0.10f
    }, 0.12f, 0.45f, 0.12f, (Color){
        20, 60, 140, 255
    });
    DrawCube({
        0.32f, 0.90f, 0.20f
    }, 0.04f, 0.30f, 0.04f, LIGHTGRAY);
    DrawCube({
        0.32f, 1.05f, 0.20f
    }, 0.10f, 0.08f, 0.06f, LIGHTGRAY);
    rlPopMatrix();
}
void DrawBuilding(int idx, Vector3 pos, Vector3 size, Color wallColor, Color roofColor) {
    DrawCube(pos, size.x, size.y, size.z, wallColor);
    DrawCubeWires(pos, size.x, size.y, size.z, DARKGRAY);
    DrawCube({
        pos.x, pos.y + size.y * 0.5f + 0.7f, pos.z
    }, size.x + 0.8f, 1.4f, size.z + 0.8f, roofColor);
    // Dvere a okna otocene k najblizsej ulici
    float doorDir = (float)buildingFrontDir[idx];
    float frontZ = pos.z + doorDir * (size.z * 0.5f + 0.1f);
    DrawCube({
        pos.x, pos.y - size.y * 0.5f + 1.25f, frontZ
    }, 2.2f, 2.5f, 0.2f, DARKBROWN);
    DrawCube({
        pos.x + size.x * 0.25f, pos.y + 0.5f, frontZ
    }, 2.0f, 2.0f, 0.2f, SKYBLUE);
    DrawCube({
        pos.x - size.x * 0.25f, pos.y + 0.5f, frontZ
    }, 2.0f, 2.0f, 0.2f, SKYBLUE);
    if (idx == 1) {
        // MOTO-TUNING SHOP
        // Reklamny neon / tabula
        DrawCube({
            pos.x, pos.y + size.y * 0.5f + 1.6f, frontZ + doorDir * 0.3f
        }, 20.0f, 2.4f, 0.4f, (Color){
            15, 15, 20, 255
        });
        DrawCubeWires({
            pos.x, pos.y + size.y * 0.5f + 1.6f, frontZ + doorDir * 0.3f
        }, 20.0f, 2.4f, 0.4f, GOLD);
        DrawCube({
            pos.x, pos.y + size.y * 0.5f + 1.6f, frontZ + doorDir * 0.55f
        }, 18.0f, 1.2f, 0.1f, (Color){
            230, 20, 20, 255
        });
        DrawCube({
            pos.x, pos.y + size.y * 0.5f + 1.6f, frontZ + doorDir * 0.62f
        }, 16.0f, 0.6f, 0.05f, YELLOW);
        // Sachovnica pod tabulou
        for (int c = -8; c <= 8; c++) {
            DrawCube({
                pos.x + (float)c * 1.1f, pos.y + size.y * 0.5f + 0.2f, frontZ + doorDir * 0.25f
            }, 1.0f, 0.35f, 0.1f, (c % 2 == 0) ? WHITE : BLACK);
        }
        // Pult a Jozo pred predajnou
        DrawCube({
            shopCounterPos.x, shopCounterPos.y + 0.6f, shopCounterPos.z
        }, 4.2f, 1.2f, 1.0f, (Color){
            40, 25, 20, 255
        });
        DrawCube({
            shopCounterPos.x, shopCounterPos.y + 1.25f, shopCounterPos.z
        }, 4.6f, 0.1f, 1.2f, (Color){
            60, 40, 30, 255
        });
        DrawShopkeeperJozo({
            shopCounterPos.x, shopCounterPos.y, shopCounterPos.z - 0.7f
        });
        // Kopy pneumatik
        for (int t = 0; t < 3; t++) {
            DrawCylinderEx({
                shopCounterPos.x - 3.2f, 0.15f + (float)t * 0.32f, shopCounterPos.z
            },
            { shopCounterPos.x - 3.2f, 0.45f + (float)t * 0.32f, shopCounterPos.z }, 0.45f, 0.45f, 14, BLACK);
            DrawCylinderEx({
                shopCounterPos.x + 3.2f, 0.15f + (float)t * 0.32f, shopCounterPos.z
            },
            { shopCounterPos.x + 3.2f, 0.45f + (float)t * 0.32f, shopCounterPos.z }, 0.45f, 0.45f, 14, BLACK);
        }
        // Olejove sudy Castrol a Motul
        DrawCylinder({
            shopCounterPos.x - 4.5f, 0.0f, shopCounterPos.z + 0.5f
        }, 0.4f, 0.4f, 1.1f, 12, (Color){
            20, 120, 20, 255
        });
        DrawCylinder({
            shopCounterPos.x + 4.5f, 0.0f, shopCounterPos.z + 0.5f
        }, 0.4f, 0.4f, 1.1f, 12, (Color){
            200, 30, 30, 255
        });
    }
}
void DrawJawa(Vector3 pos, float rotDeg, Color frameCol) {
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);
    rlRotatef(rotDeg, 0, 1, 0);
    Color chrome = (Color){ 200, 200, 210, 255 };
    Color darkMetal = (Color){ 40, 40, 45, 255 };
    // RAM a motor (Jawa 350 Californian)
    DrawCube({
        0.0f, 0.60f, 0.0f
    }, 0.13f, 0.14f, 1.55f, frameCol);
    DrawCube({
        0.0f, 0.90f, 0.50f
    }, 0.13f, 0.60f, 0.13f, frameCol);
    DrawCube({
        0.0f, 1.18f, 0.18f
    }, 0.12f, 0.12f, 0.80f, frameCol);
    // Velky motor (350cc)
    DrawCube({
        0.0f, 0.42f, 0.10f
    }, 0.38f, 0.48f, 0.65f, darkMetal);
    DrawCube({
        0.0f, 0.42f, 0.12f
    }, 0.42f, 0.22f, 0.60f, (Color){
        55, 55, 60, 255
    });
    // Valec + hlava valca
    DrawCylinder({
        0.0f, 0.68f, 0.08f
    }, 0.14f, 0.14f, 0.35f, 10, (Color){
        80,80,85,255
    });
    DrawCube({
        0.0f, 0.86f, 0.08f
    }, 0.32f, 0.07f, 0.32f, darkMetal);
    // Vyfukova rura (velka)
    DrawCylinderEx({
        0.22f, 0.42f, -0.35f
    },
    { 0.22f, 0.28f, -0.70f }, 0.055f, 0.045f, 10, chrome);
    DrawCylinderEx({
        0.22f, 0.28f, -0.70f
    },
    { 0.22f, 0.22f, -0.78f }, 0.07f, 0.09f, 10, (Color){
        120,100,60,255
    });
    // Palivova nadrz (velka chromed tank)
    DrawCube({
        0.0f, 1.22f, 0.0f
    }, 0.44f, 0.28f, 0.90f, frameCol);
    DrawCubeWires({
        0.0f, 1.22f, 0.0f
    }, 0.44f, 0.28f, 0.90f, DARKGRAY);
    DrawCube({
        0.0f, 1.26f, 0.0f
    }, 0.46f, 0.08f, 0.92f, chrome);
    // Sedlo (dlhsie pre Jawu)
    DrawCube({
        0.0f, 1.30f, -0.28f
    }, 0.36f, 0.09f, 0.80f, BLACK);
    DrawCube({
        0.0f, 1.35f, -0.28f
    }, 0.32f, 0.07f, 0.78f, (Color){
        50,30,20,255
    });
    // Predny vidlice
    DrawCylinderEx({
        -0.10f, 0.98f, 0.76f
    },
    { -0.10f, 0.38f, 0.78f }, 0.055f, 0.055f, 8, chrome);
    DrawCylinderEx({
        0.10f, 0.98f, 0.76f
    },
    { 0.10f, 0.38f, 0.78f }, 0.055f, 0.055f, 8, chrome);
    // Riadidla (sirokie cruiser style)
    DrawCube({
        0.0f, 1.22f, 0.75f
    }, 0.90f, 0.05f, 0.05f, chrome);
    DrawCube({
        -0.45f, 1.22f, 0.75f
    }, 0.05f, 0.14f, 0.05f, chrome);
    DrawCube({
        0.45f, 1.22f, 0.75f
    }, 0.05f, 0.14f, 0.05f, chrome);
    // Kolesa (vacsi priemer)
    DrawCylinderEx({
        -0.18f, 0.36f, 0.78f
    },
    { 0.18f, 0.36f, 0.78f }, 0.36f, 0.36f, 18, BLACK);
    DrawCylinderEx({
        -0.18f, 0.36f, -0.78f
    },
    { 0.18f, 0.36f, -0.78f }, 0.36f, 0.36f, 18, BLACK);
    DrawCylinderEx({
        -0.12f, 0.36f, 0.78f
    },
    { 0.12f, 0.36f, 0.78f }, 0.20f, 0.20f, 12, chrome);
    DrawCylinderEx({
        -0.12f, 0.36f, -0.78f
    },
    { 0.12f, 0.36f, -0.78f }, 0.20f, 0.20f, 12, chrome);
    // Predne + zadne svetlo
    DrawCube({
        0.0f, 1.05f, 0.82f
    }, 0.18f, 0.14f, 0.09f, chrome);
    DrawSphere({
        0.0f, 1.05f, 0.86f
    }, 0.07f, YELLOW);
    DrawCube({
        0.0f, 1.08f, -0.86f
    }, 0.14f, 0.09f, 0.07f, chrome);
    DrawSphere({
        0.0f, 1.08f, -0.89f
    }, 0.045f, RED);
    rlPopMatrix();
}
void DrawShowroomBabetta(Vector3 pos, float rotDeg, Color frameColor);
void DrawMotoDealer() {
    // BUDOVA dealera - sklenna fasada, moderna
    Vector3 bPos = { dealerPos.x, 3.0f, dealerPos.z };
    DrawCube(bPos, 30.0f, 6.0f, 24.0f, (Color){
        235, 238, 245, 255
    });
    DrawCubeWires(bPos, 30.0f, 6.0f, 24.0f, (Color){
        100, 100, 110, 255
    });
    // Strecha
    DrawCube({
        bPos.x, 6.1f, bPos.z
    }, 30.6f, 0.5f, 24.6f, (Color){
        30, 35, 45, 255
    });
    DrawCubeWires({
        bPos.x, 6.1f, bPos.z
    }, 30.6f, 0.5f, 24.6f, (Color){
        80, 110, 180, 255
    });
    // Velkoplosne sklenne vitrine - predna stena
    float frontZ = dealerPos.z + 12.1f;
    DrawCube({
        bPos.x, 2.5f, frontZ
    }, 28.0f, 4.5f, 0.15f, (Color){
        120, 170, 220, 180
    });
    DrawCubeWires({
        bPos.x, 2.5f, frontZ
    }, 28.0f, 4.5f, 0.15f, (Color){
        60, 90, 140, 255
    });
    // Sklenene stlpy fasady
    for (int i = -3; i <= 3; i++) {
        DrawCube({
            bPos.x + i * 4.5f, 2.5f, frontZ
        }, 0.22f, 5.1f, 0.22f, (Color){
            50, 60, 80, 255
        });
    }
    // Neonova reklamna tabula
    DrawCube({
        bPos.x, 6.5f, frontZ + 0.3f
    }, 28.0f, 2.0f, 0.4f, (Color){
        10, 12, 20, 255
    });
    DrawCubeWires({
        bPos.x, 6.5f, frontZ + 0.3f
    }, 28.0f, 2.0f, 0.4f, (Color){
        80, 110, 255, 255
    });
    DrawCube({
        bPos.x, 6.5f, frontZ + 0.55f
    }, 25.0f, 1.0f, 0.08f, (Color){
        60, 90, 255, 255
    });
    // Dvere (priehladne)
    DrawCube({
        bPos.x, 1.5f, frontZ
    }, 3.5f, 2.8f, 0.08f, (Color){
        160, 200, 240, 150
    });
    DrawCubeWires({
        bPos.x, 1.5f, frontZ
    }, 3.5f, 2.8f, 0.08f, SKYBLUE);
    // Parkovisko pred dealerom
    DrawCube({
        bPos.x, 0.045f, dealerPos.z + 20.0f
    }, 32.0f, 0.07f, 14.0f, (Color){
        70, 72, 76, 255
    });
    // Parkovacie ciary
    for (int i = -3; i <= 3; i++) {
        DrawCube({
            bPos.x + i * 4.8f, 0.055f, dealerPos.z + 22.0f
        }, 0.15f, 0.02f, 10.0f, (Color){
            240,240,240,255
        });
    }
    // Pult predavaca vo vnutri
    DrawCube({
        dealerCounterPos.x, dealerCounterPos.y + 0.65f, dealerCounterPos.z
    }, 5.5f, 1.3f, 1.1f, (Color){
        35, 20, 15, 255
    });
    DrawCube({
        dealerCounterPos.x, dealerCounterPos.y + 1.35f, dealerCounterPos.z
    }, 5.9f, 0.1f, 1.3f, (Color){
        55, 35, 25, 255
    });
    // Podstavce s motorkami v showroome (2x Jawa, 2x Babetta)
    DrawCube({
        jawaShowroomPos.x, 0.1f, jawaShowroomPos.z
    }, 3.2f, 0.2f, 2.2f, (Color){
        60, 65, 75, 255
    });
    DrawCube({
        babettaShowroomPos.x, 0.1f, babettaShowroomPos.z
    }, 3.0f, 0.2f, 2.0f, (Color){
        60, 65, 75, 255
    });
    DrawCube({
        jawaShowroom2Pos.x, 0.1f, jawaShowroom2Pos.z
    }, 3.2f, 0.2f, 2.2f, (Color){
        60, 65, 75, 255
    });
    DrawCube({
        babettaShowroom2Pos.x, 0.1f, babettaShowroom2Pos.z
    }, 3.0f, 0.2f, 2.0f, (Color){
        60, 65, 75, 255
    });
    // Jawa 1 a 2
    DrawJawa({
        jawaShowroomPos.x, 0.2f, jawaShowroomPos.z
    }, 20.0f, (Color){
        180, 20, 20, 255
    });
    DrawJawa({
        jawaShowroom2Pos.x, 0.2f, jawaShowroom2Pos.z
    }, -18.0f, (Color){
        30, 70, 150, 255
    });
    // Babetta 1 a 2 (nove vystavne kusy)
    DrawShowroomBabetta({
        babettaShowroomPos.x, 0.2f, babettaShowroomPos.z
    }, -12.0f, (Color){
        25, 110, 60, 255
    });
    DrawShowroomBabetta({
        babettaShowroom2Pos.x, 0.2f, babettaShowroom2Pos.z
    }, 15.0f, (Color){
        210, 120, 20, 255
    });
    // Cedula JAWA nad vystavnou Jawou
    DrawCube({
        jawaShowroomPos.x, 2.6f, jawaShowroomPos.z - 0.4f
    }, 2.4f, 0.7f, 0.1f, (Color){
        20, 20, 25, 255
    });
    // Cedula BABETTA nad vystavnymi Babettami
    DrawCube({
        babettaShowroomPos.x, 2.4f, babettaShowroomPos.z - 0.4f
    }, 2.4f, 0.6f, 0.1f, (Color){
        20, 20, 25, 255
    });
    DrawCube({
        babettaShowroom2Pos.x, 2.4f, babettaShowroom2Pos.z - 0.4f
    }, 2.4f, 0.6f, 0.1f, (Color){
        20, 20, 25, 255
    });
    // Vlajky pri vchode
    DrawCylinder({
        bPos.x - 13.0f, 0.0f, frontZ + 2.0f
    }, 0.1f, 0.1f, 5.5f, 8, (Color){
        180, 180, 185, 255
    });
    DrawCylinder({
        bPos.x + 13.0f, 0.0f, frontZ + 2.0f
    }, 0.1f, 0.1f, 5.5f, 8, (Color){
        180, 180, 185, 255
    });
    DrawCube({
        bPos.x - 12.6f, 4.8f, frontZ + 2.0f
    }, 1.6f, 0.9f, 0.08f, RED);
    DrawCube({
        bPos.x + 12.6f, 4.8f, frontZ + 2.0f
    }, 1.6f, 0.9f, 0.08f, (Color){
        0, 50, 180, 255
    });
}
// PLACEHOLDER AUTOSERVIS - docasna jednoducha budova (dve garazove brany, dvor, reklamna lista).
void DrawAutoService() {
    const Vector3 p = { -34.0f, 4.0f, 330.0f };
    // stred budovy, zapadna strana hlavnej cesty
    Color wall = (Color){ 172, 124, 74, 255 };
    Color wallTrim = (Color){ 120, 84, 50, 255 };
    Color roof = (Color){ 55, 58, 64, 255 };
    Color door = (Color){ 48, 78, 112, 255 };
    // Dvor / predajna plocha medzi budovou a hlavnou cestou
    DrawCube({
        -13.0f, 0.045f, p.z
    }, 16.0f, 0.07f, 24.0f, (Color){
        70, 72, 76, 255
    });
    // Hlavna budova
    DrawCube(p, 28.0f, 8.0f, 20.0f, wall);
    DrawCubeWires(p, 28.0f, 8.0f, 20.0f, DARKGRAY);
    // Soklova lista po obvode
    DrawCube({
        p.x, 0.6f, p.z
    }, 28.3f, 1.2f, 20.3f, wallTrim);
    // Plocha strecha s presahom
    DrawCube({
        p.x, 8.2f, p.z
    }, 29.0f, 0.4f, 21.0f, roof);
    DrawCubeWires({
        p.x, 8.2f, p.z
    }, 29.0f, 0.4f, 21.0f, (Color){
        120, 120, 130, 255
    });
    // Dve garazove brany otocene k hlavnej ceste (+X)
    float frontX = p.x + 14.1f;
    for (int i = -1; i <= 1; i += 2) {
        DrawCube({
            frontX, 2.2f, p.z + i * 5.0f
        }, 0.2f, 4.4f, 6.0f, door);
        DrawCubeWires({
            frontX, 2.2f, p.z + i * 5.0f
        }, 0.2f, 4.4f, 6.0f, (Color){
            25, 35, 50, 255
        });
        for (int l = 0; l < 5; l++) {
            DrawCube({
                frontX + 0.12f, 0.6f + (float)l * 0.85f, p.z + i * 5.0f
            }, 0.05f, 0.08f, 5.8f, (Color){
                90, 130, 170, 255
            });
        }
    }
    // Reklamna lista nad branami (placeholder namiesto 3D textu)
    DrawCube({
        frontX + 0.1f, 5.7f, p.z
    }, 0.28f, 1.4f, 20.0f, (Color){
        20, 22, 30, 255
    });
    DrawCubeWires({
        frontX + 0.1f, 5.7f, p.z
    }, 0.28f, 1.4f, 20.0f, (Color){
        220, 170, 40, 255
    });
    for (int s = 0; s < 6; s++) {
        DrawCube({
            frontX + 0.28f, 5.7f, p.z - 8.0f + (float)s * 3.2f
        }, 0.06f, 0.75f, 2.2f, (Color){
            240, 190, 40, 255
        });
    }
    // Bocny vchod
    DrawCube({
        p.x, 1.4f, 340.1f
    }, 3.0f, 2.8f, 0.2f, (Color){
        60, 42, 26, 255
    });
    DrawCubeWires({
        p.x, 1.4f, 340.1f
    }, 3.0f, 2.8f, 0.2f, DARKGRAY);
    // Dvor: kopy pneumatik, olejove sudy, rampa
    for (int t = 0; t < 3; t++) {
        DrawCylinderEx({
            -9.0f, 0.15f + (float)t * 0.32f, 322.0f
        },
        { -9.0f, 0.45f + (float)t * 0.32f, 322.0f }, 0.45f, 0.45f, 14, BLACK);
    }
    DrawCylinder({
        -9.0f, 0.0f, 338.0f
    }, 0.4f, 0.4f, 1.1f, 12, (Color){
        20, 120, 20, 255
    });
    DrawCylinder({
        -9.8f, 0.0f, 338.0f
    }, 0.4f, 0.4f, 1.1f, 12, (Color){
        200, 30, 30, 255
    });
    // Jednoducha rampa pred branou
    DrawCube({
        -14.0f, 0.15f, 330.0f
    }, 5.0f, 0.3f, 3.0f, (Color){
        90, 90, 95, 255
    });
    DrawCubeWires({
        -14.0f, 0.15f, 330.0f
    }, 5.0f, 0.3f, 3.0f, DARKGRAY);
    // Pult majstra Ferka + mechanik (tu sa otvara PREMIUM tuning menu)
    DrawCube({
        mechanicCounterPos.x, 0.6f, mechanicCounterPos.z
    }, 4.2f, 1.2f, 1.0f, (Color){
        40, 25, 20, 255
    });
    DrawCube({
        mechanicCounterPos.x, 1.25f, mechanicCounterPos.z
    }, 4.6f, 0.1f, 1.2f, (Color){
        60, 40, 30, 255
    });
    DrawCube({
        mechanicCounterPos.x - 1.7f, 0.75f, mechanicCounterPos.z + 0.45f
    }, 0.5f, 0.35f, 0.2f, (Color){
        200, 140, 40, 255
    });
    // katalog dielov
    DrawShopkeeperJozo({
        mechanicCounterPos.x, 0.0f, mechanicCounterPos.z - 0.9f
    });
}
// RESTAURACIA "U BABETTY" - pri cerpacej stanici, front do priecnej ulice (Z=100).
void DrawRestaurant() {
    Vector3 p = restaurantPos;
    // {-30, 3, 75}
    Color wall = (Color){ 214, 188, 146, 255 };
    Color roof = (Color){ 128, 44, 34, 255 };
    Color wood = (Color){ 92, 58, 34, 255 };
    // Terasa a chodnik pred restauraciou (severna strana, smerom k priecnej ulici)
    DrawCube({
        p.x, 0.045f, p.z + 15.0f
    }, 26.0f, 0.07f, 10.0f, (Color){
        86, 74, 62, 255
    });
    // Budova
    DrawCube(p, 24.0f, 6.0f, 20.0f, wall);
    DrawCubeWires(p, 24.0f, 6.0f, 20.0f, DARKGRAY);
    DrawCube({
        p.x, 0.45f, p.z
    }, 24.3f, 0.9f, 20.3f, (Color){
        150, 125, 95, 255
    });
    // sokel
    // Strecha
    DrawCube({
        p.x, 6.5f, p.z
    }, 25.0f, 0.8f, 21.0f, roof);
    DrawCube({
        p.x, 7.3f, p.z
    }, 20.0f, 0.8f, 16.0f, (Color){
        150, 55, 42, 255
    });
    DrawCubeWires({
        p.x, 6.5f, p.z
    }, 25.0f, 0.8f, 21.0f, DARKBROWN);
    // Okna a dvere na severnej stene (k priecnej ulici)
    float frontZ = p.z + 10.1f;
    for (int i = -1; i <= 1; i++) {
        DrawCube({
            p.x + (float)i * 5.5f, 2.4f, frontZ
        }, 3.4f, 2.6f, 0.18f, (Color){
            150, 200, 230, 200
        });
        DrawCubeWires({
            p.x + (float)i * 5.5f, 2.4f, frontZ
        }, 3.4f, 2.6f, 0.18f, wood);
    }
    DrawCube({
        p.x, 1.5f, frontZ + 0.05f
    }, 2.6f, 3.0f, 0.25f, wood);
    // Tabula restauracie (placeholder napis)
    DrawCube({
        p.x, 5.2f, frontZ
    }, 19.0f, 1.5f, 0.25f, (Color){
        20, 15, 10, 255
    });
    DrawCubeWires({
        p.x, 5.2f, frontZ
    }, 19.0f, 1.5f, 0.25f, (Color){
        240, 200, 60, 255
    });
    for (int i = 0; i < 5; i++) {
        DrawCube({
            p.x - 7.0f + (float)i * 3.5f, 5.2f, frontZ + 0.15f
        }, 2.4f, 0.8f, 0.05f, (Color){
            240, 200, 60, 255
        });
    }
    // Terasove stoly, stolicky a slnecniky
    float tableX[3] = { p.x - 7.0f, p.x, p.x + 7.0f };
    for (int i = 0; i < 3; i++) {
        float tz = p.z + 15.0f;
        DrawCylinder({
            tableX[i], 0.0f, tz
        }, 0.12f, 0.12f, 0.9f, 8, wood);
        DrawCylinder({
            tableX[i], 0.9f, tz
        }, 0.9f, 0.9f, 0.12f, 12, (Color){
            205, 170, 120, 255
        });
        DrawCube({
            tableX[i] - 1.6f, 0.25f, tz
        }, 0.5f, 0.5f, 0.8f, wood);
        DrawCube({
            tableX[i] + 1.6f, 0.25f, tz
        }, 0.5f, 0.5f, 0.8f, wood);
        DrawCylinder({
            tableX[i], 0.0f, tz
        }, 0.06f, 0.06f, 3.2f, 6, (Color){
            120, 120, 125, 255
        });
        DrawCylinder({
            tableX[i], 3.1f, tz
        }, 1.8f, 1.8f, 0.08f, 12, (Color){
            185, 60, 50, 255
        });
    }
    // Pult / vyzdajne okno
    DrawCube({
        restaurantCounterPos.x, 0.65f, restaurantCounterPos.z
    }, 3.6f, 1.3f, 1.0f, wood);
    DrawCube({
        restaurantCounterPos.x, 1.3f, restaurantCounterPos.z
    }, 4.0f, 0.12f, 1.2f, (Color){
        215, 180, 130, 255
    });
    DrawShopkeeperJozo({
        restaurantCounterPos.x, 0.0f, restaurantCounterPos.z - 0.9f
    });
}
// Opravar v oranzovych monterkach - vlastny model, aby sa lisil od Jozu.
void DrawRepairman(Vector3 pos) {
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);
    Color overalls = (Color){ 225, 120, 25, 255 };
    Color overallsDark = (Color){ 175, 85, 15, 255 };
    Color skin = (Color){ 255, 205, 160, 255 };
    Color cap = (Color){ 45, 55, 70, 255 };
    // Telo a nohy
    DrawCube({
        0.0f, 0.95f, 0.0f
    }, 0.52f, 0.72f, 0.32f, overalls);
    DrawCube({
        0.0f, 1.18f, 0.0f
    }, 0.34f, 0.28f, 0.30f, overallsDark);
    DrawCube({
        -0.14f, 0.34f, 0.0f
    }, 0.18f, 0.55f, 0.20f, overalls);
    DrawCube({
        0.14f, 0.34f, 0.0f
    }, 0.18f, 0.55f, 0.20f, overalls);
    DrawCube({
        -0.14f, 0.07f, 0.05f
    }, 0.20f, 0.12f, 0.30f, (Color){
        35, 30, 25, 255
    });
    DrawCube({
        0.14f, 0.07f, 0.05f
    }, 0.20f, 0.12f, 0.30f, (Color){
        35, 30, 25, 255
    });
    // Ruky
    DrawCube({
        -0.33f, 1.05f, 0.08f
    }, 0.14f, 0.50f, 0.14f, overalls);
    DrawCube({
        0.33f, 1.05f, 0.08f
    }, 0.14f, 0.50f, 0.14f, overalls);
    DrawCube({
        -0.33f, 0.76f, 0.10f
    }, 0.12f, 0.12f, 0.12f, skin);
    DrawCube({
        0.33f, 0.76f, 0.10f
    }, 0.12f, 0.12f, 0.12f, skin);
    // Kluc v pravej ruke
    DrawCube({
        0.36f, 0.72f, 0.24f
    }, 0.05f, 0.05f, 0.42f, (Color){
        170, 175, 185, 255
    });
    DrawCube({
        0.36f, 0.72f, 0.46f
    }, 0.14f, 0.10f, 0.07f, (Color){
        120, 125, 135, 255
    });
    // Hlava, tvár, capica
    DrawCube({
        0.0f, 1.60f, 0.0f
    }, 0.30f, 0.30f, 0.30f, skin);
    DrawCube({
        0.0f, 1.57f, 0.16f
    }, 0.16f, 0.06f, 0.04f, (Color){
        70, 40, 25, 255
    });
    DrawCube({
        -0.07f, 1.66f, 0.16f
    }, 0.04f, 0.04f, 0.02f, BLACK);
    DrawCube({
        0.07f, 1.66f, 0.16f
    }, 0.04f, 0.04f, 0.02f, BLACK);
    DrawCube({
        0.0f, 1.78f, 0.02f
    }, 0.34f, 0.09f, 0.34f, cap);
    DrawCube({
        0.0f, 1.76f, 0.20f
    }, 0.32f, 0.04f, 0.18f, cap);
    rlPopMatrix();
}
// OPRAVOVNA BABETT - priemyselna dielna, dve garazove brany, dvor a pult.
void DrawRepairWorkshop() {
    const Vector3 p = repairShopPos;
    // {100, 3, 290}
    Color wall = (Color){ 128, 124, 118, 255 };
    Color wallDark = (Color){ 92, 88, 84, 255 };
    Color roof = (Color){ 52, 55, 60, 255 };
    Color door = (Color){ 46, 72, 96, 255 };
    Color accent = (Color){ 238, 148, 32, 255 };
    // Dvor a vjazd od ulice stvrte (Z = 260)
    DrawCube({
        p.x, 0.045f, p.z - 16.0f
    }, 26.0f, 0.07f, 24.0f, (Color){
        70, 72, 76, 255
    });
    DrawCube({
        p.x, 0.10f, p.z - 16.0f
    }, 26.0f, 0.02f, 0.25f, (Color){
        235, 210, 120, 255
    });
    // Hlavna budova
    DrawCube(p, 24.0f, 6.0f, 20.0f, wall);
    DrawCubeWires(p, 24.0f, 6.0f, 20.0f, DARKGRAY);
    // Tehlovy sokel
    DrawCube({
        p.x, 0.55f, p.z
    }, 24.3f, 1.1f, 20.3f, wallDark);
    // Plocha strecha s presahom
    DrawCube({
        p.x, 6.2f, p.z
    }, 25.0f, 0.4f, 21.0f, roof);
    DrawCubeWires({
        p.x, 6.2f, p.z
    }, 25.0f, 0.4f, 21.0f, (Color){
        120, 120, 130, 255
    });
    // Dve garazove brany na juznej strane (k ulici stvrte)
    float frontZ = p.z - 10.1f;
    for (int i = -1; i <= 1; i += 2) {
        float dx = p.x + (float)i * 5.0f;
        DrawCube({
            dx, 2.2f, frontZ
        }, 7.4f, 4.4f, 0.2f, door);
        DrawCubeWires({
            dx, 2.2f, frontZ
        }, 7.4f, 4.4f, 0.2f, (Color){
            22, 32, 45, 255
        });
        for (int l = 0; l < 5; l++) {
            DrawCube({
                dx, 0.6f + (float)l * 0.85f, frontZ - 0.12f
            }, 7.2f, 0.09f, 0.05f, (Color){
                85, 125, 165, 255
            });
        }
    }
    // Reklamna lista nad branami (namiesto 3D textu) - oranzovy pas s "klucmi"
    DrawCube({
        p.x, 5.55f, frontZ
    }, 20.0f, 1.5f, 0.30f, (Color){
        22, 24, 30, 255
    });
    DrawCubeWires({
        p.x, 5.55f, frontZ
    }, 20.0f, 1.5f, 0.30f, accent);
    for (int s = 0; s < 6; s++) {
        float sx = p.x - 8.2f + (float)s * 3.3f;
        DrawCube({
            sx, 5.55f, frontZ - 0.20f
        }, 2.3f, 0.9f, 0.06f, accent);
        // maly naznak kluca na tabuli
        DrawCube({
            sx, 5.55f, frontZ - 0.24f
        }, 0.35f, 0.35f, 0.05f, (Color){
            250, 250, 250, 255
        });
    }
    // Bocny vchod a okna na vychodnej strane
    DrawCube({
        p.x + 12.1f, 1.45f, p.z
    }, 0.2f, 2.9f, 3.0f, (Color){
        60, 44, 28, 255
    });
    for (int w = -1; w <= 1; w += 2) {
        DrawCube({
            p.x + 12.1f, 3.2f, p.z + (float)w * 6.0f
        }, 0.15f, 1.6f, 3.2f, (Color){
            150, 195, 225, 220
        });
        DrawCubeWires({
            p.x + 12.1f, 3.2f, p.z + (float)w * 6.0f
        }, 0.15f, 1.6f, 3.2f, wallDark);
    }
    // Dvor: kopy pneumatik, olejove sudy, servisny zdvihak
    for (int t = 0; t < 3; t++) {
        DrawCylinderEx({
            p.x + 8.0f, 0.15f + (float)t * 0.32f, p.z - 16.0f
        },
        { p.x + 8.0f, 0.45f + (float)t * 0.32f, p.z - 16.0f }, 0.45f, 0.45f, 14, BLACK);
    }
    DrawCylinder({
        p.x + 8.0f, 0.0f, p.z - 12.0f
    }, 0.4f, 0.4f, 1.1f, 12, (Color){
        190, 145, 25, 255
    });
    DrawCylinder({
        p.x + 8.8f, 0.0f, p.z - 12.0f
    }, 0.4f, 0.4f, 1.1f, 12, (Color){
        40, 40, 45, 255
    });
    // Servisny zdvihak s rozlozenou Babetou (dekoracia pred dielnou)
    float liftX = p.x - 8.0f;
    float liftZ = p.z - 14.0f;
    DrawCube({
        liftX, 0.35f, liftZ
    }, 5.0f, 0.5f, 3.0f, (Color){
        80, 84, 90, 255
    });
    DrawCubeWires({
        liftX, 0.35f, liftZ
    }, 5.0f, 0.5f, 3.0f, DARKGRAY);
    DrawCube({
        liftX, 0.85f, liftZ
    }, 3.4f, 0.10f, 0.10f, (Color){
        200, 40, 35, 255
    });
    DrawCube({
        liftX - 0.8f, 1.05f, liftZ
    }, 0.9f, 0.22f, 0.34f, (Color){
        170, 30, 30, 255
    });
    DrawCube({
        liftX - 1.5f, 1.10f, liftZ
    }, 0.42f, 0.08f, 0.30f, BLACK);
    DrawCylinderEx({
        liftX - 1.4f, 0.65f, liftZ - 0.18f
    },
    { liftX - 1.4f, 0.65f, liftZ + 0.18f }, 0.42f, 0.42f, 14, BLACK);
    DrawCylinderEx({
        liftX + 1.4f, 0.65f, liftZ - 0.18f
    },
    { liftX + 1.4f, 0.65f, liftZ + 0.18f }, 0.42f, 0.42f, 14, BLACK);
    DrawCube({
        liftX + 2.2f, 1.12f, liftZ
    }, 0.04f, 0.04f, 0.55f, (Color){
        210, 210, 220, 255
    });
    // Pult majstra opravara (siroky v X, otoceny k ulici)
    DrawCube({
        repairCounterPos.x, 0.6f, repairCounterPos.z
    }, 4.2f, 1.2f, 1.0f, (Color){
        55, 48, 42, 255
    });
    DrawCube({
        repairCounterPos.x, 1.25f, repairCounterPos.z
    }, 4.6f, 0.1f, 1.2f, (Color){
        95, 88, 80, 255
    });
    DrawCube({
        repairCounterPos.x - 1.6f, 0.8f, repairCounterPos.z + 0.45f
    }, 0.55f, 0.3f, 0.2f, accent);
    // katalog oprav
    DrawRepairman({
        repairCounterPos.x, 0.0f, repairCounterPos.z + 0.9f
    });
    // Zlta zona pred pultom (na strane od ulice)
    DrawCylinder({
        repairCounterPos.x, 0.06f, repairCounterPos.z - 1.6f
    }, 1.5f, 1.5f, 0.03f, 16, (Color){
        235, 180, 40, 255
    });
}
// 2D ikona dielu pre minihru opravovne (kresli sa priamo v obrazovke).
void DrawRepairPartIcon(int idx, Vector2 pos, float s, bool ghost) {
    if (idx == 0 || idx == 1) {
        // koleso
        DrawRing(pos, 23.0f * s, 38.0f * s, 0.0f, 360.0f, 28, ColorAlpha(BLACK, ghost ? 0.35f : 1.0f));
        DrawCircleV(pos, 21.0f * s, ColorAlpha((Color){
            150, 152, 160, 255
        }, ghost ? 0.45f : 1.0f));
        DrawCircleV(pos, 8.0f * s, ColorAlpha((Color){
            205, 205, 215, 255
        }, ghost ? 0.45f : 1.0f));
        float spokeAngle = (idx == 0) ? 25.0f : 65.0f;
        for (int k = 0; k < 6; k++) {
            float a = (spokeAngle + (float)k * 60.0f) * DEG2RAD;
            DrawLineEx(pos, {
                pos.x + cosf(a) * 20.0f * s, pos.y + sinf(a) * 20.0f * s
            }, 2.0f, ColorAlpha(LIGHTGRAY, ghost ? 0.4f : 0.9f));
        }
    }
    else if (idx == 2) {
        // vyfuk
        DrawRectangleRounded({
            pos.x - 55.0f * s, pos.y - 8.0f * s, 85.0f * s, 16.0f * s
        }, 0.7f, 6, ColorAlpha((Color){
            175, 180, 190, 255
        }, ghost ? 0.45f : 1.0f));
        DrawRectangleRounded({
            pos.x + 28.0f * s, pos.y - 14.0f * s, 34.0f * s, 28.0f * s
        }, 0.5f, 6, ColorAlpha((Color){
            120, 125, 135, 255
        }, ghost ? 0.45f : 1.0f));
        DrawRectangle((int)(pos.x - 52.0f * s), (int)(pos.y - 3.0f * s), (int)(80.0f * s), (int)(6.0f * s), ColorAlpha((Color){
            90, 95, 105, 255
        }, ghost ? 0.4f : 1.0f));
    }
    else {
        // sviecka
        DrawRectangleRounded({
            pos.x - 7.0f * s, pos.y - 20.0f * s, 14.0f * s, 30.0f * s
        }, 0.4f, 6, ColorAlpha((Color){
            235, 235, 240, 255
        }, ghost ? 0.45f : 1.0f));
        DrawRectangle((int)(pos.x - 11.0f * s), (int)(pos.y + 10.0f * s), (int)(22.0f * s), (int)(10.0f * s), ColorAlpha((Color){
            140, 145, 155, 255
        }, ghost ? 0.45f : 1.0f));
        DrawRectangle((int)(pos.x - 4.0f * s), (int)(pos.y - 27.0f * s), (int)(8.0f * s), (int)(8.0f * s), ColorAlpha((Color){
            190, 190, 195, 255
        }, ghost ? 0.45f : 1.0f));
    }
}
void DrawGasStation() {
    Vector3 basePos = gasStationPos;
    // {-70.0f, 0.0f, 75.0f}
    // Vyvyseny ostrovcek pod stojanmi
    DrawCube({
        basePos.x, 0.10f, basePos.z
    }, 22.0f, 0.20f, 14.0f, (Color){
        180, 180, 185, 255
    });
    DrawCubeWires({
        basePos.x, 0.10f, basePos.z
    }, 22.0f, 0.20f, 14.0f, GRAY);
    // Velka pristreskova strecha pumpy
    DrawCube({
        basePos.x, 5.2f, basePos.z
    }, 20.0f, 0.6f, 16.0f, (Color){
        230, 230, 235, 255
    });
    DrawCube({
        basePos.x, 5.2f, basePos.z + 8.1f
    }, 20.2f, 0.7f, 0.3f, (Color){
        0, 150, 70, 255
    });
    DrawCube({
        basePos.x, 5.2f, basePos.z - 8.1f
    }, 20.2f, 0.7f, 0.3f, (Color){
        0, 150, 70, 255
    });
    DrawCube({
        basePos.x + 10.1f, 5.2f, basePos.z
    }, 0.3f, 0.7f, 16.2f, (Color){
        0, 150, 70, 255
    });
    DrawCube({
        basePos.x - 10.1f, 5.2f, basePos.z
    }, 0.3f, 0.7f, 16.2f, (Color){
        0, 150, 70, 255
    });
    DrawCube({
        basePos.x, 5.0f, basePos.z + 8.15f
    }, 19.5f, 0.15f, 0.2f, YELLOW);
    // Nosne stlpy strechy
    float pX[4] = { -78.0f, -78.0f, -62.0f, -62.0f };
    float pZ[4] = { 69.0f, 81.0f, 69.0f, 81.0f };
    for (int i = 0; i < 4; i++) {
        DrawCylinder({
            pX[i], 0.2f, pZ[i]
        }, 0.35f, 0.35f, 4.8f, 12, (Color){
            210, 210, 215, 255
        });
    }
    // Stojan 1: MIX 2T
    DrawCube(Vector3Add(gasPump1Pos, {
        0, 1.0f, 0
    }), 0.9f, 1.8f, 1.6f, (Color){
        0, 130, 60, 255
    });
    DrawCube(Vector3Add(gasPump1Pos, {
        0, 1.3f, 0
    }), 0.92f, 0.6f, 1.2f, WHITE);
    DrawCube(Vector3Add(gasPump1Pos, {
        0, 1.35f, 0.61f
    }), 0.6f, 0.35f, 0.05f, BLACK);
    DrawCube(Vector3Add(gasPump1Pos, {
        0, 1.35f, -0.61f
    }), 0.6f, 0.35f, 0.05f, BLACK);
    DrawCylinderEx(Vector3Add(gasPump1Pos, {
        0.46f, 1.4f, 0.5f
    }), Vector3Add(gasPump1Pos, {
        0.46f, 0.4f, 0.2f
    }), 0.05f, 0.05f, 8, BLACK);
    DrawCube(Vector3Add(gasPump1Pos, {
        0.48f, 1.45f, 0.5f
    }), 0.1f, 0.25f, 0.15f, YELLOW);
    // Stojan 2: N95
    DrawCube(Vector3Add(gasPump2Pos, {
        0, 1.0f, 0
    }), 0.9f, 1.8f, 1.6f, (Color){
        0, 130, 60, 255
    });
    DrawCube(Vector3Add(gasPump2Pos, {
        0, 1.3f, 0
    }), 0.92f, 0.6f, 1.2f, WHITE);
    DrawCube(Vector3Add(gasPump2Pos, {
        0, 1.35f, 0.61f
    }), 0.6f, 0.35f, 0.05f, BLACK);
    DrawCube(Vector3Add(gasPump2Pos, {
        0, 1.35f, -0.61f
    }), 0.6f, 0.35f, 0.05f, BLACK);
    DrawCylinderEx(Vector3Add(gasPump2Pos, {
        -0.46f, 1.4f, 0.5f
    }), Vector3Add(gasPump2Pos, {
        -0.46f, 0.4f, 0.2f
    }), 0.05f, 0.05f, 8, BLACK);
    DrawCube(Vector3Add(gasPump2Pos, {
        -0.48f, 1.45f, 0.5f
    }), 0.1f, 0.25f, 0.15f, GREEN);
    // Zlty kruh na zemi pre tankovanie
    DrawCylinder({
        gasPump1Pos.x, 0.21f, gasPump1Pos.z + 2.5f
    }, 2.0f, 2.0f, 0.04f, 16, YELLOW);
    // Predajna Benzinol za stojanmi
    DrawCube({
        -70.0f, 2.5f, 60.0f
    }, 15.0f, 5.0f, 10.0f, (Color){
        240, 240, 245, 255
    });
    DrawCubeWires({
        -70.0f, 2.5f, 60.0f
    }, 15.0f, 5.0f, 10.0f, GRAY);
    DrawCube({
        -70.0f, 5.1f, 60.0f
    }, 15.6f, 0.4f, 10.6f, (Color){
        0, 140, 65, 255
    });
    DrawCube({
        -70.0f, 2.0f, 65.1f
    }, 3.0f, 3.5f, 0.1f, SKYBLUE);
    DrawCube({
        -75.0f, 2.2f, 65.1f
    }, 4.0f, 2.5f, 0.1f, SKYBLUE);
    DrawCube({
        -65.0f, 2.2f, 65.1f
    }, 4.0f, 2.5f, 0.1f, SKYBLUE);
    // Reklamny totem s cenami pri vjazde z cesty
    DrawCube({
        -53.0f, 3.2f, 88.0f
    }, 1.2f, 6.4f, 1.2f, (Color){
        0, 140, 65, 255
    });
    DrawCube({
        -53.0f, 5.8f, 88.0f
    }, 1.8f, 1.4f, 1.4f, YELLOW);
    DrawCube({
        -53.0f, 3.6f, 88.0f
    }, 1.6f, 2.4f, 1.3f, BLACK);
    DrawCube({
        -53.0f, 4.2f, 88.0f
    }, 1.4f, 0.3f, 1.35f, RED);
    DrawCube({
        -53.0f, 3.4f, 88.0f
    }, 1.4f, 0.3f, 1.35f, GREEN);
}
void DrawTree(Vector3 pos) {
    DrawCylinder(pos, 0.35f, 0.45f, 2.8f, 6, DARKBROWN);
    DrawSphere({
        pos.x, pos.y + 3.5f, pos.z
    }, 2.0f, (Color){
        34, 139, 34, 255
    });
    DrawSphere({
        pos.x, pos.y + 4.6f, pos.z
    }, 1.4f, (Color){
        46, 160, 46, 255
    });
}
void DrawPlayerModel(Vector3 pos, float rotDeg, bool isSitting, float walkAnim) {
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);
    rlRotatef(rotDeg, 0, 1, 0);
    Color jacketColor = (Color){ 25, 65, 130, 255 };
    Color jeansColor = (Color){ 35, 45, 75, 255 };
    Color skinColor = (Color){ 255, 205, 160, 255 };
    Color helmetColor = (Color){ 220, 30, 30, 255 };
    if (isSitting) {
        DrawCube({
            -0.16f, 0.42f, -0.05f
        }, 0.14f, 0.45f, 0.16f, jeansColor);
        DrawCube({
            0.16f, 0.42f, -0.05f
        }, 0.14f, 0.45f, 0.16f, jeansColor);
        DrawCube({
            -0.18f, 0.24f, 0.05f
        }, 0.12f, 0.35f, 0.12f, jeansColor);
        DrawCube({
            0.18f, 0.24f, 0.05f
        }, 0.12f, 0.35f, 0.12f, jeansColor);
        DrawCube({
            -0.18f, 0.10f, 0.12f
        }, 0.13f, 0.10f, 0.24f, BLACK);
        DrawCube({
            0.18f, 0.10f, 0.12f
        }, 0.13f, 0.10f, 0.24f, BLACK);
        DrawCube({
            0.0f, 0.95f, -0.12f
        }, 0.38f, 0.50f, 0.25f, jacketColor);
        DrawCube({
            -0.22f, 0.92f, 0.15f
        }, 0.10f, 0.10f, 0.45f, jacketColor);
        DrawCube({
            0.22f, 0.92f, 0.15f
        }, 0.10f, 0.10f, 0.45f, jacketColor);
        DrawCube({
            -0.28f, 0.95f, 0.50f
        }, 0.08f, 0.08f, 0.10f, skinColor);
        DrawCube({
            0.28f, 0.95f, 0.50f
        }, 0.08f, 0.08f, 0.10f, skinColor);
        DrawCube({
            0.0f, 1.30f, -0.08f
        }, 0.22f, 0.22f, 0.22f, skinColor);
        DrawCube({
            0.0f, 1.38f, -0.08f
        }, 0.28f, 0.18f, 0.28f, helmetColor);
        DrawCube({
            0.0f, 1.34f, 0.05f
        }, 0.24f, 0.10f, 0.08f, (Color){
            30, 30, 40, 255
        });
    }
    else {
        float legSwing = sinf(walkAnim * 10.0f) * 0.18f;
        DrawCube({
            -0.12f, 0.45f, legSwing
        }, 0.14f, 0.80f, 0.15f, jeansColor);
        DrawCube({
            0.12f, 0.45f, -legSwing
        }, 0.14f, 0.80f, 0.15f, jeansColor);
        DrawCube({
            -0.12f, 0.08f, legSwing + 0.04f
        }, 0.14f, 0.14f, 0.25f, BLACK);
        DrawCube({
            0.12f, 0.08f, -legSwing + 0.04f
        }, 0.14f, 0.14f, 0.25f, BLACK);
        DrawCube({
            0.0f, 1.25f, 0.0f
        }, 0.42f, 0.70f, 0.26f, jacketColor);
        DrawCube({
            -0.26f, 1.15f, -legSwing * 0.8f
        }, 0.12f, 0.60f, 0.12f, jacketColor);
        DrawCube({
            0.26f, 1.15f, legSwing * 0.8f
        }, 0.12f, 0.60f, 0.12f, jacketColor);
        DrawCube({
            -0.26f, 0.80f, -legSwing * 0.8f
        }, 0.09f, 0.12f, 0.09f, skinColor);
        DrawCube({
            0.26f, 0.80f, legSwing * 0.8f
        }, 0.09f, 0.12f, 0.09f, skinColor);
        DrawCube({
            0.0f, 1.72f, 0.0f
        }, 0.24f, 0.24f, 0.24f, skinColor);
        DrawCube({
            0.0f, 1.82f, 0.0f
        }, 0.28f, 0.12f, 0.28f, (Color){
            180, 20, 20, 255
        });
        DrawCube({
            0.0f, 1.80f, 0.16f
        }, 0.26f, 0.04f, 0.16f, (Color){
            180, 20, 20, 255
        });
    }
    rlPopMatrix();
}
void DrawFirstPersonArms(bool isHoldingThrottle, bool isBraking) {
    Color sleeveColor = (Color){ 25, 65, 130, 255 };
    Color skinColor = (Color){ 255, 205, 160, 255 };
    float throttleTwist = isHoldingThrottle ? -0.05f : 0.0f;
    float brakePull = isBraking ? -0.04f : 0.0f;
    DrawCube({
        -0.26f, 0.95f + brakePull, 0.52f
    }, 0.08f, 0.08f, 0.12f, skinColor);
    DrawCylinderEx({
        -0.22f, 0.65f, 0.05f
    },
    { -0.26f, 0.95f, 0.50f }, 0.07f, 0.06f, 8, sleeveColor);
    DrawCube({
        0.26f, 0.95f + throttleTwist, 0.52f
    }, 0.08f, 0.08f, 0.12f, skinColor);
    DrawCylinderEx({
        0.22f, 0.65f, 0.05f
    },
    { 0.26f, 0.95f + throttleTwist, 0.50f }, 0.07f, 0.06f, 8, sleeveColor);
}
void DrawDetailedBabetta(Vector3 pos, float rotDeg, float rollDeg, const std::vector<Part>& parts, bool isTuned, bool isSportCarb, bool isSportEx, bool isNitro, bool headlightOn, bool drawPlayer, bool isFirstPerson, bool throttleDown, bool brakeDown, Color frameOverride = {
    0, 0, 0, 0
}) {
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);
    rlRotatef(rotDeg, 0, 1, 0);
    rlRotatef(rollDeg, 0, 0, 1);
    Color frameColor = (frameOverride.a != 0) ? frameOverride : (modHasCustomColor ? modBabettaColor : (Color){
        204, 34, 34, 255
    });
    Color chromeColor = (Color){ 225, 225, 235, 255 };
    Color darkMetal = (Color){ 45, 45, 50, 255 };
    // 1. RAM
    DrawCube({
        0.0f, 0.42f, 0.0f
    }, 0.10f, 0.10f, 1.15f, frameColor);
    DrawCube({
        0.0f, 0.68f, 0.36f
    }, 0.10f, 0.55f, 0.10f, frameColor);
    DrawCube({
        0.0f, 0.62f, -0.22f
    }, 0.10f, 0.45f, 0.10f, frameColor);
    // 2. PALIVOVA NADRZ
    DrawCube({
        0.0f, 0.64f, 0.10f
    }, 0.24f, 0.22f, 0.46f, frameColor);
    DrawCubeWires({
        0.0f, 0.64f, 0.10f
    }, 0.24f, 0.22f, 0.46f, MAROON);
    DrawCylinder({
        0.0f, 0.76f, 0.16f
    }, 0.05f, 0.05f, 0.05f, 8, chromeColor);
    // 3. SEDADLO
    DrawCube({
        0.0f, 0.86f, -0.22f
    }, 0.28f, 0.10f, 0.40f, BLACK);
    DrawCube({
        0.0f, 0.88f, -0.36f
    }, 0.26f, 0.12f, 0.14f, (Color){
        30, 30, 30, 255
    });
    // 4. VIDLICA A RIADIDLA
    DrawCube({
        -0.08f, 0.52f, 0.66f
    }, 0.04f, 0.65f, 0.04f, chromeColor);
    DrawCube({
        0.08f, 0.52f, 0.66f
    }, 0.04f, 0.65f, 0.04f, chromeColor);
    DrawCube({
        0.0f, 0.96f, 0.54f
    }, 0.66f, 0.04f, 0.04f, chromeColor);
    DrawCube({
        -0.31f, 0.96f, 0.54f
    }, 0.08f, 0.06f, 0.06f, BLACK);
    DrawCube({
        0.31f, 0.96f, 0.54f
    }, 0.08f, 0.06f, 0.06f, BLACK);
    DrawCube({
        -0.22f, 0.94f, 0.57f
    }, 0.08f, 0.02f, 0.02f, chromeColor);
    DrawCube({
        0.22f, 0.94f, 0.57f
    }, 0.08f, 0.02f, 0.02f, chromeColor);
    DrawCylinder({
        0.0f, 0.99f, 0.50f
    }, 0.06f, 0.06f, 0.04f, 10, chromeColor);
    DrawCylinder({
        0.0f, 1.01f, 0.50f
    }, 0.05f, 0.05f, 0.01f, 10, WHITE);
    // 5. SVETLA
    DrawCylinder({
        0.0f, 0.80f, 0.68f
    }, 0.09f, 0.09f, 0.12f, 12, darkMetal);
    DrawSphere({
        0.0f, 0.80f, 0.75f
    }, 0.08f, headlightOn ? (Color){
        255, 255, 200, 255
    }
    : GRAY);
    DrawCube({
        0.0f, 0.56f, -0.84f
    }, 0.08f, 0.08f, 0.06f, RED);
    if (headlightOn) {
        DrawCylinderEx({
            0.0f, 0.80f, 0.75f
        },
        { 0.0f, 0.40f, 12.0f }, 0.1f, 3.5f, 12, ColorAlpha(YELLOW, 0.15f));
    }
    // 6. BLATNIKY
    DrawCube({
        0.0f, 0.50f, 0.66f
    }, 0.16f, 0.04f, 0.42f, frameColor);
    DrawCube({
        0.0f, 0.48f, -0.66f
    }, 0.16f, 0.04f, 0.46f, frameColor);
    // 7. KOLESA
    if (parts.size() > 1 && parts[1].installed) {
        DrawCylinderEx({
            -0.05f, 0.35f, 0.68f
        },
        { 0.05f, 0.35f, 0.68f }, 0.35f, 0.35f, 16, (Color){
            22, 22, 22, 255
        });
        DrawCylinderEx({
            -0.06f, 0.35f, 0.68f
        },
        { 0.06f, 0.35f, 0.68f }, 0.22f, 0.22f, 12, chromeColor);
    }
    if (parts.size() > 2 && parts[2].installed) {
        DrawCylinderEx({
            -0.05f, 0.35f, -0.68f
        },
        { 0.05f, 0.35f, -0.68f }, 0.35f, 0.35f, 16, (Color){
            22, 22, 22, 255
        });
        DrawCylinderEx({
            -0.06f, 0.35f, -0.68f
        },
        { 0.06f, 0.35f, -0.68f }, 0.22f, 0.22f, 12, chromeColor);
    }
    // 8. MOTOR, VYFUK A PEDALE
    if (parts.size() > 0 && parts[0].installed) {
        Color engColor = isTuned ? (Color){
            220, 220, 230, 255
        }
        : (Color){ 100, 100, 105, 255 };
        Color headColor = isTuned ? GOLD : (Color){ 80, 80, 85, 255 };
        DrawCube({
            0.0f, 0.30f, 0.02f
        }, 0.24f, 0.22f, 0.30f, engColor);
        DrawCylinder({
            0.0f, 0.40f, 0.16f
        }, 0.10f, 0.10f, 0.18f, 10, engColor);
        DrawCube({
            0.0f, 0.50f, 0.20f
        }, 0.20f, 0.08f, 0.20f, headColor);
        Color carbColor = isSportCarb ? (Color){
            220, 30, 30, 255
        }
        : (Color){ 70, 70, 75, 255 };
        DrawCube({
            -0.12f, 0.34f, -0.06f
        }, 0.10f, 0.12f, 0.10f, carbColor);
        if (isSportEx) {
            DrawCylinderEx({
                0.10f, 0.28f, 0.14f
            },
            { 0.14f, 0.18f, -0.05f }, 0.04f, 0.04f, 8, GOLD);
            DrawCylinderEx({
                0.14f, 0.18f, -0.05f
            },
            { 0.16f, 0.18f, -0.45f }, 0.07f, 0.07f, 8, GOLD);
            DrawCylinderEx({
                0.16f, 0.18f, -0.45f
            },
            { 0.16f, 0.22f, -0.85f }, 0.04f, 0.04f, 8, (Color){
                50, 50, 200, 255
            });
        }
        else {
            DrawCylinderEx({
                0.10f, 0.28f, 0.14f
            },
            { 0.14f, 0.20f, -0.15f }, 0.035f, 0.035f, 8, chromeColor);
            DrawCylinderEx({
                0.14f, 0.20f, -0.15f
            },
            { 0.15f, 0.18f, -0.75f }, 0.045f, 0.045f, 8, chromeColor);
            DrawCylinderEx({
                0.15f, 0.18f, -0.75f
            },
            { 0.15f, 0.18f, -0.85f }, 0.03f, 0.03f, 8, darkMetal);
        }
        if (isNitro) {
            Vector3 flameStart = { 0.16f, 0.20f, -0.88f };
            Vector3 flameEnd = { 0.16f, 0.20f, -1.35f };
            DrawCylinderEx(flameStart, flameEnd, 0.05f, 0.01f, 8, SKYBLUE);
            DrawSphere(flameEnd, 0.06f, BLUE);
        }
        DrawCube({
            0.0f, 0.22f, -0.06f
        }, 0.38f, 0.04f, 0.04f, chromeColor);
        DrawCube({
            -0.19f, 0.18f, -0.06f
        }, 0.03f, 0.12f, 0.03f, darkMetal);
        DrawCube({
            0.19f, 0.26f, -0.06f
        }, 0.03f, 0.12f, 0.03f, darkMetal);
        DrawCube({
            -0.23f, 0.12f, -0.06f
        }, 0.08f, 0.03f, 0.10f, BLACK);
        DrawCube({
            0.23f, 0.32f, -0.06f
        }, 0.08f, 0.03f, 0.10f, BLACK);
    }
    if (drawPlayer && isFirstPerson) {
        DrawFirstPersonArms(throttleDown, brakeDown);
    }
    rlPopMatrix();
    if (drawPlayer && !isFirstPerson) {
        DrawPlayerModel(pos, rotDeg, true, 0.0f);
    }
}
// Vystavna Babetta v showroome dealera - kompletne zlozena, bez hraca.
void DrawShowroomBabetta(Vector3 pos, float rotDeg, Color frameColor) {
    static const std::vector<Part> showroomParts = {
        {
            { 0.0f, 0.5f, 0.0f },
            { 0.0f, 0.5f, 0.0f }, true, GRAY, false
        },
        {
            { 0.0f, 0.5f, 0.0f },
            { 0.0f, 0.4f, 0.7f }, true, BLACK, true
        },
        {
            { 0.0f, 0.5f, 0.0f },
            { 0.0f, 0.4f, -0.7f }, true, BLACK, true
        }
    };
    DrawDetailedBabetta(pos, rotDeg, 0.0f, showroomParts, false, false, false, false,                        false, false, false, false, false, frameColor);
}
void DrawRadio(Vector3 pos, bool isOn) {
    Color body = (Color){ 35, 42, 52, 255 };
    Color trim = (Color){ 120, 130, 140, 255 };
    Color speaker = (Color){ 18, 20, 24, 255 };
    // Retro prenosne radio: skrinka, predny panel, dva reproduktory, tlacidla a antena.
    DrawCube(pos, 0.95f, 0.55f, 0.38f, body);
    DrawCubeWires(pos, 0.95f, 0.55f, 0.38f, BLACK);
    DrawCube({
        pos.x, pos.y + 0.01f, pos.z + 0.205f
    }, 0.82f, 0.38f, 0.025f, trim);
    DrawCylinderEx({
        pos.x - 0.27f, pos.y, pos.z + 0.225f
    },
    { pos.x - 0.27f, pos.y, pos.z + 0.255f }, 0.13f, 0.13f, 12, speaker);
    DrawCylinderEx({
        pos.x + 0.27f, pos.y, pos.z + 0.225f
    },
    { pos.x + 0.27f, pos.y, pos.z + 0.255f }, 0.13f, 0.13f, 12, speaker);
    DrawCube({
        pos.x, pos.y + 0.14f, pos.z + 0.225f
    }, 0.16f, 0.10f, 0.03f, isOn ? (Color){
        40, 180, 70, 255
    }
    : MAROON);
    DrawSphere({
        pos.x, pos.y + 0.14f, pos.z + 0.245f
    }, 0.025f, isOn ? LIME : RED);
    DrawCylinder({
        pos.x - 0.12f, pos.y + 0.33f, pos.z
    }, 0.04f, 0.04f, 0.10f, 10, trim);
    DrawCylinder({
        pos.x + 0.12f, pos.y + 0.33f, pos.z
    }, 0.04f, 0.04f, 0.10f, 10, trim);
    DrawCylinderEx({
        pos.x - 0.35f, pos.y + 0.30f, pos.z
    },
    { pos.x - 0.55f, pos.y + 0.95f, pos.z }, 0.018f, 0.012f, 8, trim);
    DrawCube({
        pos.x, pos.y + 0.36f, pos.z
    }, 0.62f, 0.06f, 0.06f, trim);
}
// --- PSENICA: optimalizovane vykreslovanie ---
// Pole ma 270 stebiel. Kreslit kazde steblo zvlast (DrawCylinder + DrawSphere)
// znamenalo ~540 uploadov geometrie do GPU kazdy frame, co zhadzovalo FPS na ~30.
// Preto sa cele pole raz zbali do jedneho neindexovaneho meshu (triangle soup)
// a kresli sa jedinym draw callom.
// POZOR: GenMeshCylinder/GenMeshSphere v raylibe 5.5 vracaju NEindexovane meshe
// (mesh.indices == NULL), preto sa indexy vobec nepouzivaju.
static void AppendMeshPart(std::vector<float>& positions, std::vector<unsigned char>& colors,                           const Mesh& part, Matrix transform, Color color){
    int triangles = part.triangleCount;
    bool indexed = (part.indices != NULL);
    for (int t = 0; t < triangles * 3; t++) {
        int vi = indexed ? (int)part.indices[t] : t;
        Vector3 p = Vector3Transform({
            part.vertices[vi * 3], part.vertices[vi * 3 + 1], part.vertices[vi * 3 + 2]
        }, transform);
        positions.push_back(p.x);
        positions.push_back(p.y);
        positions.push_back(p.z);
        colors.push_back(color.r);
        colors.push_back(color.g);
        colors.push_back(color.b);
        colors.push_back(color.a);
    }
}
static Mesh BuildWheatFieldMesh() {
    Mesh stem = GenMeshCylinder(0.08f, 1.15f, 5);
    Mesh head = GenMeshSphere(0.12f, 8, 8);
    std::vector<float> positions;
    std::vector<unsigned char> colors;
    Color stemColor = GOLD;
    Color headColor = (Color){ 240, 195, 45, 255 };
    for (int x = 52; x <= 158; x += 6) {
        for (int z = 446; z <= 534; z += 6) {
            AppendMeshPart(positions, colors, stem,                           MatrixTranslate((float)x, 0.575f, (float)z), stemColor);
            AppendMeshPart(positions, colors, head,                           MatrixTranslate((float)x, 1.12f, (float)z), headColor);
        }
    }
    UnloadMesh(stem);
    UnloadMesh(head);
    Mesh mesh = { 0 };
    mesh.vertexCount = (int)(positions.size() / 3);
    mesh.triangleCount = mesh.vertexCount / 3;
    // triangle soup, ziadne indexy
    mesh.vertices = (float*)MemAlloc(positions.size() * sizeof(float));
    mesh.colors = (unsigned char*)MemAlloc(colors.size() * sizeof(unsigned char));
    memcpy(mesh.vertices, positions.data(), positions.size() * sizeof(float));
    memcpy(mesh.colors, colors.data(), colors.size() * sizeof(unsigned char));
    UploadMesh(&mesh, false);
    return mesh;
}
void DrawFarm() {
    // Policko je vedla cesty, nie na nej. Cesta je na Z=430, farma zacina za Z=440.
    DrawCube({
        105.0f, 0.015f, 490.0f
    }, 125.0f, 0.03f, 100.0f, (Color){
        142, 112, 35, 255
    });
    if (farmWheatReady) {
        // Mesh sa postavi len raz a potom sa celych 270 stebiel kresli jedinym draw callom.
        static Mesh wheatFieldMesh = { 0 };
        static Material wheatFieldMaterial = { 0 };
        static bool wheatFieldBuilt = false;
        if (!wheatFieldBuilt) {
            wheatFieldMesh = BuildWheatFieldMesh();
            wheatFieldMaterial = LoadMaterialDefault();
            wheatFieldBuilt = true;
        }
        DrawMesh(wheatFieldMesh, wheatFieldMaterial, MatrixIdentity());
    }
    // Jednoducha stodola a silaz, aby bolo jasne, kde zacina praca.
    DrawCube({
        150.0f, 2.5f, 520.0f
    }, 18.0f, 5.0f, 14.0f, (Color){
        155, 50, 35, 255
    });
    DrawCube({
        150.0f, 5.4f, 520.0f
    }, 20.0f, 1.0f, 16.0f, DARKBROWN);
    DrawCylinder({
        137.0f, 0.0f, 520.0f
    }, 3.0f, 3.0f, 8.0f, 16, LIGHTGRAY);
    DrawSphere({
        137.0f, 8.0f, 520.0f
    }, 3.0f, LIGHTGRAY);
    // Zlta zona pre nalozenie 20 kusov PSENICA.
    if (farmWheatReady) {
        DrawCylinder({
            farmWheatPickupPos.x, 0.06f, farmWheatPickupPos.z
        }, 2.2f, 2.2f, 0.06f, 16, YELLOW);
        for (int i = 0; i < 5; ++i) {
            DrawCube({
                farmWheatPickupPos.x - 1.0f + (float)(i % 3), 0.35f + (float)(i / 3) * 0.55f, farmWheatPickupPos.z
            }, 0.85f, 0.5f, 0.55f, (Color){
                220, 180, 65, 255
            });
        }
    }
    // Predajny stanok pri ceste.
    DrawCube({
        farmSellingStandPos.x, 1.25f, farmSellingStandPos.z
    }, 5.5f, 2.5f, 3.0f, (Color){
        125, 72, 30, 255
    });
    DrawCube({
        farmSellingStandPos.x, 2.7f, farmSellingStandPos.z
    }, 6.2f, 0.35f, 3.7f, RED);
    DrawCube({
        farmSellingStandPos.x, 1.4f, farmSellingStandPos.z + 1.7f
    }, 5.8f, 0.22f, 0.45f, DARKBROWN);
    DrawCylinder({
        farmSellingStandPos.x, 0.06f, farmSellingStandPos.z + 3.0f
    }, 2.0f, 2.0f, 0.06f, 16, GREEN);
}
// Pizzar za prednym pultom - biely kabat, cervena satka a kucharska capica.
void DrawPizzaChef(Vector3 pos) {
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);
    Color coat = (Color){ 238, 238, 232, 255 };
    Color scarf = (Color){ 190, 30, 30, 255 };
    Color skin = (Color){ 255, 205, 160, 255 };
    Color hat = (Color){ 245, 245, 240, 255 };
    // Telo, nohy, ruky
    DrawCube({
        0.0f, 0.95f, 0.0f
    }, 0.52f, 0.72f, 0.32f, coat);
    DrawCube({
        0.0f, 1.18f, 0.0f
    }, 0.30f, 0.26f, 0.30f, scarf);
    DrawCube({
        -0.14f, 0.34f, 0.0f
    }, 0.18f, 0.55f, 0.20f, (Color){
        40, 40, 48, 255
    });
    DrawCube({
        0.14f, 0.34f, 0.0f
    }, 0.18f, 0.55f, 0.20f, (Color){
        40, 40, 48, 255
    });
    DrawCube({
        -0.14f, 0.07f, 0.05f
    }, 0.20f, 0.12f, 0.30f, (Color){
        30, 25, 20, 255
    });
    DrawCube({
        0.14f, 0.07f, 0.05f
    }, 0.20f, 0.12f, 0.30f, (Color){
        30, 25, 20, 255
    });
    DrawCube({
        -0.33f, 1.05f, 0.08f
    }, 0.14f, 0.50f, 0.14f, coat);
    DrawCube({
        0.33f, 1.05f, 0.08f
    }, 0.14f, 0.50f, 0.14f, coat);
    // Pizza v ruke (tanier + cervena/zlta pizza)
    DrawCylinder({
        0.34f, 0.86f, 0.22f
    }, 0.30f, 0.30f, 0.04f, 16, (Color){
        230, 228, 220, 255
    });
    DrawCylinder({
        0.34f, 0.90f, 0.22f
    }, 0.26f, 0.26f, 0.05f, 16, (Color){
        225, 175, 70, 255
    });
    DrawCylinder({
        0.34f, 0.94f, 0.22f
    }, 0.18f, 0.18f, 0.02f, 16, (Color){
        200, 50, 40, 255
    });
    // Hlava, fuz, capica
    DrawCube({
        0.0f, 1.60f, 0.0f
    }, 0.30f, 0.30f, 0.30f, skin);
    DrawCube({
        0.0f, 1.52f, 0.17f
    }, 0.20f, 0.07f, 0.04f, (Color){
        70, 45, 30, 255
    });
    DrawCube({
        -0.07f, 1.66f, 0.16f
    }, 0.04f, 0.04f, 0.02f, BLACK);
    DrawCube({
        0.07f, 1.66f, 0.16f
    }, 0.04f, 0.04f, 0.02f, BLACK);
    DrawCube({
        0.0f, 1.80f, 0.0f
    }, 0.34f, 0.14f, 0.34f, hat);
    DrawCylinder({
        0.0f, 1.90f, 0.0f
    }, 0.18f, 0.15f, 0.18f, 12, hat);
    DrawCube({
        0.0f, 1.88f, 0.0f
    }, 0.35f, 0.03f, 0.35f, scarf);
    rlPopMatrix();
}
// PIZZERIA LUCIA - predna cast je predaj pizze, zadna cast je job rozvozu.
void DrawPizzeria() {
    const Vector3 p = pizzeriaPos;
    // {90, 3, 125}
    Color wall = (Color){ 236, 220, 196, 255 };
    // kremova omietka
    Color wallDark = (Color){ 196, 168, 138, 255 };
    Color roof = (Color){ 150, 40, 34, 255 };
    // cervena strecha
    Color wood = (Color){ 96, 62, 38, 255 };
    Color sign = (Color){ 30, 22, 18, 255 };
    Color accent = (Color){ 214, 42, 34, 255 };
    Color cream = (Color){ 245, 238, 220, 255 };
    // Prizemie + poschodie
    DrawCube(p, 18.0f, 6.0f, 16.0f, wall);
    DrawCubeWires(p, 18.0f, 6.0f, 16.0f, wallDark);
    DrawCube({
        p.x, 0.5f, p.z
    }, 18.3f, 1.0f, 16.3f, wallDark);
    // Strecha s presahom
    DrawCube({
        p.x, 6.2f, p.z
    }, 19.4f, 0.5f, 17.4f, roof);
    DrawCubeWires({
        p.x, 6.2f, p.z
    }, 19.4f, 0.5f, 17.4f, (Color){
        90, 22, 18, 255
    });
    // Komin piecky (zadna cast - dovoz)
    DrawCube({
        p.x + 5.5f, 7.4f, p.z + 4.0f
    }, 1.4f, 2.6f, 1.4f, (Color){
        120, 74, 60, 255
    });
    DrawCylinder({
        p.x + 5.5f, 8.7f, p.z + 4.0f
    }, 1.0f, 1.0f, 0.3f, 12, (Color){
        70, 55, 50, 255
    });
    // --- PREDNA CAST (juh, k ulici Z=100) ---
    float frontZ = p.z - 8.1f;
    // Velke vykladne okno a dvere
    DrawCube({
        p.x - 3.6f, 2.3f, frontZ
    }, 6.0f, 3.6f, 0.2f, (Color){
        110, 150, 170, 235
    });
    DrawCubeWires({
        p.x - 3.6f, 2.3f, frontZ
    }, 6.0f, 3.6f, 0.2f, wood);
    DrawCube({
        p.x + 3.6f, 1.85f, frontZ
    }, 3.4f, 3.0f, 0.2f, wood);
    DrawCube({
        p.x + 4.9f, 1.95f, frontZ - 0.12f
    }, 0.12f, 0.12f, 0.06f, GOLD);
    // Striebor cervena markiza nad vstupom
    for (int a = 0; a < 9; a++) {
        float ax = p.x - 8.4f + (float)a * 2.1f;
        DrawCube({
            ax, 4.35f, frontZ - 1.1f
        }, 2.0f, 0.16f, 2.2f, (a % 2 == 0) ? accent : cream);
    }
    DrawCube({
        p.x, 4.55f, frontZ - 2.2f
    }, 18.0f, 0.12f, 0.12f, wood);
    // Velka reklamna tabula "Real Svinna Pizzeria Lucia" (bloky, bez 3D fontu)
    DrawCube({
        p.x, 5.35f, frontZ - 0.05f
    }, 14.0f, 1.6f, 0.24f, sign);
    DrawCubeWires({
        p.x, 5.35f, frontZ - 0.05f
    }, 14.0f, 1.6f, 0.24f, (Color){
        240, 200, 90, 255
    });
    // Cerveny pas + zlte bodky ako napis/pizza
    DrawCube({
        p.x, 5.55f, frontZ - 0.20f
    }, 12.6f, 0.55f, 0.06f, accent);
    for (int d = 0; d < 7; d++) {
        DrawCube({
            p.x - 4.8f + (float)d * 1.6f, 5.55f, frontZ - 0.26f
        }, 0.45f, 0.45f, 0.05f, (Color){
            250, 205, 90, 255
        });
    }
    DrawCube({
        p.x, 5.0f, frontZ - 0.20f
    }, 10.5f, 0.20f, 0.06f, cream);
    // Predny pult (predaj pizze) + pizzar
    DrawCube({
        pizzeriaCounterPos.x, 0.62f, pizzeriaCounterPos.z
    }, 5.2f, 1.24f, 1.1f, wood);
    DrawCube({
        pizzeriaCounterPos.x, 1.30f, pizzeriaCounterPos.z
    }, 5.6f, 0.12f, 1.3f, (Color){
        215, 185, 145, 255
    });
    // Sklenena vitrina s pizzami
    DrawCube({
        pizzeriaCounterPos.x - 1.4f, 1.55f, pizzeriaCounterPos.z
    }, 2.2f, 0.5f, 0.8f, (Color){
        150, 195, 215, 190
    });
    for (int pz = 0; pz < 3; pz++) {
        DrawCylinder({
            pizzeriaCounterPos.x - 2.1f + (float)pz * 0.7f, 1.40f, pizzeriaCounterPos.z
        }, 0.30f, 0.30f, 0.06f, 14, (Color){
            225, 170, 70, 255
        });
        DrawCylinder({
            pizzeriaCounterPos.x - 2.1f + (float)pz * 0.7f, 1.44f, pizzeriaCounterPos.z
        }, 0.20f, 0.20f, 0.02f, 14, (Color){
            200, 55, 40, 255
        });
    }
    DrawPizzaChef({
        pizzeriaCounterPos.x, 0.0f, pizzeriaCounterPos.z + 1.0f
    });
    // Zlta interakcna zona pred pultom (strana od cesty)
    DrawCylinder({
        pizzeriaCounterPos.x, 0.06f, pizzeriaCounterPos.z - 1.6f
    }, 1.5f, 1.5f, 0.03f, 16, (Color){
        235, 185, 45, 255
    });
    // --- ZADNA CAST (sever, job rozvozu) ---
    float backZ = p.z + 8.1f;
    // Nakladna rampa a dvere pre rozvoz
    DrawCube({
        p.x, 0.15f, p.z + 10.2f
    }, 12.0f, 0.3f, 4.0f, (Color){
        95, 96, 100, 255
    });
    DrawCube({
        p.x, 1.9f, backZ
    }, 5.0f, 3.2f, 0.2f, (Color){
        70, 78, 88, 255
    });
    DrawCubeWires({
        p.x, 1.9f, backZ
    }, 5.0f, 3.2f, 0.2f, (Color){
        35, 40, 48, 255
    });
    for (int l = 0; l < 5; l++) {
        DrawCube({
            p.x, 0.6f + (float)l * 0.75f, backZ + 0.14f
        }, 4.8f, 0.08f, 0.05f, (Color){
            120, 130, 145, 255
        });
    }
    // Reklamna lista nad zadnymi dverami
    DrawCube({
        p.x, 4.15f, backZ + 0.05f
    }, 9.0f, 1.1f, 0.22f, sign);
    DrawCubeWires({
        p.x, 4.15f, backZ + 0.05f
    }, 9.0f, 1.1f, 0.22f, (Color){
        240, 200, 90, 255
    });
    DrawCube({
        p.x, 4.15f, backZ + 0.20f
    }, 8.0f, 0.35f, 0.05f, accent);
    // Mali svetielka nad rampou
    for (int s = -1; s <= 1; s += 2) {
        DrawCube({
            p.x + (float)s * 4.0f, 3.2f, backZ + 0.3f
        }, 0.7f, 0.35f, 0.3f, (Color){
            250, 215, 120, 255
        });
    }
    // Zadny pult (job rozvozu) + kucharsky pomocnik
    DrawCube({
        pizzaJobPos.x, 0.62f, pizzaJobPos.z
    }, 4.2f, 1.24f, 1.0f, (Color){
        60, 72, 88, 255
    });
    DrawCube({
        pizzaJobPos.x, 1.30f, pizzaJobPos.z
    }, 4.6f, 0.10f, 1.2f, (Color){
        150, 160, 175, 255
    });
    // Prepravky a pizzove krabice pripravene na rozvoz
    for (int b = 0; b < 3; b++) {
        DrawCube({
            pizzaJobPos.x + 1.5f, 0.45f + (float)b * 0.22f, pizzaJobPos.z + 0.15f
        }, 0.9f, 0.2f, 0.7f, (Color){
            205, 170, 120, 255
        });
    }
    DrawRepairman({
        pizzaJobPos.x - 1.4f, 0.0f, pizzaJobPos.z - 0.9f
    });
    // Zlta interakcna zona za pultom
    DrawCylinder({
        pizzaJobPos.x, 0.06f, pizzaJobPos.z + 1.6f
    }, 1.5f, 1.5f, 0.03f, 16, (Color){
        235, 185, 45, 255
    });
    // Male stromy/stolicky vpredu pre hosti
    for (int t = -1; t <= 1; t += 2) {
        float tx = p.x + (float)t * 6.5f;
        float tz = pizzeriaCounterPos.z - 2.0f;
        DrawCylinder({
            tx, 0.4f, tz
        }, 0.15f, 0.15f, 0.8f, 8, DARKGRAY);
        DrawCylinder({
            tx, 0.82f, tz
        }, 1.0f, 1.0f, 0.10f, 12, (Color){
            205, 190, 165, 255
        });
    }
}
void DrawMapEnvironment() {
    DrawPlane({
        0, 0, 0
    },
    { 4000, 4000 }, modNightMode ? (Color){
        15, 30, 15, 255
    }
    : (Color){
        40, 110, 40, 255
    });
    Color roadColor = (Color){ 52, 54, 58, 255 };
    Color sideStreetColor = (Color){ 60, 62, 65, 255 };
    // 1. HLAVNA CESTA
    DrawCube({
        0, 0.05f, 0
    }, 12.0f, 0.08f, 4000.0f, roadColor);
    DrawCube({
        -5.6f, 0.10f, 0
    }, 0.3f, 0.02f, 4000.0f, WHITE);
    DrawCube({
        5.6f, 0.10f, 0
    }, 0.3f, 0.02f, 4000.0f, WHITE);
    for (int z = -1980; z <= 1980; z += 16) {
        if (z >= 85 && z <= 115) continue;
        DrawCube({
            0.0f, 0.10f, (float)z
        }, 0.3f, 0.02f, 8.0f, (Color){
            250, 250, 250, 255
        });
    }
    // 2. KRIZOVATKA A PRIECNA ULICA (Z = 100)
    // Cesta je predlzena na vychod (x = 50 +/- 230 -> od -180 do +280), aby dosla az k Moto Dealerovi.
    DrawCube({
        50.0f, 0.052f, 100.0f
    }, 460.0f, 0.08f, 10.0f, sideStreetColor);
    DrawCube({
        50.0f, 0.10f, 95.2f
    }, 460.0f, 0.02f, 0.25f, WHITE);
    DrawCube({
        50.0f, 0.10f, 104.8f
    }, 460.0f, 0.02f, 0.25f, WHITE);
    for (int x = -170; x <= 270; x += 16) {
        if (x >= -10 && x <= 10) continue;
        DrawCube({
            (float)x, 0.10f, 100.0f
        }, 8.0f, 0.02f, 0.25f, (Color){
            250, 250, 250, 255
        });
    }
    // Priechody pre chodcov (Zebra)
    for (int p = -4; p <= 4; p += 2) {
        DrawCube({
            (float)p, 0.10f, 92.0f
        }, 1.0f, 0.02f, 3.5f, WHITE);
        DrawCube({
            (float)p, 0.10f, 108.0f
        }, 1.0f, 0.02f, 3.5f, WHITE);
    }
    // 3. ULICA K GARAZE
    DrawCube({
        11.0f, 0.046f, 20.0f
    }, 14.0f, 0.07f, 8.0f, (Color){
        65, 65, 65, 255
    });
    // 4. SEVERNA DEDINSKA SPOJKA (Z = 180)
    // Predlzena na vychod, aby sa napojila na novu vychodnu ulicku (X = 180).
    DrawCube({
        60.0f, 0.044f, 180.0f
    }, 380.0f, 0.07f, 8.0f, sideStreetColor);
    DrawCube({
        -120.0f, 0.043f, 140.0f
    }, 8.0f, 0.07f, 80.0f, sideStreetColor);
    DrawCube({
        120.0f, 0.043f, 140.0f
    }, 8.0f, 0.07f, 80.0f, sideStreetColor);
    // 6. NOVA VYCHODNA ULICKA (X = 180) - spaja priecnu ulicu (Z=100), spojku (Z=180) a novu stvrt (Z=260)
    DrawCube({
        180.0f, 0.044f, 210.0f
    }, 10.0f, 0.07f, 220.0f, sideStreetColor);
    DrawCube({
        180.0f, 0.042f, 320.0f
    }, 10.0f, 0.07f, 8.0f, sideStreetColor);
    // 7. NOVA ULICKA NOVEJ STVRTE (Z = 260) - krizuje hlavnu cestu a konci pri vychodnej ulicke
    DrawCube({
        25.0f, 0.044f, 260.0f
    }, 320.0f, 0.07f, 8.0f, sideStreetColor);
    // 5. DLSIA CESTA K FARME (Z = 430), napojena na hlavnu cestu.
    DrawCube({
        80.0f, 0.044f, 430.0f
    }, 170.0f, 0.07f, 9.0f, sideStreetColor);
    DrawCube({
        80.0f, 0.10f, 425.7f
    }, 170.0f, 0.02f, 0.22f, WHITE);
    DrawCube({
        80.0f, 0.10f, 434.3f
    }, 170.0f, 0.02f, 0.22f, WHITE);
    for (int x = 12; x <= 160; x += 16) DrawCube({
        (float)x, 0.10f, 430.0f
    }, 8.0f, 0.02f, 0.22f, WHITE);
    // 5. PARKOVISKA A VJAZDY (mimo hlavnych pruhov)
    // Moto-Shop velke nadvorie (spaja obchod s priecnou cestou)
    DrawCube({
        60.0f, 0.045f, 88.5f
    }, 32.0f, 0.07f, 15.0f, (Color){
        70, 72, 76, 255
    });
    // Cerpacia stanica vjazd / nadvorie (spaja pumpu s priecnou cestou)
    DrawCube({
        -70.0f, 0.045f, 87.0f
    }, 36.0f, 0.07f, 18.0f, (Color){
        70, 72, 76, 255
    });
    // Zlta interakcna zona pred pultom Moto-Shopu
    DrawCylinder({
        shopCounterPos.x, 0.06f, shopCounterPos.z + 1.8f
    }, 1.6f, 1.6f, 0.03f, 16, YELLOW);
    // Parkovisko + vjazd pre Moto Dealer (vedla cesty na vychode Z=100)
    DrawCube({
        dealerPos.x, 0.045f, dealerPos.z + 16.0f
    }, 36.0f, 0.07f, 14.0f, (Color){
        70, 72, 76, 255
    });
    // Spajaci vjazd dealera s priecnou cestou (Z=100 -> dealer Z=91)
    DrawCube({
        dealerPos.x, 0.044f, 93.5f
    }, 12.0f, 0.07f, 7.0f, (Color){
        70, 72, 76, 255
    });
    // Pizzeria Lucia - zadny vjazd a parkovisko (dovoz pizze)
    // Vjazd popri vychodnej strane budovy, napojeny na priecnu ulicu (Z=100).
    DrawCube({
        pizzeriaPos.x + 18.0f, 0.044f, 130.0f
    }, 11.0f, 0.07f, 52.0f, (Color){
        70, 72, 76, 255
    });
    // Parkovisko za pizzou
    DrawCube({
        pizzeriaPos.x + 10.0f, 0.045f, 146.0f
    }, 36.0f, 0.07f, 18.0f, (Color){
        70, 72, 76, 255
    });
    // Biele ciary parkovacich miest (5 miest)
    for (int pb = 0; pb < 5; pb++) {
        float lx = pizzeriaPos.x - 7.0f + (float)pb * 8.0f;
        DrawCube({
            lx, 0.10f, 146.5f
        }, 0.16f, 0.02f, 13.0f, WHITE);
    }
    DrawCube({
        pizzeriaPos.x + 9.0f, 0.10f, 152.6f
    }, 33.0f, 0.02f, 0.16f, WHITE);
    DrawGasStation();
    DrawMotoDealer();
    DrawAutoService();
    DrawRestaurant();
    DrawRepairWorkshop();
    DrawPizzeria();
    DrawFullGarage({
        20.0f, 0.0f, 20.0f
    });
    DrawFarm();
    for (int i = 0; i < NUM_BUILDINGS; i++) {
        DrawBuilding(i, buildingsPos[i], buildingsSize[i], buildingWallColors[i], buildingRoofColors[i]);
        // Vjazdovy chodnik od domu k najblizsej ceste
        float hDir = (float)buildingFrontDir[i];
        float pathZ = buildingsPos[i].z + hDir * (buildingsSize[i].z * 0.5f + 4.5f);
        DrawCube({
            buildingsPos[i].x, 0.044f, pathZ
        }, 5.0f, 0.05f, 9.0f, (Color){
            80, 82, 85, 255
        });
    }
    // Smerove tabule
    DrawCylinder({
        -6.5f, 0.0f, 92.0f
    }, 0.08f, 0.08f, 3.2f, 8, GRAY);
    DrawCube({
        -6.5f, 2.9f, 92.0f
    }, 0.1f, 0.5f, 1.8f, (Color){
        20, 100, 200, 255
    });
    DrawCube({
        -6.5f, 2.3f, 92.0f
    }, 0.1f, 0.5f, 2.2f, (Color){
        200, 100, 20, 255
    });
    for (int z = -400; z <= 400; z += 40) {
        if (z >= 10 && z <= 30) continue;
        if (z >= 80 && z <= 120) continue;
        if (z >= 170 && z <= 190) continue;
        // Zapadna strana pri autoservise - strom by trcal z dvora
        if (!(z >= 310 && z <= 350)) DrawTree({
            -9.5f, 0.0f, (float)z
        });
        DrawTree({
            9.5f, 0.0f, (float)z
        });
    }
    float lampZ[4] = { 20.0f, 90.0f, 110.0f, 180.0f };
    for (int l = 0; l < 4; l++) {
        DrawCylinder({
            -7.0f, 0.0f, lampZ[l]
        }, 0.12f, 0.12f, 5.5f, 8, DARKGRAY);
        DrawCube({
            -6.0f, 5.4f, lampZ[l]
        }, 2.0f, 0.15f, 0.15f, DARKGRAY);
        DrawSphere({
            -5.0f, 5.3f, lampZ[l]
        }, 0.25f, (Color){
            255, 255, 180, 255
        });
    }
}
int main() {
    InitWindow(1280, 720, "My Summer Babetta - Version: 0.4.8DEV  || ModLoader Version: 0.1.8ADEV");
    InitAudioDevice();
    Music ambient = { 0 };
    // Declare notification variables for music loading
    std::string notificationMsg = "";
    float notificationTimer = 0.0f;
    // Načítanie vlastní hudby podľa modu
    if (modEnableCustomMusic) {
        if (fs::exists(modCustomMusicPath)) {
            ambient = LoadMusicStream(modCustomMusicPath.c_str());
            PlayMusicStream(ambient);
            notificationMsg = "Načítaná vlastní hudba";
            notificationTimer = 2.0f;
        }
        else {
            ambient = LoadMusicStream("Sounds/birds_background.mp3");
            PlayMusicStream(ambient);
            notificationMsg = "Hudba neexistuje, použije sa predvolená";
            notificationTimer = 2.0f;
        }
    }
    else {
        if (fs::exists("Sounds/birds_background.mp3")) {
            ambient = LoadMusicStream("Sounds/birds_background.mp3");
            PlayMusicStream(ambient);
        }
    }
    std::vector<Mod> activeMods = LoadAndApplyMods();
    // Aplikovanie modov po nacitani
    if (modUnlockEverything) {
        hasTuningMotor = true;
        hasSportCarb = true;
        hasSportExhaust = true;
        hasTurboKit = true;
        hasSportSuspension = true;
        hasSportWheels = true;
        hasHeadlightKit = true;
        hasAlarmSystem = true;
        hasBigBoreKit = true;
        hasRaceCDI = true;
        hasTitaniumExhaust = true;
        hasRaceClutch = true;
        hasBremboBrakes = true;
        hasOhlinsSuspension = true;
        ownsJawa = true;
    }
    if (modDisableWheatRegrow) {
        farmWheatReady = false;
        // Zakze rast psenice kompletne (aj pri startovani)
    }
    // Initialize job position
    jobPos = jobDeliveryPos;
    bool radioOn = true;
    bool headlightOn = true;
    SetExitKey(0);
    Camera3D camera = {
        { 20.0f, 1.8f, 25.0f },
        { 20.0f, 1.8f, 20.0f },
        { 0.0f, 1.0f, 0.0f }, 75.0f, CAMERA_PERSPECTIVE
    };
    std::vector<Part> parts = {
        {
            { 16.0f, 0.5f, 16.0f },
            { 0.0f, 0.5f, 0.0f }, false, GRAY, false
        },
        {
            { 16.0f, 0.5f, 17.0f },
            { 0.0f, 0.4f, 0.7f }, false, BLACK, true
        },
        {
            { 16.0f, 0.5f, 18.0f },
            { 0.0f, 0.4f, -0.7f }, false, BLACK, true
        }
    };
    float money = 2500.0f + modMoneyBonus;
    float hunger = 0.0f, thirst = 0.0f, fuel = 5.0f;
    int wheatInventory = 0;
    int holdingIdx = -1;
    bool isRiding = false, isPaused = false, showFPS = true;
    bool isShopOpen = false;
    bool isDealerOpen = false;
    bool isMechanicOpen = false;
    bool isRestaurantOpen = false;
    bool isRepairOpen = false;
    bool isPizzeriaOpen = false;
    bool pizzaJobActive = false;
    int pizzaJobTarget = 0;
    int pizzaJobsDone = 0;
    float pizzaJobMsgTimer = 0.0f;
    bool firstPersonMode = true;
    float hornTimer = 0.0f;
    // --- MINIHRA: OPRAVOVNA BABETT ---
    // Hrac pretiahne 4 diely z tray pola na spravne miesto v 2D scheme Babetty.
    const Vector2 repairHome[4] = {
        { 880.0f, 215.0f },
        { 880.0f, 320.0f },
        { 880.0f, 425.0f },
        { 880.0f, 530.0f }
    };
    const Vector2 repairSlot[4] = {
        { 330.0f, 430.0f },
        { 560.0f, 430.0f },
        { 445.0f, 466.0f },
        { 442.0f, 340.0f }
    };
    Vector2 repairPartPos[4] = { repairHome[0], repairHome[1], repairHome[2], repairHome[3] };
    bool repairPlaced[4] = { false, false, false, false };
    int repairDrag = -1;
    int repairDone = 0;
    bool repairFinished = false;
    std::string repairMsg = "";
    float repairMsgTimer = 0.0f;
    Vector3 babettaPos = { 20, 0, 20 };
    // Polica v garazi je na Y=1.50 (horna plocha Y=1.55). Radio je vysoke 0.55,
    // preto je jeho stred na Y=1.825, aby stalo presne na polici.
    Vector3 radioPos = { 13.0f, 1.825f, 20.0f };
    float babettaRot = 0.0f, babettaLean = 0.0f, speed = 0.0f;
    float babettaPosY = 0.0f, babettaVelY = 0.0f;
    float distanceTraveledKM = 0.0f;
    float playerWalkAnim = 0.0f;
    float mouseLookPitch = 0.0f;
    float mouseLookYaw = 0.0f;
    bool isNitroActive = false;
    float radioVolume = 0.0f;
    DisableCursor();
    SetTargetFPS(120);
    while (!WindowShouldClose()) {
        if (ambient.stream.buffer != nullptr) {
            // Plna hlasitost do 10 m, potom plynuly utlm az na nulu vo vzdialenosti 60 m.
            float radioDistance = Vector3Distance(camera.position, radioPos);
            if (radioOn) {
                radioVolume = (radioDistance <= 10.0f) ? 0.85f : 0.85f * std::max(0.0f, 1.0f - (radioDistance - 10.0f) / 50.0f);
            }
            else {
                radioVolume = 0.0f;
            }
            SetMusicVolume(ambient, radioVolume);
            UpdateMusicStream(ambient);
        }
        if (notificationTimer > 0.0f) {
            notificationTimer -= GetFrameTime();
            if (notificationTimer <= 0.0f) notificationMsg = "";
        }
        if (hornTimer > 0.0f) {
            hornTimer -= GetFrameTime();
        }
        if (!farmWheatReady) {
            farmWheatRegrowTimer -= GetFrameTime();
            if (farmWheatRegrowTimer <= 0.0f) farmWheatReady = true;
        }
        if (IsKeyPressed(KEY_V)) {
            firstPersonMode = !firstPersonMode;
            notificationMsg = firstPersonMode ? "Pohlad: 1. OSOBA (First Person)" : "Pohlad: 3. OSOBA (Third Person)";
            notificationTimer = 2.0f;
        }
        if (IsKeyPressed(KEY_L)) {
            headlightOn = !headlightOn;
            notificationMsg = headlightOn ? "Predne svetlo: ZAPNUTE" : "Predne svetlo: VYPNUTE";
            notificationTimer = 1.5f;
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (isShopOpen) {
                isShopOpen = false;
                DisableCursor();
            }
            else if (isDealerOpen) {
                isDealerOpen = false;
                DisableCursor();
            }
            else if (isMechanicOpen) {
                isMechanicOpen = false;
                DisableCursor();
            }
            else if (isRestaurantOpen) {
                isRestaurantOpen = false;
                DisableCursor();
            }
            else if (isRepairOpen) {
                isRepairOpen = false;
                repairDrag = -1;
                DisableCursor();
            }
            else if (isPizzeriaOpen) {
                isPizzeriaOpen = false;
                DisableCursor();
            }
            else {
                isPaused = !isPaused;
                if (isPaused) EnableCursor();
                else DisableCursor();
            }
        }
        if (IsKeyPressed(KEY_F6)) showFPS = !showFPS;
        if (IsKeyPressed(KEY_F9)) money = 999999.0f;
        if (!isPaused && !isShopOpen && !isDealerOpen && !isMechanicOpen && !isRestaurantOpen && !isRepairOpen && !isPizzeriaOpen) {
            hunger += 0.000008f;
            thirst += 0.00003f;
            if (!isRiding) {
                Vector3 oldPos = camera.position;
                Vector2 moveInput = { (float)(IsKeyDown(KEY_W) - IsKeyDown(KEY_S)), (float)(IsKeyDown(KEY_D) - IsKeyDown(KEY_A)) };
                if (moveInput.x != 0 || moveInput.y != 0) {
                    playerWalkAnim += GetFrameTime() * 1.5f;
                }
                UpdateCameraPro(&camera,                     {
                    moveInput.x * 0.15f, moveInput.y * 0.15f, 0
                },
                { GetMouseDelta().x * 0.1f, GetMouseDelta().y * 0.1f, 0 }, 0);
                if (CheckCollisionWithWorld(camera.position, 0.5f)) camera.position = oldPos;
                camera.position.y = 1.8f;
                bool nearShopCounter = CheckCollisionSpheres(camera.position, 3.2f, {
                    shopCounterPos.x, 1.0f, shopCounterPos.z + 1.8f
                }, 1.2f);
                if (nearShopCounter && IsKeyPressed(KEY_F)) {
                    isShopOpen = true;
                    EnableCursor();
                }
                bool nearDealer = CheckCollisionSpheres(camera.position, 4.5f, {
                    dealerCounterPos.x, 1.0f, dealerCounterPos.z + 2.0f
                }, 1.5f);
                if (nearDealer && IsKeyPressed(KEY_F)) {
                    isDealerOpen = true;
                    EnableCursor();
                }
                bool nearMechanic = CheckCollisionSpheres(camera.position, 3.5f, {
                    mechanicCounterPos.x, 1.0f, mechanicCounterPos.z + 1.5f
                }, 1.5f);
                if (nearMechanic && IsKeyPressed(KEY_F)) {
                    isMechanicOpen = true;
                    EnableCursor();
                }
                bool nearRestaurant = CheckCollisionSpheres(camera.position, 3.5f, {
                    restaurantCounterPos.x, 1.0f, restaurantCounterPos.z + 1.5f
                }, 1.5f);
                if (nearRestaurant && IsKeyPressed(KEY_F)) {
                    isRestaurantOpen = true;
                    EnableCursor();
                }
                bool nearRepair = CheckCollisionSpheres(camera.position, 3.5f, {
                    repairCounterPos.x, 1.0f, repairCounterPos.z - 1.5f
                }, 1.5f);
                if (nearRepair && IsKeyPressed(KEY_F)) {
                    isRepairOpen = true;
                    EnableCursor();
                }
                bool nearPizzeriaCounter = CheckCollisionSpheres(camera.position, 3.5f, {
                    pizzeriaCounterPos.x, 1.0f, pizzeriaCounterPos.z - 1.5f
                }, 1.5f);
                if (nearPizzeriaCounter && IsKeyPressed(KEY_F)) {
                    isPizzeriaOpen = true;
                    EnableCursor();
                }
                bool nearPizzaJob = CheckCollisionSpheres(camera.position, 3.5f, {
                    pizzaJobPos.x, 1.0f, pizzaJobPos.z + 1.5f
                }, 1.5f);
                if (nearPizzaJob && IsKeyPressed(KEY_F)) {
                    if (!pizzaJobActive) {
                        pizzaJobActive = true;
                        pizzaJobTarget = GetRandomValue(0, PIZZA_TARGET_COUNT - 1);
                        notificationMsg = TextFormat("Dovoz pizze: %s. Sipka ta navadza na miesto!", pizzaTargetNames[pizzaJobTarget]);
                    }
                    else {
                        notificationMsg = "Mas uz aktivny dovol pizze!";
                    }
                    notificationTimer = 3.5f;
                }
                if (pizzaJobActive) {
                    Vector3 pt = pizzaTargets[pizzaJobTarget];
                    if (CheckCollisionSpheres(camera.position, 5.0f, {
                        pt.x, 1.0f, pt.z
                    }, 3.0f) && IsKeyPressed(KEY_F)) {
                        money += modPizzaPay;
                        pizzaJobsDone++;
                        notificationMsg = TextFormat("Pizza dorucena do %s! Zarobene %.0f EUR", pizzaTargetNames[pizzaJobTarget], modPizzaPay);
                        notificationTimer = 3.5f;
                        pizzaJobActive = false;
                    }
                }
                bool nearGasPump = CheckCollisionSpheres(camera.position, 3.5f, gasPump1Pos, 1.5f);
                if (nearGasPump && (IsKeyPressed(KEY_F) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionSpheres(camera.position, 2.5f, gasPump1Pos, 1.0f)))) {
                    if (fuel >= 4.95f) {
                        notificationMsg = "Nadrz Babetty je uz plna!";
                        notificationTimer = 2.5f;
                    }
                    else {
                        float litersNeeded = 5.0f - fuel;
                        float fuelCost = litersNeeded * 1.65f;
                        if (money >= fuelCost) {
                            money -= fuelCost;
                            fuel = 5.0f;
                            notificationMsg = TextFormat("Natankovane %.1f L MIX 2T za %.2f EUR!", litersNeeded, fuelCost);
                            notificationTimer = 3.5f;
                        }
                        else {
                            notificationMsg = "Nemas dost penazi na natankovanie!";
                            notificationTimer = 2.5f;
                        }
                    }
                }
                bool nearWheatPickup = CheckCollisionSpheres(camera.position, 3.5f, farmWheatPickupPos, 1.8f);
                bool nearWheatSeller = CheckCollisionSpheres(camera.position, 3.5f, {
                    farmSellingStandPos.x, 1.0f, farmSellingStandPos.z + 3.0f
                }, 1.8f);
                if (nearWheatPickup && IsKeyPressed(KEY_F)) {
                    if (!farmWheatReady) {
                        notificationMsg = "PSENICA este rastie. Pockaj chvilu.";
                    }
                    else if (wheatInventory == 0) {
                        wheatInventory = farmWheatLoad;
                        farmWheatReady = false;
                        farmWheatRegrowTimer = modFarmRegrowTime;
                        notificationMsg = "Nalozil si 20 PSENICA. Odvez ju do predajneho stanku!";
                    }
                    else {
                        notificationMsg = "Mas uz nalozenych 20 PSENICA.";
                    }
                    notificationTimer = 3.5f;
                }
                else if (nearWheatSeller && IsKeyPressed(KEY_F)) {
                    if (wheatInventory > 0) {
                        float payment = wheatInventory * farmWheatPrice;
                        money += payment;
                        notificationMsg = TextFormat("Predal si %i PSENICA za %.0f EUR!", wheatInventory, payment);
                        wheatInventory = 0;
                    }
                    else {
                        notificationMsg = "Najprv naloz PSENICA na farme!";
                    }
                    notificationTimer = 3.5f;
                }
                // NEW: Delivery Job system
                float distToJob = Vector3Distance(camera.position, jobDeliveryPos);
                if (modEnableJobs && !jobIsActive && distToJob < 15.0f) {
                    if (IsKeyPressed(KEY_F)) {
                        jobIsActive = true;
                        jobRegrowTimer = modJobTimer;
                        notificationMsg = TextFormat("Zacal si DODACIU Job! Zarobis %.0f EUR za dokoncenie.", modJobPay);
                        notificationTimer = 3.5f;
                    }
                }
                if (jobIsActive) {
                    // Show timer and instruction
                    notificationMsg = TextFormat("Dostav baliky do mesta za %.1f min... (F pre dokoncenie)", modJobTimer/60.0f);
                    if (IsKeyPressed(KEY_F) && jobRegrowTimer <= 0.0f) {
                        money += modJobPay;
                        jobIsActive = false;
                        notificationMsg = TextFormat("Dostavka dokoncena! Zarobene: %.0f EUR", modJobPay);
                        notificationTimer = 3.5f;
                    }
                    else if (IsKeyPressed(KEY_F)) {
                        jobRegrowTimer -= GetFrameTime();
                        if (jobRegrowTimer <= 0.0f) {
                            jobIsActive = false;
                            notificationMsg = "Cas vypršel! Job zrušený.";
                            notificationTimer = 3.5f;
                        }
                    }
                }
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (nearShopCounter) {
                        isShopOpen = true;
                        EnableCursor();
                    }
                    if (nearMechanic) {
                        isMechanicOpen = true;
                        EnableCursor();
                    }
                    if (nearRestaurant) {
                        isRestaurantOpen = true;
                        EnableCursor();
                    }
                    if (nearRepair) {
                        isRepairOpen = true;
                        EnableCursor();
                    }
                    if (nearPizzeriaCounter) {
                        isPizzeriaOpen = true;
                        EnableCursor();
                    }
                    if (nearPizzaJob && !pizzaJobActive) {
                        pizzaJobActive = true;
                        pizzaJobTarget = GetRandomValue(0, PIZZA_TARGET_COUNT - 1);
                        notificationMsg = TextFormat("Dovoz pizze: %s. Sipka ta navadza na miesto!", pizzaTargetNames[pizzaJobTarget]);
                        notificationTimer = 3.5f;
                    }
                    if (CheckCollisionSpheres(camera.position, 1.5f, radioPos, 0.5f)) {
                        radioOn = !radioOn;
                        notificationMsg = radioOn ? "Radio zapnute" : "Radio vypnute";
                        notificationTimer = 1.5f;
                    }
                    if (holdingIdx == -1) {
                        for (int i = 0; i < (int)parts.size(); i++)                             if (!parts[i].installed && CheckCollisionSpheres(camera.position, 2.0f, parts[i].pos, 0.6f)) {
                            holdingIdx = i;
                            break;
                        }
                        if (holdingIdx == -1 && CheckCollisionSpheres(camera.position, 1.5f, babettaPos, 1.0f)) {
                            for (auto& p : parts) if (p.installed) {
                                p.installed = false;
                                p.pos = Vector3Add(camera.position, {
                                    1,0,0
                                });
                                break;
                            }
                        }
                    }
                    else holdingIdx = -1;
                }
                if (holdingIdx != -1) {
                    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
                    parts[holdingIdx].pos = Vector3Add(camera.position, Vector3Scale(forward, 1.5f));
                    if (CheckCollisionSpheres(parts[holdingIdx].pos, 0.6f, babettaPos, 1.0f)) {
                        parts[holdingIdx].installed = true;
                        holdingIdx = -1;
                    }
                }
                bool ready = true;
                for(auto& p : parts) if(!p.installed) ready = false;
                if (ready && IsKeyPressed(KEY_E)) {
                    isRiding = true;
                    holdingIdx = -1;
                    mouseLookPitch = 0.0f;
                    mouseLookYaw = 0.0f;
                }
            }
            else {
                // JAZDA NA BABETTE / JAWE
                float baseSpeedKMH = isRidingJawa ? 110.0f : 42.0f;
                if (!isRidingJawa) {
                    if (hasTuningMotor) baseSpeedKMH = 65.0f;
                    if (hasSportCarb) baseSpeedKMH += 10.0f;
                    if (hasSportExhaust) baseSpeedKMH += 5.0f;
                    if (hasTurboKit) baseSpeedKMH += 10.0f;
                    if (hasSportWheels) baseSpeedKMH += 3.0f;
                    // PREMIUM diely z autoservisu
                    if (hasBigBoreKit) baseSpeedKMH += 20.0f;
                    if (hasRaceCDI) baseSpeedKMH += 8.0f;
                    if (hasTitaniumExhaust) baseSpeedKMH += 10.0f;
                    if (modMaxSpeed != 42.0f && modMaxSpeed > 0.0f) baseSpeedKMH = modMaxSpeed;
                }
                float finalMaxKMH = baseSpeedKMH * modSpeedMultiplier;
                float maxSpeedVal = finalMaxKMH / 100.0f;
                bool throttleActive = false;
                bool brakeActive = false;
                isNitroActive = false;
                if (IsKeyPressed(KEY_H)) {
                    hornTimer = 0.8f;
                }
                if ((modEnableJump || IsKeyDown(KEY_SPACE)) && babettaPosY <= 0.001f && IsKeyPressed(KEY_SPACE)) {
                    babettaVelY = 0.20f * (1.0f / modGravity);
                }
                if (IsKeyDown(KEY_LEFT_SHIFT) && (modEnableNitro || hasTuningMotor) && fuel > 0.1f && speed > 0.05f) {
                    isNitroActive = true;
                    if (!modInfiniteFuel) fuel -= 0.00025f;
                    speed += 0.006f * modSpeedMultiplier;
                    float nitroMax = maxSpeedVal * 1.5f;
                    if (speed > nitroMax) speed = nitroMax;
                }
                if (IsKeyDown(KEY_W) && (fuel > 0.0f || modInfiniteFuel)) {
                    throttleActive = true;
                    if (!modInfiniteFuel) fuel -= 0.00007f;
                    float speedRatio = speed / maxSpeedVal;
                    if (speedRatio < 0.0f) speedRatio = 0.0f;
                    if (speedRatio > 1.0f) speedRatio = 1.0f;
                    float powerCurve = 1.0f - speedRatio * 0.82f;
                    float accel = (hasTuningMotor ? 0.0036f : 0.0026f) * powerCurve * modSpeedMultiplier;
                    if (hasRaceClutch) accel *= 1.35f;
                    speed += accel;
                    if (!isNitroActive && speed > maxSpeedVal) speed = maxSpeedVal;
                }
                else if (IsKeyDown(KEY_S)) {
                    brakeActive = true;
                    if (speed > 0.005f) {
                        speed -= hasBremboBrakes ? 0.013f : 0.008f;
                        if (speed < 0.0f) speed = 0.0f;
                    }
                    else {
                        speed -= 0.0008f;
                        if (speed < -0.03f) speed = -0.03f;
                    }
                }
                else {
                    speed *= 0.988f;
                    if (fabs(speed) < 0.001f) speed = 0.0f;
                }
                babettaPosY += babettaVelY;
                babettaVelY -= 0.012f * modGravity;
                if (babettaPosY <= 0.0f) {
                    babettaPosY = 0.0f;
                    babettaVelY = 0.0f;
                }
                float targetLean = 0.0f;
                if (fabs(speed) > 0.002f) {
                    float steerSpeed = 2.2f - (speed / maxSpeedVal) * 0.6f;
                    if (hasOhlinsSuspension) steerSpeed *= 1.25f;
                    if (speed < 0.0f) steerSpeed = -steerSpeed;
                    if (IsKeyDown(KEY_A)) {
                        babettaRot += steerSpeed;
                        targetLean = -14.0f * (speed / maxSpeedVal);
                    }
                    if (IsKeyDown(KEY_D)) {
                        babettaRot -= steerSpeed;
                        targetLean = 14.0f * (speed / maxSpeedVal);
                    }
                }
                babettaLean += (targetLean - babettaLean) * 0.14f;
                Vector3 nextBabettaPos = babettaPos;
                nextBabettaPos.x += sinf(babettaRot * DEG2RAD) * speed;
                nextBabettaPos.z += cosf(babettaRot * DEG2RAD) * speed;
                if (CheckCollisionWithWorld(nextBabettaPos, 0.6f)) {
                    speed = 0.0f;
                }
                else {
                    babettaPos = nextBabettaPos;
                }
                if (speed > 0.0f) {
                    distanceTraveledKM += (speed * 100.0f / 3600.0f) * GetFrameTime();
                }
                bool atGasPump = CheckCollisionSpheres(babettaPos, 3.2f, gasPump1Pos, 1.5f);
                if (atGasPump && IsKeyPressed(KEY_F)) {
                    if (fuel >= 4.95f) {
                        notificationMsg = "Nadrz Babetty je uz plna!";
                        notificationTimer = 2.5f;
                    }
                    else {
                        float litersNeeded = 5.0f - fuel;
                        float fuelCost = litersNeeded * 1.65f;
                        if (money >= fuelCost) {
                            money -= fuelCost;
                            fuel = 5.0f;
                            notificationMsg = TextFormat("Natankovane %.1f L MIX 2T za %.2f EUR!", litersNeeded, fuelCost);
                            notificationTimer = 3.5f;
                        }
                        else {
                            notificationMsg = "Nemas dost penazi!";
                            notificationTimer = 2.5f;
                        }
                    }
                }
                bool atWheatPickup = CheckCollisionSpheres(babettaPos, 3.5f, farmWheatPickupPos, 1.8f);
                bool atWheatSeller = CheckCollisionSpheres(babettaPos, 3.5f, {
                    farmSellingStandPos.x, 1.0f, farmSellingStandPos.z + 3.0f
                }, 1.8f);
                if (atWheatPickup && IsKeyPressed(KEY_F)) {
                    if (!farmWheatReady) {
                        notificationMsg = "PSENICA este rastie. Pockaj chvilu.";
                    }
                    else if (wheatInventory == 0) {
                        wheatInventory = farmWheatLoad;
                        farmWheatReady = false;
                        farmWheatRegrowTimer = modFarmRegrowTime;
                        notificationMsg = "Nalozil si 20 PSENICA. Odvez ju do predajneho stanku!";
                    }
                    else {
                        notificationMsg = "Mas uz nalozenych 20 PSENICA.";
                    }
                    notificationTimer = 3.5f;
                }
                else if (atWheatSeller && IsKeyPressed(KEY_F)) {
                    if (wheatInventory > 0) {
                        float payment = wheatInventory * farmWheatPrice;
                        money += payment;
                        notificationMsg = TextFormat("Predal si %i PSENICA za %.0f EUR!", wheatInventory, payment);
                        wheatInventory = 0;
                    }
                    else {
                        notificationMsg = "Najprv naloz PSENICA na farme!";
                    }
                    notificationTimer = 3.5f;
                }
                bool atMechanic = CheckCollisionSpheres(babettaPos, 4.0f, {
                    mechanicCounterPos.x, 1.0f, mechanicCounterPos.z + 1.5f
                }, 2.0f);
                if (atMechanic && IsKeyPressed(KEY_F)) {
                    isMechanicOpen = true;
                    EnableCursor();
                }
                bool atRestaurant = CheckCollisionSpheres(babettaPos, 4.0f, {
                    restaurantCounterPos.x, 1.0f, restaurantCounterPos.z + 1.5f
                }, 2.0f);
                if (atRestaurant && IsKeyPressed(KEY_F)) {
                    isRestaurantOpen = true;
                    EnableCursor();
                }
                bool atRepair = CheckCollisionSpheres(babettaPos, 4.0f, {
                    repairCounterPos.x, 1.0f, repairCounterPos.z - 1.5f
                }, 2.0f);
                if (atRepair && IsKeyPressed(KEY_F)) {
                    isRepairOpen = true;
                    EnableCursor();
                }
                bool atPizzeria = CheckCollisionSpheres(babettaPos, 4.0f, {
                    pizzeriaCounterPos.x, 1.0f, pizzeriaCounterPos.z - 1.5f
                }, 2.0f);
                if (atPizzeria && IsKeyPressed(KEY_F)) {
                    isPizzeriaOpen = true;
                    EnableCursor();
                }
                bool atPizzaJob = CheckCollisionSpheres(babettaPos, 4.0f, {
                    pizzaJobPos.x, 1.0f, pizzaJobPos.z + 1.5f
                }, 2.0f);
                if (atPizzaJob && IsKeyPressed(KEY_F)) {
                    if (!pizzaJobActive) {
                        pizzaJobActive = true;
                        pizzaJobTarget = GetRandomValue(0, PIZZA_TARGET_COUNT - 1);
                        notificationMsg = TextFormat("Dovoz pizze: %s. Sipka ta navadza na miesto!", pizzaTargetNames[pizzaJobTarget]);
                    }
                    else {
                        notificationMsg = "Mas uz aktivny dovol pizze!";
                    }
                    notificationTimer = 3.5f;
                }
                if (pizzaJobActive) {
                    Vector3 pt = pizzaTargets[pizzaJobTarget];
                    if (CheckCollisionSpheres(babettaPos, 6.0f, {
                        pt.x, 1.0f, pt.z
                    }, 3.0f) && IsKeyPressed(KEY_F)) {
                        money += modPizzaPay;
                        pizzaJobsDone++;
                        notificationMsg = TextFormat("Pizza dorucena do %s! Zarobene %.0f EUR", pizzaTargetNames[pizzaJobTarget], modPizzaPay);
                        notificationTimer = 3.5f;
                        pizzaJobActive = false;
                    }
                }
                Vector3 current3DPos = { babettaPos.x, babettaPos.y + babettaPosY, babettaPos.z };
                if (firstPersonMode) {
                    Vector2 mDelta = GetMouseDelta();
                    mouseLookPitch -= mDelta.y * 0.12f;
                    mouseLookYaw += mDelta.x * 0.12f;
                    if (mouseLookPitch > 65.0f) mouseLookPitch = 65.0f;
                    if (mouseLookPitch < -65.0f) mouseLookPitch = -65.0f;
                    if (mouseLookYaw > 75.0f) mouseLookYaw = 75.0f;
                    if (mouseLookYaw < -75.0f) mouseLookYaw = -75.0f;
                    Vector3 headOffsetLocal = { 0.0f, 1.25f, -0.15f };
                    Vector3 headPos = Vector3Add(current3DPos, Vector3RotateByQuaternion(headOffsetLocal, QuaternionFromEuler(0, babettaRot * DEG2RAD, babettaLean * DEG2RAD)));
                    float totalYaw = babettaRot + mouseLookYaw;
                    Vector3 lookDir = { sinf(totalYaw * DEG2RAD) * cosf(mouseLookPitch * DEG2RAD),                        sinf(mouseLookPitch * DEG2RAD),                        cosf(totalYaw * DEG2RAD) * cosf(mouseLookPitch * DEG2RAD) };
                    camera.position = headPos;
                    camera.target = Vector3Add(headPos, lookDir);
                }
                else {
                    float camDist = 4.2f + (speed / maxSpeedVal) * 1.6f;
                    Vector3 desiredCamPos = { current3DPos.x - sinf(babettaRot * DEG2RAD) * camDist,                        current3DPos.y + 2.1f,                        current3DPos.z - cosf(babettaRot * DEG2RAD) * camDist };
                    camera.position = Vector3Lerp(camera.position, desiredCamPos, 0.14f);
                    camera.target = Vector3Add(current3DPos, (Vector3){
                        0, 0.85f, 0
                    });
                }
                if (IsKeyPressed(KEY_E)) isRiding = false;
            }
        }
        // ==========================================
        // VYKRESLOVANIE SCENY
        // ==========================================
        BeginDrawing();
        ClearBackground(modNightMode ? (Color){
            10, 15, 30, 255
        }
        : SKYBLUE);
        BeginMode3D(camera);
        DrawMapEnvironment();
        // 3D marker ciela rozvozu pizze
        if (pizzaJobActive) {
            Vector3 t = pizzaTargets[pizzaJobTarget];
            float bob = sinf((float)GetTime() * 3.0f) * 0.25f;
            DrawCylinder({
                t.x, 0.06f, t.z
            }, 2.4f, 2.4f, 0.04f, 24, ColorAlpha((Color){
                250, 200, 60, 255
            }, 0.45f));
            DrawCylinderEx({
                t.x, 0.1f, t.z
            },
            { t.x, 3.5f, t.z }, 2.2f, 1.4f, 16, ColorAlpha((Color){
                250, 200, 60, 255
            }, 0.20f));
            rlPushMatrix();
            rlTranslatef(t.x, 3.4f + bob, t.z);
            rlRotatef((float)GetTime() * 90.0f, 0.0f, 1.0f, 0.0f);
            DrawCube({
                0.0f, 0.0f, 0.0f
            }, 1.2f, 1.2f, 1.2f, (Color){
                250, 200, 60, 255
            });
            DrawCubeWires({
                0.0f, 0.0f, 0.0f
            }, 1.3f, 1.3f, 1.3f, (Color){
                150, 90, 20, 255
            });
            rlPopMatrix();
        }
        DrawRadio(radioPos, radioOn);
        Vector3 babettaRenderPos = { babettaPos.x, babettaPos.y + babettaPosY, babettaPos.z };
        bool throttleDown = IsKeyDown(KEY_W);
        bool brakeDown = IsKeyDown(KEY_S);
        DrawDetailedBabetta(babettaRenderPos, babettaRot, babettaLean, parts, hasTuningMotor, hasSportCarb, hasSportExhaust, isNitroActive, headlightOn, isRiding, firstPersonMode, throttleDown, brakeDown);
        if (!isRiding && !firstPersonMode) {
            float playerYaw = atan2f(camera.target.x - camera.position.x, camera.target.z - camera.position.z) * RAD2DEG;
            DrawPlayerModel(Vector3Subtract(camera.position, {
                0, 1.8f, 0
            }), playerYaw, false, playerWalkAnim);
        }
        for (auto& p : parts) {
            if (!p.installed) {
                if (p.isSphere) {
                    DrawCylinderEx(Vector3Add(p.pos, {
                        -0.05f, 0, 0
                    }), Vector3Add(p.pos, {
                        0.05f, 0, 0
                    }), 0.35f, 0.35f, 12, BLACK);
                }
                else {
                    DrawCube(p.pos, 0.3f, 0.3f, 0.3f, GRAY);
                    DrawCylinder(Vector3Add(p.pos, {
                        0, 0.15f, 0
                    }), 0.1f, 0.1f, 0.2f, 8, DARKGRAY);
                }
            }
        }
        EndMode3D();
        // ==========================================
        // 2D HUD A INTERFEJS (CISTE TEXTY BEZ MAKCEŇOV)
        // ==========================================
        if (showFPS) DrawFPS(10, 10);
        DrawText(TextFormat("Peniaze: %.0f EUR", money), 10, 40, 20, GREEN);
        DrawText(TextFormat("PSENICA: %i / %i", wheatInventory, farmWheatLoad), 10, 65, 20, GOLD);
        DrawText(TextFormat("Hlad: %.1f%%", hunger*100), 10, 90, 20, ORANGE);
        DrawText(radioOn ? TextFormat("Radio: ON (%.0f%%)", radioVolume / 0.85f * 100.0f) : "Radio: OFF", 10, 120, 20, radioOn ? BLUE : RED);
        DrawText(TextFormat("MODS LOADED: %i", (int)activeMods.size()), 10, 130, 20, GOLD);
        if (modSpeedMultiplier != 1.0f) DrawText(TextFormat("SPEED BOOST: %.1fx", modSpeedMultiplier), 10, 155, 18, YELLOW);
        if (modEnableNitro || hasTuningMotor) DrawText("NITRO: [LSHIFT]", 10, 175, 18, SKYBLUE);
        if (modGravity != 1.0f) DrawText("LOW GRAV MOD", 10, 195, 18, MAGENTA);
        if (modHasCustomColor) DrawText("CUSTOM FARBA", 10, 215, 18, GREEN);
        if (hasTuningMotor) DrawText("MOTOR: 70cc TUNING", 10, 235, 18, GOLD);
        if (hasSportCarb) DrawText("KARBURATOR: DELLORTO", 10, 255, 18, RED);
        if (hasSportExhaust) DrawText("VYFUK: GIANNELLI SPORT", 10, 275, 18, (Color){
            100, 180, 255, 255
        });
        if (modFastGrowing) DrawText("FAST GROW MOD: 0.1 ms", 10, 295, 18, LIME);
        if (hasBigBoreKit) DrawText("PREMIUM: 90cc BIG BORE + RACE DIELY", 10, 315, 18, (Color){
            255, 175, 70, 255
        });
        if (hornTimer > 0.0f) {
            DrawRectangle(520, 220, 240, 60, ColorAlpha(YELLOW, 0.85f));
            DrawRectangleLines(520, 220, 240, 60, RED);
            DrawText("TUUT! (Klakson)", 540, 235, 24, RED);
        }
        if (!isRiding && !isShopOpen && !isDealerOpen && !isMechanicOpen && !isRestaurantOpen && !isRepairOpen && !isPizzeriaOpen) {
            bool nearCounter = CheckCollisionSpheres(camera.position, 3.2f, {
                shopCounterPos.x, 1.0f, shopCounterPos.z + 1.8f
            }, 1.2f);
            if (nearCounter) {
                DrawRectangle(400, 560, 480, 50, ColorAlpha(BLACK, 0.75f));
                DrawRectangleLines(400, 560, 480, 50, GOLD);
                DrawText("[F] Otvorit MOTO-TUNING SHOP", 460, 575, 20, YELLOW);
            }
            bool nearDlr = CheckCollisionSpheres(camera.position, 4.5f, {
                dealerCounterPos.x, 1.0f, dealerCounterPos.z + 2.0f
            }, 1.5f);
            if (nearDlr) {
                DrawRectangle(340, 560, 600, 50, ColorAlpha((Color){
                    10,15,40,255
                }, 0.85f));
                DrawRectangleLines(340, 560, 600, 50, (Color){
                    80,110,255,255
                });
                DrawText("[F] Otvorit MOTO DEALER - Jawa & Babetta", 365, 575, 20, (Color){
                    160,190,255,255
                });
            }
            bool nearMech = CheckCollisionSpheres(camera.position, 3.5f, {
                mechanicCounterPos.x, 1.0f, mechanicCounterPos.z + 1.5f
            }, 1.5f);
            if (nearMech) {
                DrawRectangle(300, 560, 680, 50, ColorAlpha((Color){
                    45,25,5,255
                }, 0.9f));
                DrawRectangleLines(300, 560, 680, 50, (Color){
                    255,170,60,255
                });
                DrawText("[F] Otvorit AUTOSERVIS - PREMIUM UPGRADY", 345, 575, 20, (Color){
                    255,190,90,255
                });
            }
            bool nearRest = CheckCollisionSpheres(camera.position, 3.5f, {
                restaurantCounterPos.x, 1.0f, restaurantCounterPos.z + 1.5f
            }, 1.5f);
            if (nearRest) {
                DrawRectangle(320, 560, 640, 50, ColorAlpha((Color){
                    30,20,10,255
                }, 0.9f));
                DrawRectangleLines(320, 560, 640, 50, (Color){
                    240,200,60,255
                });
                DrawText("[F] Otvorit RESTAURACIU - Slovenske jedla", 355, 575, 20, (Color){
                    255,225,140,255
                });
            }
            bool nearRep = CheckCollisionSpheres(camera.position, 3.5f, {
                repairCounterPos.x, 1.0f, repairCounterPos.z - 1.5f
            }, 1.5f);
            if (nearRep) {
                DrawRectangle(300, 560, 680, 50, ColorAlpha((Color){
                    45,28,8,255
                }, 0.9f));
                DrawRectangleLines(300, 560, 680, 50, (Color){
                    238,148,32,255
                });
                DrawText("[F] Pracovat v OPRAVOVNI - minihra: Zloz Babetu", 325, 575, 20, (Color){
                    255,190,90,255
                });
            }
            bool nearPizzaCounter = CheckCollisionSpheres(camera.position, 3.5f, {
                pizzeriaCounterPos.x, 1.0f, pizzeriaCounterPos.z - 1.5f
            }, 1.5f);
            if (nearPizzaCounter) {
                DrawRectangle(300, 560, 680, 50, ColorAlpha((Color){
                    40,18,14,255
                }, 0.9f));
                DrawRectangleLines(300, 560, 680, 50, (Color){
                    214,42,34,255
                });
                DrawText("[F] Real Svinna PIZZERIA LUCIA - kupit pizzu", 360, 575, 20, (Color){
                    255,215,150,255
                });
            }
            bool nearPizzaJob = CheckCollisionSpheres(camera.position, 3.5f, {
                pizzaJobPos.x, 1.0f, pizzaJobPos.z + 1.5f
            }, 1.5f);
            if (nearPizzaJob) {
                DrawRectangle(320, 560, 640, 50, ColorAlpha((Color){
                    18,28,40,255
                }, 0.9f));
                DrawRectangleLines(320, 560, 640, 50, (Color){
                    235,185,45,255
                });
                DrawText(pizzaJobActive ? "[F] Uz mas aktivny dovol pizze" : "[F] Pracovat - DOVOZ PIZZE", 385, 575, 20, (Color){
                    255,220,140,255
                });
            }
        }
        // HUD aktivneho dovozu pizze
        if (pizzaJobActive) {
            DrawRectangle(420, 115, 440, 58, ColorAlpha((Color){
                30,20,10,255
            }, 0.85f));
            DrawRectangleLines(420, 115, 440, 58, (Color){
                240,185,45,255
            });
            DrawText("DOVOZ PIZZE", 435, 122, 18, (Color){
                255,215,120,255
            });
            DrawText(TextFormat("Ciel: %s", pizzaTargetNames[pizzaJobTarget]), 435, 144, 18, WHITE);
        }
        Vector3 testPos = isRiding ? babettaPos : camera.position;
        bool nearFarmPickup = CheckCollisionSpheres(testPos, 3.5f, farmWheatPickupPos, 1.8f);
        bool nearFarmSeller = CheckCollisionSpheres(testPos, 3.5f, {
            farmSellingStandPos.x, 1.0f, farmSellingStandPos.z + 3.0f
        }, 1.8f);
        if ((nearFarmPickup || nearFarmSeller) && !isShopOpen && !isDealerOpen && !isMechanicOpen && !isRestaurantOpen && !isRepairOpen && !isPizzeriaOpen) {
            DrawRectangle(340, 445, 600, 50, ColorAlpha((Color){
                85, 55, 5, 255
            }, 0.9f));
            DrawRectangleLines(340, 445, 600, 50, GOLD);
            if (nearFarmPickup && !farmWheatReady) DrawText("PSENICA RASTIE...", 555, 460, 18, YELLOW);
            else if (nearFarmPickup) DrawText("[F] Nalozit 20 PSENICA (predaj: 10 = 100 EUR)", 365, 460, 18, YELLOW);
            else DrawText("[F] Predat PSENICA - 10 kusov = 100 EUR", 390, 460, 18, YELLOW);
        }
        bool nearGas = CheckCollisionSpheres(testPos, 3.5f, gasPump1Pos, 1.5f);
        if (nearGas && !isShopOpen && !isDealerOpen && !isMechanicOpen && !isRestaurantOpen && !isRepairOpen && !isPizzeriaOpen) {
            DrawRectangle(370, 505, 540, 50, ColorAlpha((Color){
                0, 80, 35, 255
            }, 0.85f));
            DrawRectangleLines(370, 505, 540, 50, GREEN);
            DrawText("[F] Natankovat nadrz Babetty (MIX 2T - 1.65 EUR/L)", 385, 520, 18, WHITE);
        }
        if (!notificationMsg.empty()) {
            DrawRectangle(420, 60, 440, 45, ColorAlpha(DARKGREEN, 0.85f));
            DrawRectangleLines(420, 60, 440, 45, GREEN);
            DrawText(notificationMsg.c_str(), 435, 72, 18, WHITE);
        }
        if (isRiding) {
            float baseSpeedKMH = isRidingJawa ? 110.0f : 42.0f;
            if (!isRidingJawa) {
                if (hasTuningMotor) baseSpeedKMH = 65.0f;
                if (hasSportCarb) baseSpeedKMH += 10.0f;
                if (hasSportExhaust) baseSpeedKMH += 5.0f;
                if (hasTurboKit) baseSpeedKMH += 10.0f;
                if (hasSportWheels) baseSpeedKMH += 3.0f;
                // PREMIUM diely z autoservisu
                if (hasBigBoreKit) baseSpeedKMH += 20.0f;
                if (hasRaceCDI) baseSpeedKMH += 8.0f;
                if (hasTitaniumExhaust) baseSpeedKMH += 10.0f;
                if (modMaxSpeed != 42.0f && modMaxSpeed > 0.0f) baseSpeedKMH = modMaxSpeed;
            }
            float finalMaxKMH = baseSpeedKMH * modSpeedMultiplier;
            DrawRectangle(925, 565, 345, 145, ColorAlpha(BLACK, 0.75f));
            DrawRectangleLines(925, 565, 345, 145, RED);
            DrawText(TextFormat("Rychlost: %i KM/H", (int)(speed*100)), 940, 573, 22, WHITE);
            DrawText(TextFormat("Limit: %.0f KM/H", finalMaxKMH), 940, 600, 18, ORANGE);
            DrawText(TextFormat("Najazdene: %.2f KM", distanceTraveledKM), 940, 625, 18, LIGHTGRAY);
            DrawText(TextFormat("Palivo: %.1f L", fuel), 940, 650, 18, (fuel < 1.0f) ? RED : YELLOW);
            DrawText(TextFormat("Pohlad: %s [V]", firstPersonMode ? "1. OSOBA" : "3. OSOBA"), 940, 672, 14, SKYBLUE);
            DrawText("Svetlo: [L] | Klakson: [H] | Skok: [SPACE]", 940, 692, 12, GOLD);
            DrawText("W - Plyn | S - Brzda/Cuvanie | A,D - Zatacanie | LSHIFT - Nitro | E - Zosadnut", 220, 685, 17, WHITE);
        }
        else if (!isShopOpen && !isDealerOpen && !isMechanicOpen && !isRestaurantOpen && !isRepairOpen && !isPizzeriaOpen) {
            DrawText("E - Karovat | LMB - Interakcia | V - Kamera | L - Svetlo | F - Obchod / Pumpa", 230, 685, 17, WHITE);
        }
        // ============================================
        // MOTO-TUNING SHOP MENU
        // ============================================
        if (isShopOpen) {
            DrawRectangle(0, 0, 1280, 720, ColorAlpha(BLACK, 0.72f));
            int shopX = 80, shopY = 30, shopW = 1120, shopH = 650;
            DrawRectangle(shopX, shopY, shopW, shopH, (Color){
                22, 28, 38, 255
            });
            DrawRectangleLines(shopX, shopY, shopW, shopH, GOLD);
            DrawRectangle(shopX, shopY, shopW, 52, (Color){
                12, 17, 28, 255
            });
            DrawText("MOTO-TUNING SHOP  -  MECHANIK JOZO", shopX + 25, shopY + 15, 22, GOLD);
            DrawText(TextFormat("Peniaze: %.0f EUR", money), shopX + 820, shopY + 16, 20, GREEN);
            Vector2 mPos = GetMousePosition();
            // Lava kolumna (uplne nowe + stare upgrady)
            int col1X = shopX + 20, col2X = shopX + 575;
            int rowH = 72, startY = shopY + 62;
            // --- helper lambda approach via inline draw ---
            // ROW 1: Motor 70cc
            Rectangle r1 = { (float)col1X, (float)startY, 540.0f, (float)rowH };
            DrawRectangleRec(r1, hasTuningMotor ? (Color){
                20,45,20,255
            }
            : (Color){
                32,38,52,255
            });
            DrawRectangleLinesEx(r1, 1, hasTuningMotor ? GREEN : GRAY);
            DrawText("1. Tuning Motor 70cc", r1.x+10, r1.y+8, 17, YELLOW);
            DrawText("+23 KM/H | Odomkne NITRO [LSHIFT]", r1.x+10, r1.y+30, 13, LIGHTGRAY);
            DrawText("450 EUR", r1.x+10, r1.y+50, 13, GREEN);
            {
                Rectangle b={ r1.x+r1.width-130,r1.y+16,120,38 };
                bool h=CheckCollisionPointRec(mPos,b);
                if(hasTuningMotor){
                    DrawRectangleRec(b,DARKGRAY);
                    DrawText("KUPENE",b.x+22,b.y+11,14,GREEN);
                }
                else{
                    DrawRectangleRec(b,h?(Color){
                        50,160,50,255
                    }
                    :(Color){
                        35,120,35,255
                    });
                    DrawText("KUPIT [1]",b.x+16,b.y+11,14,WHITE);
                    if((h&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON))||IsKeyPressed(KEY_ONE)){
                        if(money>=450.f){
                            money-=450.f;
                            hasTuningMotor=true;
                            notificationMsg="Kupil si Tuning Motor 70cc!";
                            notificationTimer=3.5f;
                        }
                        else{
                            notificationMsg="Nemas dostatok penazi!";
                            notificationTimer=2.f;
                        }
                    }
                }
            }
            // ROW 2: Karburator Dellorto
            Rectangle r2 = { (float)col1X, (float)startY+rowH+6, 540.0f, (float)rowH };
            DrawRectangleRec(r2, hasSportCarb ? (Color){
                20,45,20,255
            }
            : (Color){
                32,38,52,255
            });
            DrawRectangleLinesEx(r2, 1, hasSportCarb ? GREEN : GRAY);
            DrawText("2. Karburator Dellorto 16mm", r2.x+10, r2.y+8, 17, RED);
            DrawText("+10 KM/H | Lepsia odozva plynu", r2.x+10, r2.y+30, 13, LIGHTGRAY);
            DrawText("180 EUR", r2.x+10, r2.y+50, 13, GREEN);
            {
                Rectangle b={ r2.x+r2.width-130,r2.y+16,120,38 };
                bool h=CheckCollisionPointRec(mPos,b);
                if(hasSportCarb){
                    DrawRectangleRec(b,DARKGRAY);
                    DrawText("KUPENE",b.x+22,b.y+11,14,GREEN);
                }
                else{
                    DrawRectangleRec(b,h?(Color){
                        50,160,50,255
                    }
                    :(Color){
                        35,120,35,255
                    });
                    DrawText("KUPIT [2]",b.x+16,b.y+11,14,WHITE);
                    if((h&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON))||IsKeyPressed(KEY_TWO)){
                        if(money>=180.f){
                            money-=180.f;
                            hasSportCarb=true;
                            notificationMsg="Kupil si Karburator Dellorto (+10 KM/H)!";
                            notificationTimer=3.5f;
                        }
                        else{
                            notificationMsg="Nemas dostatok penazi!";
                            notificationTimer=2.f;
                        }
                    }
                }
            }
            // ROW 3: Vyfuk Giannelli
            Rectangle r3 = { (float)col1X, (float)startY+(rowH+6)*2, 540.0f, (float)rowH };
            DrawRectangleRec(r3, hasSportExhaust ? (Color){
                20,45,20,255
            }
            : (Color){
                32,38,52,255
            });
            DrawRectangleLinesEx(r3, 1, hasSportExhaust ? GREEN : GRAY);
            DrawText("3. Vyfuk Giannelli Sport", r3.x+10, r3.y+8, 17, (Color){
                100,180,255,255
            });
            DrawText("+5 KM/H | Zlata komora | Sportovy zvuk", r3.x+10, r3.y+30, 13, LIGHTGRAY);
            DrawText("120 EUR", r3.x+10, r3.y+50, 13, GREEN);
            {
                Rectangle b={ r3.x+r3.width-130,r3.y+16,120,38 };
                bool h=CheckCollisionPointRec(mPos,b);
                if(hasSportExhaust){
                    DrawRectangleRec(b,DARKGRAY);
                    DrawText("KUPENE",b.x+22,b.y+11,14,GREEN);
                }
                else{
                    DrawRectangleRec(b,h?(Color){
                        50,160,50,255
                    }
                    :(Color){
                        35,120,35,255
                    });
                    DrawText("KUPIT [3]",b.x+16,b.y+11,14,WHITE);
                    if((h&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON))||IsKeyPressed(KEY_THREE)){
                        if(money>=120.f){
                            money-=120.f;
                            hasSportExhaust=true;
                            notificationMsg="Kupil si Vyfuk Giannelli (+5 KM/H)!";
                            notificationTimer=3.5f;
                        }
                        else{
                            notificationMsg="Nemas dostatok penazi!";
                            notificationTimer=2.f;
                        }
                    }
                }
            }
            // ROW 4: Turbo Kit
            Rectangle r4 = { (float)col1X, (float)startY+(rowH+6)*3, 540.0f, (float)rowH };
            DrawRectangleRec(r4, hasTurboKit ? (Color){
                20,45,20,255
            }
            : (Color){
                32,38,52,255
            });
            DrawRectangleLinesEx(r4, 1, hasTurboKit ? GREEN : GRAY);
            DrawText("4. Turbo Kit (Mini Turbo Kompressor)", r4.x+10, r4.y+8, 17, ORANGE);
            DrawText("+10 KM/H | Boost pri rozjazde | Turbovy zvuk", r4.x+10, r4.y+30, 13, LIGHTGRAY);
            DrawText("650 EUR", r4.x+10, r4.y+50, 13, GREEN);
            {
                Rectangle b={ r4.x+r4.width-130,r4.y+16,120,38 };
                bool h=CheckCollisionPointRec(mPos,b);
                if(hasTurboKit){
                    DrawRectangleRec(b,DARKGRAY);
                    DrawText("KUPENE",b.x+22,b.y+11,14,GREEN);
                }
                else{
                    DrawRectangleRec(b,h?(Color){
                        50,160,50,255
                    }
                    :(Color){
                        35,120,35,255
                    });
                    DrawText("KUPIT [4]",b.x+16,b.y+11,14,WHITE);
                    if((h&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON))||IsKeyPressed(KEY_FOUR)){
                        if(money>=650.f){
                            money-=650.f;
                            hasTurboKit=true;
                            notificationMsg="Kupil si Turbo Kit! +10 KM/H a boost!";
                            notificationTimer=3.5f;
                        }
                        else{
                            notificationMsg="Nemas dostatok penazi!";
                            notificationTimer=2.f;
                        }
                    }
                }
            }
            // ROW 5: Suspenzia
            Rectangle r5 = { (float)col1X, (float)startY+(rowH+6)*4, 540.0f, (float)rowH };
            DrawRectangleRec(r5, hasSportSuspension ? (Color){
                20,45,20,255
            }
            : (Color){
                32,38,52,255
            });
            DrawRectangleLinesEx(r5, 1, hasSportSuspension ? GREEN : GRAY);
            DrawText("5. Sportova Suspenzia Marzocchi", r5.x+10, r5.y+8, 17, (Color){
                180,100,255,255
            });
            DrawText("Lepsi handling a zatacanie pri vysokej rychlosti", r5.x+10, r5.y+30, 13, LIGHTGRAY);
            DrawText("280 EUR", r5.x+10, r5.y+50, 13, GREEN);
            {
                Rectangle b={ r5.x+r5.width-130,r5.y+16,120,38 };
                bool h=CheckCollisionPointRec(mPos,b);
                if(hasSportSuspension){
                    DrawRectangleRec(b,DARKGRAY);
                    DrawText("KUPENE",b.x+22,b.y+11,14,GREEN);
                }
                else{
                    DrawRectangleRec(b,h?(Color){
                        50,160,50,255
                    }
                    :(Color){
                        35,120,35,255
                    });
                    DrawText("KUPIT [5]",b.x+16,b.y+11,14,WHITE);
                    if((h&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON))||IsKeyPressed(KEY_FIVE)){
                        if(money>=280.f){
                            money-=280.f;
                            hasSportSuspension=true;
                            notificationMsg="Kupil si Sportovu Suspenzii Marzocchi!";
                            notificationTimer=3.5f;
                        }
                        else{
                            notificationMsg="Nemas dostatok penazi!";
                            notificationTimer=2.f;
                        }
                    }
                }
            }
            // ROW 6: Kolesa Pirelli
            Rectangle r6 = { (float)col1X, (float)startY+(rowH+6)*5, 540.0f, (float)rowH };
            DrawRectangleRec(r6, hasSportWheels ? (Color){
                20,45,20,255
            }
            : (Color){
                32,38,52,255
            });
            DrawRectangleLinesEx(r6, 1, hasSportWheels ? GREEN : GRAY);
            DrawText("6. Sportove Kolesa Pirelli 17\"", r6.x+10, r6.y+8, 17, (Color){
                255,210,80,255
            });
            DrawText("+3 KM/H | Lepsia prilanavost | Chromove disky", r6.x+10, r6.y+30, 13, LIGHTGRAY);
            DrawText("220 EUR", r6.x+10, r6.y+50, 13, GREEN);
            {
                Rectangle b={ r6.x+r6.width-130,r6.y+16,120,38 };
                bool h=CheckCollisionPointRec(mPos,b);
                if(hasSportWheels){
                    DrawRectangleRec(b,DARKGRAY);
                    DrawText("KUPENE",b.x+22,b.y+11,14,GREEN);
                }
                else{
                    DrawRectangleRec(b,h?(Color){
                        50,160,50,255
                    }
                    :(Color){
                        35,120,35,255
                    });
                    DrawText("KUPIT [6]",b.x+16,b.y+11,14,WHITE);
                    if((h&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON))||IsKeyPressed(KEY_SIX)){
                        if(money>=220.f){
                            money-=220.f;
                            hasSportWheels=true;
                            notificationMsg="Kupil si Kolesa Pirelli 17\" (+3 KM/H)!";
                            notificationTimer=3.5f;
                        }
                        else{
                            notificationMsg="Nemas dostatok penazi!";
                            notificationTimer=2.f;
                        }
                    }
                }
            }
            // ROW 7: LED svetla
            Rectangle r7 = { (float)col1X, (float)startY+(rowH+6)*6, 540.0f, (float)rowH };
            DrawRectangleRec(r7, hasHeadlightKit ? (Color){
                20,45,20,255
            }
            : (Color){
                32,38,52,255
            });
            DrawRectangleLinesEx(r7, 1, hasHeadlightKit ? GREEN : GRAY);
            DrawText("7. LED Svetlometovy Kit", r7.x+10, r7.y+8, 17, (Color){
                80,220,255,255
            });
            DrawText("Silnejsie predne LED svetlo | Lepsia viditelnost v noci", r7.x+10, r7.y+30, 13, LIGHTGRAY);
            DrawText("95 EUR", r7.x+10, r7.y+50, 13, GREEN);
            {
                Rectangle b={ r7.x+r7.width-130,r7.y+16,120,38 };
                bool h=CheckCollisionPointRec(mPos,b);
                if(hasHeadlightKit){
                    DrawRectangleRec(b,DARKGRAY);
                    DrawText("KUPENE",b.x+22,b.y+11,14,GREEN);
                }
                else{
                    DrawRectangleRec(b,h?(Color){
                        50,160,50,255
                    }
                    :(Color){
                        35,120,35,255
                    });
                    DrawText("KUPIT [7]",b.x+16,b.y+11,14,WHITE);
                    if((h&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON))||IsKeyPressed(KEY_SEVEN)){
                        if(money>=95.f){
                            money-=95.f;
                            hasHeadlightKit=true;
                            notificationMsg="Kupil si LED Svetlometovy Kit!";
                            notificationTimer=3.5f;
                        }
                        else{
                            notificationMsg="Nemas dostatok penazi!";
                            notificationTimer=2.f;
                        }
                    }
                }
            }
            // --- PRAVA KOLUMNA: Palivo, jedlo, farba ---
            // ROW A: Kanister benzinu
            Rectangle rA = { (float)col2X, (float)startY, 540.0f, (float)rowH };
            DrawRectangleRec(rA, (Color){
                32,38,52,255
            });
            DrawRectangleLinesEx(rA, 1, GRAY);
            DrawText("A. Plechovy Kanister (5L MIX 2T)", rA.x+10, rA.y+8, 17, GOLD);
            DrawText("Doplni plnu nadrz Babetty okamzite", rA.x+10, rA.y+30, 13, LIGHTGRAY);
            DrawText("15 EUR", rA.x+10, rA.y+50, 13, GREEN);
            {
                Rectangle b={ rA.x+rA.width-130,rA.y+16,120,38 };
                bool h=CheckCollisionPointRec(mPos,b);
                DrawRectangleRec(b,h?(Color){
                    50,160,50,255
                }
                :(Color){
                    35,120,35,255
                });
                DrawText("KUPIT [Q]",b.x+12,b.y+11,14,WHITE);
                if((h&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON))||IsKeyPressed(KEY_Q)){
                    if(money>=15.f){
                        money-=15.f;
                        fuel=5.f;
                        notificationMsg="Doplnil si plnu nadrz (5.0 L)!";
                        notificationTimer=3.f;
                    }
                    else{
                        notificationMsg="Nemas dostatok penazi!";
                        notificationTimer=2.f;
                    }
                }
            }
            // ROW B: Jedlo
            Rectangle rB = { (float)col2X, (float)startY+rowH+6, 540.0f, (float)rowH };
            DrawRectangleRec(rB, (Color){
                32,38,52,255
            });
            DrawRectangleLinesEx(rB, 1, GRAY);
            DrawText("B. Kofola & Horalka (Dedinske obcerstvenie)", rB.x+10, rB.y+8, 17, SKYBLUE);
            DrawText("Hlad a smad na 0% | Posilni postavy", rB.x+10, rB.y+30, 13, LIGHTGRAY);
            DrawText("5 EUR", rB.x+10, rB.y+50, 13, GREEN);
            {
                Rectangle b={ rB.x+rB.width-130,rB.y+16,120,38 };
                bool h=CheckCollisionPointRec(mPos,b);
                DrawRectangleRec(b,h?(Color){
                    50,160,50,255
                }
                :(Color){
                    35,120,35,255
                });
                DrawText("KUPIT [E]",b.x+12,b.y+11,14,WHITE);
                if((h&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON))){
                    if(money>=5.f){
                        money-=5.f;
                        hunger=0.f;
                        thirst=0.f;
                        notificationMsg="Kofola a Horalka ta posilnili!";
                        notificationTimer=3.f;
                    }
                    else{
                        notificationMsg="Nemas dostatok penazi!";
                        notificationTimer=2.f;
                    }
                }
            }
            // FARBY BABETTY - 6 stvorcekov
            DrawText("--- FARBY BABETTY ---", col2X+10, startY+(rowH+6)*2+5, 18, GOLD);
            struct PaintOpt {
                const char* name;
                Color col;
                int price;
            };
            PaintOpt paints[6] = {
                {
                    "CERVENA (default)", (Color){
                        204,34,34,255
                    }, 0
                },
                {
                    "MODRA Sapphire", (Color){
                        30,95,230,255
                    }, 120
                },
                {
                    "ZLATA Metallic", (Color){
                        200,160,20,255
                    }, 180
                },
                {
                    "CIERNA Matt", (Color){
                        22,22,25,255
                    }, 150
                },
                {
                    "ZELENA Racing", (Color){
                        35,175,55,255
                    }, 130
                },
                {
                    "BIELA Pearl", (Color){
                        240,240,245,255
                    }, 160
                }
            };
            for (int pi = 0; pi < 6; pi++) {
                int px = col2X + (pi % 3) * 180 + 10;
                int py = startY + (rowH+6)*2 + 32 + (pi/3)*105;
                Rectangle pr = { (float)px, (float)py, 165.0f, 95.0f };
                bool isPicked = (babettaPaintIndex == pi);
                DrawRectangleRec(pr, (Color){
                    32,38,52,255
                });
                DrawRectangleLinesEx(pr, isPicked ? 3 : 1, isPicked ? GOLD : GRAY);
                DrawRectangle(px+8, py+8, 149, 42, paints[pi].col);
                DrawText(paints[pi].name, px+5, py+56, 11, WHITE);
                if (paints[pi].price == 0) {
                    DrawText("ZADARMO", px+8, py+72, 11, GREEN);
                }
                else {
                    DrawText(TextFormat("%d EUR", paints[pi].price), px+8, py+72, 11, GREEN);
                }
                bool hpaint = CheckCollisionPointRec(mPos, pr);
                if (hpaint && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !isPicked) {
                    if (paints[pi].price == 0 || money >= paints[pi].price) {
                        money -= paints[pi].price;
                        babettaPaintIndex = pi;
                        modHasCustomColor = true;
                        modBabettaColor = paints[pi].col;
                        notificationMsg = TextFormat("Babetta prelakirana: %s!", paints[pi].name);
                        notificationTimer = 3.f;
                    }
                    else {
                        notificationMsg = "Nemas dostatok penazi na lakovanie!";
                        notificationTimer = 2.f;
                    }
                }
            }
            Rectangle closeBtnRec = { (float)shopX + (shopW - 240)/2, (float)shopY + shopH - 48, 240, 38 };
            bool hovClose = CheckCollisionPointRec(mPos, closeBtnRec);
            DrawRectangleRec(closeBtnRec, hovClose ? RED : MAROON);
            DrawText("ZATVORIT OBCHOD (ESC)", closeBtnRec.x + 22, closeBtnRec.y + 10, 16, WHITE);
            if (hovClose && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                isShopOpen = false;
                DisableCursor();
            }
        }
        // ============================================
        // MOTO DEALER - Jawa + Babetta farby
        // ============================================
        if (isDealerOpen) {
            DrawRectangle(0, 0, 1280, 720, ColorAlpha(BLACK, 0.75f));
            int dx = 100, dy = 40, dw = 1080, dh = 630;
            DrawRectangle(dx, dy, dw, dh, (Color){
                12, 15, 28, 255
            });
            DrawRectangleLines(dx, dy, dw, dh, (Color){
                80,110,255,255
            });
            DrawRectangle(dx, dy, dw, 52, (Color){
                8, 10, 20, 255
            });
            DrawText("MOTO DEALER  -  JAWA & BABETTA PREDAJNA", dx+25, dy+15, 22, (Color){
                160,190,255,255
            });
            DrawText(TextFormat("Peniaze: %.0f EUR", money), dx+800, dy+16, 20, GREEN);
            Vector2 mPos = GetMousePosition();
            // --- JAWA 350 ---
            Rectangle jawaPan = { (float)dx+20, (float)dy+65, 500.0f, 230.0f };
            DrawRectangleRec(jawaPan, (Color){
                20,22,35,255
            });
            DrawRectangleLinesEx(jawaPan, 2, (Color){
                80,110,255,255
            });
            DrawText("JAWA 350 Californian", jawaPan.x+15, jawaPan.y+12, 20, (Color){
                160,190,255,255
            });
            DrawText("Motor: 350cc OHV 4-takt", jawaPan.x+15, jawaPan.y+45, 14, WHITE);
            DrawText("Max. rychlost: 110 KM/H", jawaPan.x+15, jawaPan.y+66, 14, WHITE);
            DrawText("Palivova nadrz: 14 Litrov", jawaPan.x+15, jawaPan.y+87, 14, WHITE);
            DrawText("Ovladanie rovnake ako Babetta", jawaPan.x+15, jawaPan.y+108, 14, LIGHTGRAY);
            DrawText("Iba 1 kus dostupny!", jawaPan.x+15, jawaPan.y+135, 15, ORANGE);
            DrawText("Cena: 2800 EUR", jawaPan.x+15, jawaPan.y+165, 18, GREEN);
            {
                Rectangle b={ jawaPan.x+jawaPan.width-180,jawaPan.y+jawaPan.height-55,165,42 };
                bool h=CheckCollisionPointRec(mPos,b);
                if(ownsJawa){
                    DrawRectangleRec(b,(Color){
                        20,40,20,255
                    });
                    DrawText("UZ VLASTNIS",b.x+10,b.y+12,14,GREEN);
                }
                else{
                    DrawRectangleRec(b,h?(Color){
                        60,80,200,255
                    }
                    :(Color){
                        40,55,160,255
                    });
                    DrawText("KUPIT JAWU",b.x+12,b.y+12,15,WHITE);
                    if(h&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                        if(money>=2800.f){
                            money-=2800.f;
                            ownsJawa=true;
                            notificationMsg="Kupil si Jawu 350 Californian! (110 KM/H)";
                            notificationTimer=4.f;
                        }
                        else{
                            notificationMsg="Nemas 2800 EUR na Jawu!";
                            notificationTimer=2.5f;
                        }
                    }
                }
            }
            // Ikona Jawy (farbok swatch)
            DrawRectangle(jawaPan.x + jawaPan.width - 180, jawaPan.y + 15, 165, 85, (Color){
                180,20,20,255
            });
            DrawText("JAWA 350", jawaPan.x + jawaPan.width - 170, jawaPan.y + 50, 16, WHITE);
            // --- NOVA BABETTA (ak chce kupit novu) ---
            Rectangle newBabPan = { (float)dx+540, (float)dy+65, 500.0f, 230.0f };
            DrawRectangleRec(newBabPan, (Color){
                20,22,35,255
            });
            DrawRectangleLinesEx(newBabPan, 2, (Color){
                200,160,20,255
            });
            DrawText("BABETTA 210 (Nova - Tovarnske nova)", newBabPan.x+15, newBabPan.y+12, 18, GOLD);
            DrawText("Motor: 50cc 2-takt Minarelli", newBabPan.x+15, newBabPan.y+45, 14, WHITE);
            DrawText("Max. rychlost: 42 KM/H (tovarenska)", newBabPan.x+15, newBabPan.y+66, 14, WHITE);
            DrawText("Palivo: MIX 2T (5L nadrz)", newBabPan.x+15, newBabPan.y+87, 14, WHITE);
            DrawText("Resetuje vsetky diely + tuning", newBabPan.x+15, newBabPan.y+108, 14, LIGHTGRAY);
            DrawText("Cena: 350 EUR", newBabPan.x+15, newBabPan.y+165, 18, GREEN);
            {
                Rectangle b={ newBabPan.x+newBabPan.width-180,newBabPan.y+newBabPan.height-55,165,42 };
                bool h=CheckCollisionPointRec(mPos,b);
                DrawRectangleRec(b,h?(Color){
                    160,120,0,255
                }
                :(Color){
                    120,90,0,255
                });
                DrawText("KUPIT BABETTU",b.x+8,b.y+12,14,WHITE);
                if(h&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                    if(money>=350.f){
                        money-=350.f;
                        hasTuningMotor=false;
                        hasSportCarb=false;
                        hasSportExhaust=false;
                        hasTurboKit=false;
                        hasSportSuspension=false;
                        hasSportWheels=false;
                        hasBigBoreKit=false;
                        hasRaceCDI=false;
                        hasTitaniumExhaust=false;
                        hasRaceClutch=false;
                        hasBremboBrakes=false;
                        hasOhlinsSuspension=false;
                        babettaPaintIndex=0;
                        modHasCustomColor=false;
                        modBabettaColor=(Color){ 204,34,34,255 };
                        fuel=5.f;
                        notificationMsg="Kupil si novu Babettu 210! Vsetky upgrady sa vynulovali.";
                        notificationTimer=4.f;
                    }
                    else{
                        notificationMsg="Nemas 350 EUR na novu Babettu!";
                        notificationTimer=2.5f;
                    }
                }
            }
            // --- JAWA - JAZDIM PREPINANIE ---
            if (ownsJawa) {
                Rectangle jawaSwitchPan = { (float)dx+20, (float)dy+310, 500.0f, 85.0f };
                DrawRectangleRec(jawaSwitchPan, (Color){
                    18,20,30,255
                });
                DrawRectangleLinesEx(jawaSwitchPan, 2, isRidingJawa ? (Color){
                    80,110,255,255
                }
                : GRAY);
                DrawText(isRidingJawa ? "Aktualny motocykel: JAWA 350" : "Aktualny motocykel: BABETTA 210", jawaSwitchPan.x+15, jawaSwitchPan.y+15, 16, isRidingJawa ? (Color){
                    160,190,255,255
                }
                : GOLD);
                Rectangle sb={ jawaSwitchPan.x+jawaSwitchPan.width-180,jawaSwitchPan.y+22,165,42 };
                bool sh=CheckCollisionPointRec(mPos,sb);
                DrawRectangleRec(sb,sh?(Color){
                    50,70,160,255
                }
                :(Color){
                    35,50,120,255
                });
                DrawText(isRidingJawa ? "-> Sadnut BABETTU" : "-> Sadnut JAWU",sb.x+8,sb.y+12,14,WHITE);
                if(sh&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                    isRidingJawa=!isRidingJawa;
                    notificationMsg=isRidingJawa?"Prepnuty na: JAWA 350 (110 KM/H)":"Prepnuty na: BABETTA 210";
                    notificationTimer=3.f;
                }
            }
            // --- STAV GARAZ / UPGRADY ---
            Rectangle statsPan = { (float)dx+20, (float)dy+415, 1040.0f, 165.0f };
            DrawRectangleRec(statsPan, (Color){
                16,20,30,255
            });
            DrawRectangleLinesEx(statsPan, 1, (Color){
                60,70,100,255
            });
            DrawText("STAV VASHO MOTORAVSKEHO PARKU:", statsPan.x+15, statsPan.y+10, 16, GOLD);
            float maxKMH = 42.0f;
            if(hasTuningMotor) maxKMH=65.f;
            if(hasSportCarb) maxKMH+=10.f;
            if(hasSportExhaust) maxKMH+=5.f;
            if(hasTurboKit) maxKMH+=10.f;
            if(hasSportWheels) maxKMH+=3.f;
            if(hasBigBoreKit) maxKMH+=20.f;
            if(hasRaceCDI) maxKMH+=8.f;
            if(hasTitaniumExhaust) maxKMH+=10.f;
            if(isRidingJawa) maxKMH=110.f;
            DrawText(TextFormat("Max. rychlost Babetty: %.0f KM/H", maxKMH), statsPan.x+15, statsPan.y+38, 15, WHITE);
            DrawText(TextFormat("Motor: %s", hasTuningMotor?"70cc TUNING":"50cc STANDARD"), statsPan.x+15, statsPan.y+60, 14, hasTuningMotor?GOLD:LIGHTGRAY);
            DrawText(TextFormat("Turbo: %s", hasTurboKit?"INSTALOVANY":"NIE"), statsPan.x+15, statsPan.y+78, 14, hasTurboKit?ORANGE:LIGHTGRAY);
            DrawText(TextFormat("Kolesa: %s", hasSportWheels?"PIRELLI 17\"":"STANDARD"), statsPan.x+15, statsPan.y+96, 14, hasSportWheels?(Color){
                255,210,80,255
            }
            :LIGHTGRAY);
            DrawText(TextFormat("Suspenzia: %s", hasSportSuspension?"MARZOCCHI SPORT":"STANDARD"), statsPan.x+300, statsPan.y+60, 14, hasSportSuspension?(Color){
                180,100,255,255
            }
            :LIGHTGRAY);
            DrawText(TextFormat("Vyfuk: %s", hasSportExhaust?"GIANNELLI SPORT":"STANDARD"), statsPan.x+300, statsPan.y+78, 14, hasSportExhaust?(Color){
                100,180,255,255
            }
            :LIGHTGRAY);
            DrawText(TextFormat("LED Svetla: %s", hasHeadlightKit?"INSTALOVANE":"STANDARD"), statsPan.x+300, statsPan.y+96, 14, hasHeadlightKit?(Color){
                80,220,255,255
            }
            :LIGHTGRAY);
            DrawText(TextFormat("Jawa 350: %s", ownsJawa?"VLASTNIS":"NEVLASTNIS"), statsPan.x+700, statsPan.y+60, 14, ownsJawa?(Color){
                160,190,255,255
            }
            :LIGHTGRAY);
            DrawText(TextFormat("Farba: Babetta %s", babettaPaintIndex==0?"CERVENA":babettaPaintIndex==1?"MODRA":babettaPaintIndex==2?"ZLATA":babettaPaintIndex==3?"CIERNA":babettaPaintIndex==4?"ZELENA":"BIELA"), statsPan.x+700, statsPan.y+78, 14, WHITE);
            Rectangle dCloseBtnRec = { (float)dx + (dw - 240)/2, (float)dy + dh - 48, 240, 38 };
            bool dHovClose = CheckCollisionPointRec(mPos, dCloseBtnRec);
            DrawRectangleRec(dCloseBtnRec, dHovClose ? RED : MAROON);
            DrawText("ZATVORIT DEALER (ESC)", dCloseBtnRec.x + 22, dCloseBtnRec.y + 10, 16, WHITE);
            if (dHovClose && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                isDealerOpen = false;
                DisableCursor();
            }
        }
        // ============================================
        // AUTOSERVIS - PREMIUM UPGRADY (drahsie, ale najlepsie)
        // ============================================
        if (isMechanicOpen) {
            DrawRectangle(0, 0, 1280, 720, ColorAlpha(BLACK, 0.78f));
            int mx = 60, my = 25, mw = 1160, mh = 670;
            DrawRectangle(mx, my, mw, mh, (Color){
                30, 22, 14, 255
            });
            DrawRectangleLines(mx, my, mw, mh, (Color){
                255, 160, 50, 255
            });
            DrawRectangle(mx, my, mw, 52, (Color){
                18, 12, 7, 255
            });
            DrawText("AUTOSERVIS  -  MAJSTER FERKO (PREMIUM UPGRADY)", mx + 25, my + 15, 22, (Color){
                255, 175, 70, 255
            });
            DrawText(TextFormat("Peniaze: %.0f EUR", money), mx + 880, my + 16, 20, GREEN);
            DrawText("Drahsie diely, ale Babetta pojde ako nikdy predtym.", mx + 25, my + 60, 15, LIGHTGRAY);
            Vector2 mp = GetMousePosition();
            struct MechItem {
                const char* name;
                const char* desc;
                const char* effect;
                float price;
                bool* owned;
                Color color;
            };
            MechItem items[6] = {
                { "1. Big Bore Kit 90cc", "Kompletny valec, piest a hlava", "+20 KM/H | Brutalny krutak", 850.0f, &hasBigBoreKit, ORANGE },
                {
                    "2. Race CDI (Unlimited RPM)", "Zruseny obmedzovac otacok", "+8 KM/H | Vyssi vykon", 600.0f, &hasRaceCDI, (Color){
                        255, 90, 90, 255
                    }
                },
                {
                    "3. Titanium Racing Vyfuk", "Full titanium, rovny prietok", "+10 KM/H | Agresivny zvuk", 750.0f, &hasTitaniumExhaust, (Color){
                        185, 205, 235, 255
                    }
                },
                {
                    "4. Race Spojka + Prevodovka", "Rychlejsi rozjazd a radenie", "Rychlejsi rozjazd", 650.0f, &hasRaceClutch, (Color){
                        120, 255, 160, 255
                    }
                },
                {
                    "5. Brembo Racing Brzdy", "Dvoj kotucove racing brzdy", "Ovela silnejsie brzdenie", 700.0f, &hasBremboBrakes, (Color){
                        255, 70, 70, 255
                    }
                },
                {
                    "6. Ohlins Racing Podvozok", "Stelovatelne racing tlmice", "Lepsie zatacanie", 900.0f, &hasOhlinsSuspension, (Color){
                        255, 215, 80, 255
                    }
                }
            };
            for (int i = 0; i < 6; i++) {
                int col = i % 2, row = i / 2;
                float px = (float)(mx + 20 + col * 560);
                float py = (float)(my + 95 + row * 172);
                Rectangle panel = { px, py, 550.0f, 160.0f };
                bool owned = *items[i].owned;
                DrawRectangleRec(panel, owned ? (Color){
                    26, 48, 26, 255
                }
                : (Color){
                    42, 34, 24, 255
                });
                DrawRectangleLinesEx(panel, 2, owned ? GREEN : (Color){
                    120, 90, 50, 255
                });
                DrawText(items[i].name, (int)px + 14, (int)py + 10, 18, items[i].color);
                DrawText(items[i].desc, (int)px + 14, (int)py + 40, 13, LIGHTGRAY);
                DrawText(items[i].effect, (int)px + 14, (int)py + 62, 13, (Color){
                    220, 200, 150, 255
                });
                DrawText(TextFormat("%.0f EUR", items[i].price), (int)px + 14, (int)py + 118, 18, GREEN);
                Rectangle b = { px + panel.width - 175.0f, py + panel.height - 52.0f, 160.0f, 40.0f };
                bool h = CheckCollisionPointRec(mp, b);
                if (owned) {
                    DrawRectangleRec(b, DARKGRAY);
                    DrawText("KUPENE", (int)b.x + 36, (int)b.y + 11, 15, GREEN);
                }
                else {
                    DrawRectangleRec(b, h ? (Color){
                        210, 130, 30, 255
                    }
                    : (Color){
                        160, 95, 25, 255
                    });
                    DrawText("KUPIT", (int)b.x + 46, (int)b.y + 11, 15, WHITE);
                    if ((h && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) || IsKeyPressed(KEY_ONE + i)) {
                        if (money >= items[i].price) {
                            money -= items[i].price;
                            *items[i].owned = true;
                            notificationMsg = TextFormat("Kupil si: %s", items[i].name);
                            notificationTimer = 3.5f;
                        }
                        else {
                            notificationMsg = "Nemas dostatok penazi na tento diel!";
                            notificationTimer = 2.0f;
                        }
                    }
                }
            }
            Rectangle mClose = { (float)mx + (mw - 240) / 2, (float)my + mh - 50, 240, 38 };
            bool mHov = CheckCollisionPointRec(mp, mClose);
            DrawRectangleRec(mClose, mHov ? RED : MAROON);
            DrawText("ZATVORIT (ESC)", (int)mClose.x + 46, (int)mClose.y + 10, 16, WHITE);
            if (mHov && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                isMechanicOpen = false;
                DisableCursor();
            }
        }
        // ============================================
        // RESTAURACIA - SLOVENSKE JEDLA
        // ============================================
        if (isRestaurantOpen) {
            DrawRectangle(0, 0, 1280, 720, ColorAlpha(BLACK, 0.78f));
            int rx = 60, ry = 25, rw = 1160, rh = 670;
            DrawRectangle(rx, ry, rw, rh, (Color){
                32, 26, 18, 255
            });
            DrawRectangleLines(rx, ry, rw, rh, (Color){
                240, 200, 60, 255
            });
            DrawRectangle(rx, ry, rw, 52, (Color){
                20, 16, 10, 255
            });
            DrawText("RESTAURACIA  -  U BABETTY (SLOVENSKE JEDLA)", rx + 25, ry + 15, 22, (Color){
                255, 220, 120, 255
            });
            DrawText(TextFormat("Peniaze: %.0f EUR", money), rx + 880, ry + 16, 20, GREEN);
            DrawText(TextFormat("Hlad: %.0f%%   |   Smad: %.0f%%", hunger * 100.0f, thirst * 100.0f), rx + 25, ry + 62, 16, ORANGE);
            Vector2 rp = GetMousePosition();
            struct Dish {
                const char* name;
                const char* desc;
                float price;
                float hungerRelief;
                float thirstRelief;
                Color color;
            };
            Dish dishes[6] = {
                {
                    "1. Rezen so zemiakmi a citronom", "Domaci rezen, opekcane zemiaky, citron", 12.0f, 0.50f, 0.20f, (Color){
                        255, 205, 120, 255
                    }
                },
                {
                    "2. Bryndzove halusky", "Tradicne halušky s bryndzou a slaninou", 9.0f, 0.55f, 0.10f, (Color){
                        240, 240, 200, 255
                    }
                },
                {
                    "3. Gulas", "Hovadzi gulas s knedlou a chlebom", 14.0f, 0.60f, 0.25f, (Color){
                        220, 130, 60, 255
                    }
                },
                {
                    "4. Kapustnica", "Kysla kapusta, klobasa a huby", 8.0f, 0.40f, 0.30f, (Color){
                        180, 200, 120, 255
                    }
                },
                {
                    "5. Vyprazany syr so zemiakmi", "Eidam v trojobale, zemiaky, tatarska omacka", 10.0f, 0.45f, 0.15f, (Color){
                        255, 220, 80, 255
                    }
                },
                {
                    "6. Kofola 0.5L", "Dedinsky napoj, poriadne schladi", 3.0f, 0.0f, 0.60f, (Color){
                        140, 90, 50, 255
                    }
                }
            };
            for (int i = 0; i < 6; i++) {
                int col = i % 2, row = i / 2;
                float px = (float)(rx + 20 + col * 560);
                float py = (float)(ry + 95 + row * 172);
                Rectangle panel = { px, py, 550.0f, 160.0f };
                DrawRectangleRec(panel, (Color){
                    46, 36, 22, 255
                });
                DrawRectangleLinesEx(panel, 2, (Color){
                    150, 115, 45, 255
                });
                DrawText(dishes[i].name, (int)px + 14, (int)py + 10, 18, dishes[i].color);
                DrawText(dishes[i].desc, (int)px + 14, (int)py + 40, 13, LIGHTGRAY);
                if (dishes[i].hungerRelief > 0.0f)                        DrawText(TextFormat("Hlad -%.0f%% | Smad -%.0f%%", dishes[i].hungerRelief * 100.0f, dishes[i].thirstRelief * 100.0f), (int)px + 14, (int)py + 62, 13, (Color){
                    220, 200, 150, 255
                });
                else                        DrawText(TextFormat("Smad -%.0f%%", dishes[i].thirstRelief * 100.0f), (int)px + 14, (int)py + 62, 13, (Color){
                    220, 200, 150, 255
                });
                DrawText(TextFormat("%.0f EUR", dishes[i].price), (int)px + 14, (int)py + 118, 18, GREEN);
                Rectangle b = { px + panel.width - 175.0f, py + panel.height - 52.0f, 160.0f, 40.0f };
                bool h = CheckCollisionPointRec(rp, b);
                DrawRectangleRec(b, h ? (Color){
                    200, 150, 30, 255
                }
                : (Color){
                    150, 110, 25, 255
                });
                DrawText("OBJEDNAT", (int)b.x + 34, (int)b.y + 11, 15, WHITE);
                if ((h && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) || IsKeyPressed(KEY_ONE + i)) {
                    if (money >= dishes[i].price) {
                        money -= dishes[i].price;
                        hunger = (hunger - dishes[i].hungerRelief > 0.0f) ? hunger - dishes[i].hungerRelief : 0.0f;
                        thirst = (thirst - dishes[i].thirstRelief > 0.0f) ? thirst - dishes[i].thirstRelief : 0.0f;
                        notificationMsg = TextFormat("Zjedol si: %s", dishes[i].name);
                        notificationTimer = 3.5f;
                    }
                    else {
                        notificationMsg = "Nemas dostatok penazi na toto jedlo!";
                        notificationTimer = 2.0f;
                    }
                }
            }
            Rectangle rClose = { (float)rx + (rw - 240) / 2, (float)ry + rh - 50, 240, 38 };
            bool rHov = CheckCollisionPointRec(rp, rClose);
            DrawRectangleRec(rClose, rHov ? RED : MAROON);
            DrawText("ZATVORIT (ESC)", (int)rClose.x + 46, (int)rClose.y + 10, 16, WHITE);
            if (rHov && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                isRestaurantOpen = false;
                DisableCursor();
            }
        }
        // ============================================
        // PIZZERIA LUCIA - PREDAJ PIZZE
        // ============================================
        if (isPizzeriaOpen) {
            DrawRectangle(0, 0, 1280, 720, ColorAlpha(BLACK, 0.78f));
            int mx = 90, my = 40, mw = 1100, mh = 640;
            DrawRectangle(mx, my, mw, mh, (Color){ 40, 22, 16, 255 });
            DrawRectangleLines(mx, my, mw, mh, (Color){ 235, 185, 45, 255 });
            DrawRectangle(mx, my, mw, 56, (Color){ 26, 12, 8, 255 });
            DrawText("REAL SVINNA PIZZERIA LUCIA", mx + 25, my + 16, 24, (Color){ 255, 210, 120, 255 });
            DrawText(TextFormat("Peniaze: %.0f EUR", money), mx + 830, my + 18, 20, GREEN);
            DrawText(TextFormat("Hlad: %.0f%%   |   Dovezene pizze: %d", hunger * 100.0f, pizzaJobsDone), mx + 25, my + 70, 16, ORANGE);
            Vector2 mp = GetMousePosition();
            struct PizzaItem {
                const char* name;
                const char* desc;
                float price;
                float hungerRelief;
                Color color;
            };
            PizzaItem pizzaMenu[5] = {
                { "1. Pizza Margherita", "Paradajkova omacka, mozzarella, bazalka", 8.0f, 0.45f, (Color){ 235, 120, 90, 255 } },
                { "2. Pizza Sunkova", "Sunka, syr, kukurica, oregano", 10.0f, 0.55f, (Color){ 240, 165, 120, 255 } },
                { "3. Pizza Syrova (4 druhy)", "Eidam, niva, hermelin, parmezan", 11.0f, 0.60f, (Color){ 245, 225, 150, 255 } },
                { "4. Pizza Lucia Special", "Salama, slanina, cibula, feferonky", 14.0f, 0.70f, (Color){ 230, 100, 70, 255 } },
                { "5. Pizza rodinna (XL)", "Velka pizza pre celu rodinu", 18.0f, 0.85f, (Color){ 255, 195, 100, 255 } }
            };
            for (int i = 0; i < 5; i++) {
                int col = i % 2, row = i / 2;
                float px = (float)(mx + 20 + col * 535);
                float py = (float)(my + 105 + row * 160);
                Rectangle panel = { px, py, 525.0f, 148.0f };
                DrawRectangleRec(panel, (Color){ 56, 30, 22, 255 });
                DrawRectangleLinesEx(panel, 2, (Color){ 155, 105, 45, 255 });
                DrawText(pizzaMenu[i].name, (int)px + 14, (int)py + 10, 18, pizzaMenu[i].color);
                DrawText(pizzaMenu[i].desc, (int)px + 14, (int)py + 40, 13, LIGHTGRAY);
                DrawText(TextFormat("Hlad -%.0f%%", pizzaMenu[i].hungerRelief * 100.0f), (int)px + 14, (int)py + 64, 13, (Color){ 220, 200, 150, 255 });
                DrawText(TextFormat("%.0f EUR", pizzaMenu[i].price), (int)px + 14, (int)py + 110, 18, GREEN);
                Rectangle b = { px + panel.width - 165.0f, py + panel.height - 50.0f, 150.0f, 38.0f };
                bool h = CheckCollisionPointRec(mp, b);
                DrawRectangleRec(b, h ? (Color){ 210, 140, 30, 255 } : (Color){ 150, 95, 22, 255 });
                DrawText("KUPIT", (int)b.x + 42, (int)b.y + 10, 15, WHITE);
                if ((h && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) || IsKeyPressed(KEY_ONE + i)) {
                    if (money >= pizzaMenu[i].price) {
                        money -= pizzaMenu[i].price;
                        hunger = (hunger - pizzaMenu[i].hungerRelief > 0.0f) ? hunger - pizzaMenu[i].hungerRelief : 0.0f;
                        notificationMsg = TextFormat("Kupil si: %s", pizzaMenu[i].name);
                        notificationTimer = 3.5f;
                    }
                    else {
                        notificationMsg = "Nemas dostatok penazi na tuto pizzu!";
                        notificationTimer = 2.0f;
                    }
                }
            }
            // Info panel vpravo dole (volne miesto v mriezke)
            Rectangle info = { (float)(mx + 20 + 535), (float)(my + 105 + 2 * 160), 525.0f, 148.0f };
            DrawRectangleRec(info, (Color){ 34, 20, 16, 255 });
            DrawRectangleLinesEx(info, 2, (Color){ 110, 75, 35, 255 });
            DrawText("DOVOZ PIZZE", (int)info.x + 14, (int)info.y + 12, 18, (Color){ 255, 210, 120, 255 });
            DrawText("Chces zarobit? Chod ZA budovu k zadnemu", (int)info.x + 14, (int)info.y + 42, 13, LIGHTGRAY);
            DrawText("pultu a stlac F. Odmena za dovoz je", (int)info.x + 14, (int)info.y + 62, 13, LIGHTGRAY);
            DrawText(TextFormat("%.0f EUR za jednu pizzu.", modPizzaPay), (int)info.x + 14, (int)info.y + 82, 13, GOLD);
            Rectangle pClose = { (float)mx + (mw - 240) / 2, (float)my + mh - 46, 240, 36 };
            bool pCloseHover = CheckCollisionPointRec(mp, pClose);
            DrawRectangleRec(pClose, pCloseHover ? RED : MAROON);
            DrawText("ZATVORIT (ESC)", (int)pClose.x + 46, (int)pClose.y + 9, 16, WHITE);
            if (pCloseHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                isPizzeriaOpen = false;
                DisableCursor();
            }
        }
        // ============================================
        // OPRAVOVNA BABETT - 2D MINIHRA
        // ============================================
        if (isRepairOpen) {
            DrawRectangle(0, 0, 1280, 720, ColorAlpha(BLACK, 0.82f));
            int px = 60, py = 110, pw = 740, ph = 520;
            DrawRectangle(px, py, pw, ph, (Color){
                26, 30, 36, 255
            });
            DrawRectangleLinesEx((Rectangle){
                (float)px, (float)py, (float)pw, (float)ph
            }, 2, (Color){
                238, 148, 32, 255
            });
            DrawText("OPRAVOVNA BABETT  -  MINIHRA", px + 20, py + 14, 22, (Color){
                255, 175, 70, 255
            });
            DrawText("Pretiahni diely z praveho panelu na spravne miesto na Babete.", px + 20, py + 44, 15, LIGHTGRAY);
            DrawText(TextFormat("Opravene Babetty: %i", repairDone), px + pw - 230, py + 18, 16, GOLD);
            // Podlaha / zem
            DrawLineEx({
                (float)px + 30.0f, (float)py + 380.0f
            },
            { (float)px + pw - 30.0f, (float)py + 380.0f }, 3.0f, (Color){
                95, 90, 80, 255
            });
            // --- Volne sloty (kym tam diel nie je) ---
            if (!repairPlaced[0]) {
                DrawCircleV(repairSlot[0], 47.0f, ColorAlpha(BLACK, 0.25f));
                DrawCircleLinesV(repairSlot[0], 47.0f, (Color){
                    160, 160, 170, 200
                });
                DrawText("ZADNE", 288, 384, 14, (Color){
                    190, 190, 190, 255
                });
            }
            if (!repairPlaced[1]) {
                DrawCircleV(repairSlot[1], 47.0f, ColorAlpha(BLACK, 0.25f));
                DrawCircleLinesV(repairSlot[1], 47.0f, (Color){
                    160, 160, 170, 200
                });
                DrawText("PREDNE", 522, 384, 14, (Color){
                    190, 190, 190, 255
                });
            }
            if (!repairPlaced[2]) {
                Rectangle exSlot = { repairSlot[2].x - 85.0f, repairSlot[2].y - 16.0f, 170.0f, 32.0f };
                DrawRectangleRec(exSlot, ColorAlpha(BLACK, 0.25f));
                DrawRectangleLinesEx(exSlot, 2, (Color){
                    160, 160, 170, 200
                });
                DrawText("VYFUK", 414, 480, 14, (Color){
                    190, 190, 190, 255
                });
            }
            if (!repairPlaced[3]) {
                DrawCircleV(repairSlot[3], 21.0f, ColorAlpha(BLACK, 0.25f));
                DrawCircleLinesV(repairSlot[3], 21.0f, (Color){
                    160, 160, 170, 200
                });
                DrawText("SVIECKA", 468, 333, 14, (Color){
                    190, 190, 190, 255
                });
            }
            // --- 2D nakres Babetty ---
            Color frameCol = (Color){ 204, 60, 50, 255 };
            Color chrome = (Color){ 205, 208, 218, 255 };
            // ram + vidlica + riadidla
            DrawLineEx({
                330, 430
            },
            { 400, 350 }, 7.0f, frameCol);
            DrawLineEx({
                400, 350
            },
            { 560, 430 }, 7.0f, frameCol);
            DrawLineEx({
                400, 350
            },
            { 348, 331 }, 7.0f, frameCol);
            DrawLineEx({
                560, 430
            },
            { 566, 306 }, 5.0f, chrome);
            DrawLineEx({
                545, 302
            },
            { 590, 300 }, 5.0f, chrome);
            DrawCircleV({
                578, 300
            }, 12.0f, YELLOW);
            // nadrz a sedadlo
            DrawRectangleRounded((Rectangle){
                400, 292, 115, 42
            }, 0.45f, 10, frameCol);
            DrawRectangleRounded((Rectangle){
                400, 292, 115, 12
            }, 0.55f, 8, (Color){
                235, 95, 85, 255
            });
            DrawRectangleRounded((Rectangle){
                342, 318, 90, 24
            }, 0.6f, 8, BLACK);
            DrawRectangleRounded((Rectangle){
                342, 314, 90, 12
            }, 0.6f, 8, (Color){
                55, 40, 30, 255
            });
            // motor a valec
            DrawRectangleRounded((Rectangle){
                405, 372, 80, 46
            }, 0.15f, 4, (Color){
                75, 75, 82, 255
            });
            DrawRectangleRounded((Rectangle){
                425, 352, 34, 22
            }, 0.2f, 4, (Color){
                105, 105, 112, 255
            });
            // blatniky
            DrawLineEx({
                285, 402
            },
            { 330, 386 }, 4.0f, frameCol);
            DrawLineEx({
                375, 402
            },
            { 330, 386 }, 4.0f, frameCol);
            DrawLineEx({
                515, 402
            },
            { 560, 386 }, 4.0f, frameCol);
            DrawLineEx({
                605, 402
            },
            { 560, 386 }, 4.0f, frameCol);
            // osadene diely
            for (int i = 0; i < 4; i++) {
                if (repairPlaced[i]) DrawRepairPartIcon(i, repairSlot[i], 1.0f, false);
            }
            // --- Pravy panel: tray s dielmi ---
            const char* repNames[4] = { "Zadne koleso", "Predne koleso", "Vyfuk", "Sviecka" };
            const char* repDesc[4] = { "Pneu 16\" - zadna vidlica", "Pneu 16\" - predna vidlica", "Chromovy vyfuk", "Zapalovacia sviecka" };
            int tx = 820, ty = 110, tw = 400, th = 520;
            DrawRectangle(tx, ty, tw, th, (Color){
                22, 26, 32, 255
            });
            DrawRectangleLinesEx((Rectangle){
                (float)tx, (float)ty, (float)tw, (float)th
            }, 2, (Color){
                120, 125, 135, 255
            });
            DrawText("DIELY NA OPRAVU", tx + 20, ty + 16, 20, GOLD);
            DrawText("Chyt a pretiahni na Babetu", tx + 20, ty + 44, 14, LIGHTGRAY);
            for (int i = 0; i < 4; i++) {
                Rectangle card = { (float)(tx + 20), (float)(ty + 60 + i * 105), (float)(tw - 40), 90.0f };
                bool placed = repairPlaced[i];
                bool hover = CheckCollisionPointRec(GetMousePosition(), card);
                DrawRectangleRec(card, placed ? (Color){
                    28, 48, 30, 255
                }
                : (hover ? (Color){
                    50, 42, 32, 255
                }
                : (Color){
                    36, 40, 48, 255
                }));
                DrawRectangleLinesEx(card, placed ? 2 : 1, placed ? GREEN : (hover ? (Color){
                    238, 148, 32, 255
                }
                : (Color){
                    80, 85, 95, 255
                }));
                if (!placed && repairDrag != i) DrawRepairPartIcon(i, repairHome[i], 0.9f, false);
                DrawText(repNames[i], (int)card.x + 105, (int)card.y + 16, 17, placed ? GREEN : WHITE);
                DrawText(repDesc[i], (int)card.x + 105, (int)card.y + 40, 12, LIGHTGRAY);
                if (placed) DrawText("OSADENE", (int)card.x + 105, (int)card.y + 62, 13, GREEN);
            }
            // --- Logika drag & drop ---
            Vector2 rmouse = GetMousePosition();
            if (repairMsgTimer > 0.0f) repairMsgTimer -= GetFrameTime();
            if (!repairFinished) {
                if (repairDrag == -1 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    for (int i = 0; i < 4; i++) {
                        Rectangle card = { (float)(tx + 20), (float)(ty + 60 + i * 105), (float)(tw - 40), 90.0f };
                        if (!repairPlaced[i] && CheckCollisionPointRec(rmouse, card)) {
                            repairDrag = i;
                            repairPartPos[i] = rmouse;
                            break;
                        }
                    }
                }
                if (repairDrag != -1) {
                    repairPartPos[repairDrag] = rmouse;
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                        int i = repairDrag;
                        repairDrag = -1;
                        bool ok = false;
                        if (i == 2) {
                            Rectangle exSlot = { repairSlot[2].x - 85.0f, repairSlot[2].y - 16.0f, 170.0f, 32.0f };
                            ok = CheckCollisionPointRec(rmouse, exSlot);
                        }
                        else {
                            ok = (Vector2Distance(rmouse, repairSlot[i]) < 58.0f);
                        }
                        if (ok) {
                            repairPlaced[i] = true;
                            repairPartPos[i] = repairSlot[i];
                            repairMsg = "Diel je na svojom mieste!";
                            repairMsgTimer = 1.5f;
                            bool all = true;
                            for (int k = 0; k < 4; k++) if (!repairPlaced[k]) all = false;
                            if (all) {
                                repairFinished = true;
                                repairDone++;
                                money += modRepairPay;
                                repairMsg = TextFormat("Babetta opravena! Zarobene: %.0f EUR", modRepairPay);
                                repairMsgTimer = 5.0f;
                            }
                        }
                        else {
                            repairPartPos[i] = repairHome[i];
                            repairMsg = "Zly diel alebo zle miesto! Skus to znova.";
                            repairMsgTimer = 2.0f;
                        }
                    }
                }
            }
            // Ikona prave tahaneho dielu
            if (repairDrag != -1) DrawRepairPartIcon(repairDrag, repairPartPos[repairDrag], 1.0f, false);
            // Sprava / stav
            if (!repairMsg.empty() && repairMsgTimer > 0.0f) {
                DrawRectangle(px + 40, py + ph - 46, pw - 80, 32, ColorAlpha((Color){
                    10, 40, 15, 230
                }, 0.9f));
                DrawText(repairMsg.c_str(), px + 54, py + ph - 40, 15, (Color){
                    160, 255, 170, 255
                });
            }
            // Hotovo - panel
            if (repairFinished) {
                DrawRectangle(px + 120, py + 130, pw - 240, 230, ColorAlpha((Color){
                    12, 38, 16, 245
                }, 0.96f));
                DrawRectangleLinesEx((Rectangle){
                    (float)(px + 120), (float)(py + 130), (float)(pw - 240), 230
                }, 3, GREEN);
                DrawText("BABETTA OPRAVENA!", px + 225, py + 175, 32, (Color){
                    120, 255, 140, 255
                });
                DrawText(TextFormat("Zakaznik zaplatil: %.0f EUR", modRepairPay), px + 255, py + 225, 20, GOLD);
                DrawText(TextFormat("Celkovo opravene: %i Babetty", repairDone), px + 250, py + 260, 17, WHITE);
                DrawText("Dalsia Babetta caka v rade...", px + 245, py + 300, 16, LIGHTGRAY);
            }
            // Tlacidla dole
            if (repairFinished) {
                Rectangle nb = { (float)(px + 150), 648.0f, 280.0f, 44.0f };
                bool nh = CheckCollisionPointRec(rmouse, nb);
                DrawRectangleRec(nb, nh ? (Color){
                    50, 150, 60, 255
                }
                : (Color){
                    35, 110, 45, 255
                });
                DrawRectangleLinesEx(nb, 1, GREEN);
                DrawText("DALSIA OPRAVA [R]", (int)nb.x + 42, (int)nb.y + 13, 17, WHITE);
                if ((nh && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) || IsKeyPressed(KEY_R)) {
                    for (int i = 0; i < 4; i++) {
                        repairPlaced[i] = false;
                        repairPartPos[i] = repairHome[i];
                    }
                    repairFinished = false;
                    repairDrag = -1;
                    repairMsg = "Nova Babetta caka na opravu.";
                    repairMsgTimer = 2.5f;
                }
            }
            Rectangle rpClose = { (float)(px + pw - 270), 648.0f, 240.0f, 44.0f };
            bool rpHov = CheckCollisionPointRec(rmouse, rpClose);
            DrawRectangleRec(rpClose, rpHov ? RED : MAROON);
            DrawText("ZATVORIT (ESC)", (int)rpClose.x + 46, (int)rpClose.y + 13, 16, WHITE);
            if (rpHov && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                isRepairOpen = false;
                repairDrag = -1;
                DisableCursor();
            }
        }
        if (isPaused) {
            DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.7f));
            DrawText("PAUZA", 580, 320, 40, WHITE);
            DrawText("Version: 0.4.8DEV || ModLoader: 0.1.8ADEV", 530, 100, 30, RED);
        }
        EndDrawing();
    }
    if (ambient.stream.buffer != nullptr) UnloadMusicStream(ambient);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
