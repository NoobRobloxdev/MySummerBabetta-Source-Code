# 🛵 My Summer Babetta (ALPHA) - v0.4.8ADEV1

A rural Slovak simulator built with C++ and Raylib. Build, maintain, tune and ride your legendary Babetta through a village while managing your survival and money.

> [!CAUTION]
> **COPYRIGHT NOTICE:** This project is built by LOM_Noob and Jurmat, also with ??. Do not republish this code on GitHub or any other platform.

🛑🛑🛑🛑
- 🛑 **MODLOADER ADDED.** Mods go into the `Mods (TESTING)` folder. To make a mod, read `HLAVNE_PRECITAT_PRE_DAVANIE_MODOV.txt` (in Slovak, English will be released soon).
- 🛑 **PLEASE READ `PRECITAJMA.txt`** (in Slovak, English will be released soon).
🛑🛑🛑🛑

---

## 🛠 Features

### Modular Assembly System
The Babetta is not just a vehicle, it's a project. It will not move unless you have manually installed:
- The Engine ⚙️
- Front Tire 🛞
- Rear Tire 🛞

### Detailed World
- **The Garage:** 4-wall structure with shelves for part storage.
- **Solid Collisions:** All 30 houses and buildings are physical objects with driveways. You can walk into the garage, but you can't walk through walls.
- **Village streets:** main cross street, east side street, new-quarter street and a northern connector, with houses on both sides.
- **Optimized wheat field:** the whole field is baked into a single mesh and drawn with one draw call (big FPS boost).

### Places to visit
| Place | What you can do |
|---|---|
| **Car Service (Autoservis)** | Premium Babetta tuning GUI (open with `F` / LMB at the counter) |
| **Restaurant** (next to the gas station) | Buy Slovak food and drinks to restore hunger and thirst |
| **Babetta Repair Shop** | Work as a mechanic and play the "Assemble the Babetta" minigame |
| **Pizzeria Lucia** | Buy pizza at the front, deliver pizza as a job from the back |
| **Moto Dealer** | Buy a new Babetta, showroom with 2x Jawa and 2x Babetta |
| **Gas Station, Farm** | Fuel, delivery targets |

### 🔧 Premium Tuning (Car Service)
| Part | Price | Effect |
|---|---|---|
| Big Bore Kit 90cc | 850 EUR | +20 km/h |
| Race CDI | 600 EUR | +8 km/h |
| Titanium Racing Exhaust | 750 EUR | +10 km/h |
| Race Clutch | 650 EUR | +35% acceleration |
| Brembo Brakes | 700 EUR | stronger braking |
| Ohlins Suspension | 900 EUR | +25% cornering |

Buying a new Babetta at the dealer resets all premium parts.

### 🍽 Restaurant Menu
| Item | Price |
|---|---|
| Schnitzel with potatoes and lemon | 12 EUR |
| Bryndzove halusky | 9 EUR |
| Goulash | 14 EUR |
| Sauerkraut soup (Kapustnica) | 8 EUR |
| Fried cheese with potatoes | 10 EUR |
| Kofola 0.5L | 3 EUR |

The menu shows your current Hunger / Thirst in percent.

### 🔩 Repair Shop - Mechanic Job
Work at the repair counter and play the 2D minigame **"Assemble the Babetta"**:
- Drag 4 parts from the tray (rear wheel, front wheel, exhaust, spark plug) onto the correct slot of the schematic Babetta.
- Correct = snaps in, wrong = returns to the tray.
- When all 4 parts are placed, the bike is repaired and you get paid (default 60 EUR). Press `R` for the next repair.
- A counter tracks how many Babettas you have repaired.

### 🍕 Pizzeria Lucia - "Real Svinna Pizzeria Lucia"
**Front (shop):**
| Pizza | Price |
|---|---|
| Margherita | 8 EUR |
| Ham | 10 EUR |
| Four Cheese | 11 EUR |
| Lucia Special | 14 EUR |
| Family (XL) | 18 EUR |

**Back (delivery job):** start the pizza delivery at the back counter, get a random target (Farm, Gas Station, Moto Dealer or Repair Shop), ride there and press `F` to get paid (default 55 EUR). A 3D arrow marker and a HUD panel show your destination. The back has its own entrance and a parking lot.

### 💼 Jobs & Earning Money
- Economic System: start with **2000 EUR**.
- Generic delivery job (enable with `ENABLE_JOBS=1`): pick up at the delivery point next to the gas station, press `F` to start, finish within the time limit (default 10 min) and earn 50 EUR.
- Survival Mechanics: real-time Hunger and Thirst tracking. (WIP, still buggy)

---

## 🎮 Controls

| Key | Action |
|---|---|
| `W` / `A` / `S` / `D` | Walk / Drive Babetta |
| Mouse | Look around |
| Left Click | Pick up part / Install part / Pull part off bike |
| `F` / Left Click at a counter | Open shop / restaurant / tuning / start job |
| `E` | Mount or Dismount Babetta |
| `R` | Next repair (repair minigame) |
| `ESC` | Toggle Pause Menu / Show Cursor |
| `F6` | Toggle FPS Counter |
| `F9` | Cheat: Instant 999,999 EUR |
| `F12` | **NEW:** Screenshot (APR/17/2026) |

---

## 🧩 Modding

Put `.cfg` or `.txt` files into the mods folder. Supported options:

| Option | Aliases | Default | Description |
|---|---|---|---|
| `REPAIR_PAY=100` | `SERVIS_PAY`, `OPRAVA_PAY` | 60 | Reward for a repair |
| `PIZZA_PAY=80` | `PIZZA_JOB_PAY`, `DOVOZ_PAY` | 55 | Reward for a pizza delivery |
| `ENABLE_JOBS=1` | `JOBS` | off | Enable the generic delivery job |
| `JOB_PAY=50` | | 50 | EUR per job |
| `JOB_TIME=600` | | 600 | Seconds to finish the job |
| `ENABLE_CUSTOM_MUSIC=1` | | off | Enable custom music |
| `CUSTOM_MUSIC_PATH=Sounds/song.mp3` | | | Path to your music (.mp3, .ogg, .wav). If missing, the game falls back to `birds_background.mp3` |

---

## 🏗 Technical Details
- **Language:** C++17
- **Library:** Raylib 5.5
- **Compiler:** MinGW-w64 (GCC, msys64 ucrt64)
- **Physics:** Custom ray-cast camera logic and state-based vehicle movement.
- **Structure:** `main.cpp` = launcher, `Game.cpp` = the game, `net.cpp` = networking.
- In-game texts must be plain ASCII (the default raylib font has no Slovak diacritics).

### Building
Run `build_all.bat`, or for a quick test build:

```
g++ code/Game.cpp code/net.cpp -o build/Game.exe -std=c++17 -lraylib -lopengl32 -lgdi32 -lwinmm -lshell32 -lws2_32
```

🛑 **Troubleshooting "Permission Denied"**
If you get `ld.exe: cannot open output file build/Game.exe: Permission denied`, the game is still running.
1. Open Task Manager.
2. Find `Game.exe`.
3. Click **End Task** and compile again.

If you can't find it, create a bug report or contact: vondranoob233@gmail.com

---

## 🗺 Roadmap
- Bake roadside trees into a mesh (more FPS, same technique as the wheat).
- Custom mechanic model (currently uses the shopkeeper model).
- Food as a temporary buff (e.g. goulash = temporary speed boost).
- Sounds for buying, eating and finishing jobs.
- Unify max-speed calculation in one place (currently duplicated 3x).
- Custom 3D shop name signs (currently only colored stripes).

---

## 🤝 Credits
- **LOM_Noob** - Lead Developer / Logic / Programming / no fucking ai.
- **Jurmat** - Co-Developer / Optimization / Textures / Actually a founder of the Idea lol - LOM_Noob
- **??** - Ideas for Launcher and Game and Helper (It's a girl) - LOM_Noob
