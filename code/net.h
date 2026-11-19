#pragma once
// Jednoduchy UDP multiplayer (len test). Je v samostatnom subore net.cpp,
// pretoze Winsock a raylib maju konfliktne nazvy (Rectangle, CloseWindow, ...).
#include <cstdint>

struct NetPlayer {
    uint8_t id;        // 0 = host, 1..7 = klienti
    uint8_t riding;    // 1 = sedi na Babette
    uint8_t r, g, b;   // farba Babetty
    float px, py, pz, pyaw;            // postava (ked ide pesky)
    float bx, by, bz, brot, blean;     // Babetta
};

namespace Net {
    bool StartHost(unsigned short port);
    bool StartClient(const char* host, unsigned short port);
    void Stop();
    bool IsActive();
    bool IsHost();
    void Update(const NetPlayer& self);                 // volat raz za frame
    int  GetRemotePlayers(NetPlayer* out, int maxOut);  // ostatni hraci
    const char* Status();                               // text pre HUD
    const char* LocalIP();                              // IP hosta (pre kamaratov)

    // --- LAN zoznam serverov (pouziva launcher) ---
    // Host v hre vysiela kazdu sekundu oznam na UDP port 7778, launcher ho zachyti.
    struct LanServer {
        char ip[46];
        char name[33];
        uint8_t players;
        uint8_t maxPlayers;
    };
    bool DiscoveryStart();                              // zacni pocuvat (launcher)
    void DiscoveryStop();
    void DiscoveryClear();                              // vymaz zoznam (OBNOVIT)
    int  DiscoveryPoll(LanServer* out, int maxOut);     // volat kazdy frame, vrati pocet serverov
}