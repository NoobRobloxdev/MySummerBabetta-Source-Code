#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdio>
#include <cstring>
#include "net.h"

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

#ifndef SIO_UDP_CONNRESET
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
#endif

namespace {
    const uint32_t MAGIC = 0xBAB37770u;
    const int MAX_PLAYERS = 8;
    const DWORD TIMEOUT_MS = 5000;
    const DWORD SEND_EVERY_MS = 33; // ~30x za sekundu

    // typ 1 = klient -> host (stav klienta), typ 2 = host -> klient (stav vsetkych)
    struct Header { uint32_t magic; uint8_t type; uint8_t yourId; uint8_t count; uint8_t pad; };
    struct Remote { bool used; sockaddr_in addr; DWORD lastSeen; NetPlayer st; };

    SOCKET g_sock = INVALID_SOCKET;
    bool g_wsa = false, g_active = false, g_host = false, g_connected = false;
    unsigned short g_port = 0;
    sockaddr_in g_server;
    Remote g_remotes[MAX_PLAYERS];   // host: index = id klienta (1..7)
    NetPlayer g_others[MAX_PLAYERS]; // klient: ostatni hraci z posledneho paketu
    int g_otherCount = 0;
    int g_myId = 0;
    DWORD g_lastSend = 0, g_lastRecv = 0;
    char g_err[160] = "";
    char g_status[200] = "";
    char g_ip[64] = "";
    char g_target[64] = "";

    // --- LAN objavovanie serverov ---
    const unsigned short DISC_PORT = 7778;
    const uint32_t DISC_MAGIC = 0xBAB37771u;
    const DWORD ANNOUNCE_EVERY_MS = 1000;
    const DWORD DISC_TIMEOUT_MS = 4000;
    struct DiscPacket { uint32_t magic; uint8_t type; uint8_t players; uint8_t maxPlayers; uint8_t pad; uint16_t gamePort; char name[32]; };
    struct DiscEntry { bool used; Net::LanServer s; DWORD lastSeen; };
    SOCKET g_disc = INVALID_SOCKET;
    bool g_discWsa = false;
    DiscEntry g_discList[16];
    DWORD g_lastAnnounce = 0;
    char g_hostName[32] = "Babetta";

    bool SameAddr(const sockaddr_in& a, const sockaddr_in& b) {
        return a.sin_addr.s_addr == b.sin_addr.s_addr && a.sin_port == b.sin_port;
    }

    bool OpenSocket(bool bindPort, unsigned short port) {
        if (!g_wsa) {
            WSADATA w;
            if (WSAStartup(MAKEWORD(2, 2), &w) != 0) {
                snprintf(g_err, sizeof(g_err), "Winsock sa nepodarilo spustit");
                return false;
            }
            g_wsa = true;
        }
        g_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (g_sock == INVALID_SOCKET) {
            snprintf(g_err, sizeof(g_err), "Socket sa nepodarilo vytvorit (WSA %d)", WSAGetLastError());
            return false;
        }
        u_long nonBlocking = 1;
        ioctlsocket(g_sock, FIONBIO, &nonBlocking);

        // Windows: ignoruj ICMP "port unreachable" (inak recvfrom hadze chybu 10054)
        BOOL noReset = FALSE;
        DWORD bytes = 0;
        WSAIoctl(g_sock, SIO_UDP_CONNRESET, &noReset, sizeof(noReset), nullptr, 0, &bytes, nullptr, nullptr);

        if (bindPort) {
            sockaddr_in a;
            memset(&a, 0, sizeof(a));
            a.sin_family = AF_INET;
            a.sin_addr.s_addr = htonl(INADDR_ANY);
            a.sin_port = htons(port);
            if (bind(g_sock, (sockaddr*)&a, sizeof(a)) == SOCKET_ERROR) {
                snprintf(g_err, sizeof(g_err), "Port %u je obsadeny (WSA %d). Uz bezi iny host?", (unsigned)port, WSAGetLastError());
                closesocket(g_sock);
                g_sock = INVALID_SOCKET;
                return false;
            }
        }
        return true;
    }

    int CountRemotes() {
        int n = 0;
        for (int i = 1; i < MAX_PLAYERS; i++) if (g_remotes[i].used) n++;
        return n;
    }
}

namespace Net {

bool StartHost(unsigned short port) {
    Stop();
    g_err[0] = 0;
    if (!OpenSocket(true, port)) return false;
    g_active = true;
    g_host = true;
    g_port = port;
    g_myId = 0;
    g_lastAnnounce = 0;

    BOOL yes = TRUE;
    setsockopt(g_sock, SOL_SOCKET, SO_BROADCAST, (const char*)&yes, sizeof(yes));
    char name[256];
    if (gethostname(name, sizeof(name)) == 0) snprintf(g_hostName, sizeof(g_hostName), "%s", name);
    return true;
}

bool StartClient(const char* host, unsigned short port) {
    Stop();
    g_err[0] = 0;

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    addrinfo* res = nullptr;

    if (!g_wsa) {
        WSADATA w;
        if (WSAStartup(MAKEWORD(2, 2), &w) != 0) {
            snprintf(g_err, sizeof(g_err), "Winsock sa nepodarilo spustit");
            return false;
        }
        g_wsa = true;
    }
    if (getaddrinfo(host, nullptr, &hints, &res) != 0 || !res) {
        snprintf(g_err, sizeof(g_err), "Neplatna IP / meno hosta: %s", host);
        return false;
    }
    memcpy(&g_server, res->ai_addr, sizeof(sockaddr_in));
    freeaddrinfo(res);
    g_server.sin_port = htons(port);

    if (!OpenSocket(false, 0)) return false;
    snprintf(g_target, sizeof(g_target), "%s", host);
    g_active = true;
    g_host = false;
    g_connected = false;
    g_port = port;
    g_lastRecv = GetTickCount();
    return true;
}

void Stop() {
    if (g_sock != INVALID_SOCKET) { closesocket(g_sock); g_sock = INVALID_SOCKET; }
    if (g_wsa) { WSACleanup(); g_wsa = false; }
    g_active = false;
    g_host = false;
    g_connected = false;
    g_otherCount = 0;
    g_myId = 0;
    memset(g_remotes, 0, sizeof(g_remotes));
}

bool IsActive() { return g_active; }
bool IsHost() { return g_host; }

void Update(const NetPlayer& self) {
    if (!g_active) return;
    DWORD now = GetTickCount();

    // ---- PRIJEM ----
    for (;;) {
        char buf[1024];
        sockaddr_in from;
        int fromLen = sizeof(from);
        int n = recvfrom(g_sock, buf, sizeof(buf), 0, (sockaddr*)&from, &fromLen);
        if (n == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAECONNRESET) continue;
            break; // WSAEWOULDBLOCK = nic dalsie na citanie
        }
        if (n < (int)sizeof(Header)) continue;
        Header h;
        memcpy(&h, buf, sizeof(h));
        if (h.magic != MAGIC) continue;

        if (g_host) {
            if (h.type != 1) continue;
            if ((size_t)n < sizeof(Header) + sizeof(NetPlayer)) continue;

            int idx = -1;
            for (int i = 1; i < MAX_PLAYERS; i++)
                if (g_remotes[i].used && SameAddr(g_remotes[i].addr, from)) { idx = i; break; }
            if (idx < 0) {
                for (int i = 1; i < MAX_PLAYERS; i++)
                    if (!g_remotes[i].used) {
                        idx = i;
                        g_remotes[i].used = true;
                        g_remotes[i].addr = from;
                        break;
                    }
            }
            if (idx < 0) continue; // server plny

            NetPlayer p;
            memcpy(&p, buf + sizeof(Header), sizeof(p));
            p.id = (uint8_t)idx;
            g_remotes[idx].st = p;
            g_remotes[idx].lastSeen = now;
        } else {
            if (h.type != 2) continue;
            int count = h.count;
            if (count > MAX_PLAYERS) count = MAX_PLAYERS;
            if ((size_t)n < sizeof(Header) + (size_t)count * sizeof(NetPlayer)) continue;

            g_myId = h.yourId;
            g_connected = true;
            g_lastRecv = now;
            g_otherCount = 0;
            for (int i = 0; i < count; i++) {
                NetPlayer p;
                memcpy(&p, buf + sizeof(Header) + (size_t)i * sizeof(NetPlayer), sizeof(p));
                if (p.id != g_myId) g_others[g_otherCount++] = p;
            }
        }
    }

    // ---- ODOSIELANIE ----
    if (g_host) {
        for (int i = 1; i < MAX_PLAYERS; i++)
            if (g_remotes[i].used && now - g_remotes[i].lastSeen > TIMEOUT_MS) g_remotes[i].used = false;

        // Oznam pre LAN zoznam serverov v launcheri
        if (now - g_lastAnnounce >= ANNOUNCE_EVERY_MS) {
            g_lastAnnounce = now;
            DiscPacket dp;
            memset(&dp, 0, sizeof(dp));
            dp.magic = DISC_MAGIC;
            dp.type = 3;
            dp.players = (uint8_t)(CountRemotes() + 1);
            dp.maxPlayers = (uint8_t)MAX_PLAYERS;
            dp.gamePort = g_port;
            snprintf(dp.name, sizeof(dp.name), "%s", g_hostName);
            sockaddr_in b;
            memset(&b, 0, sizeof(b));
            b.sin_family = AF_INET;
            b.sin_addr.s_addr = htonl(INADDR_BROADCAST);
            b.sin_port = htons(DISC_PORT);
            sendto(g_sock, (const char*)&dp, (int)sizeof(dp), 0, (sockaddr*)&b, sizeof(b));
        }

        if (now - g_lastSend >= SEND_EVERY_MS) {
            g_lastSend = now;
            NetPlayer list[MAX_PLAYERS];
            int n = 0;
            NetPlayer me = self;
            me.id = 0;
            list[n++] = me;
            for (int i = 1; i < MAX_PLAYERS; i++)
                if (g_remotes[i].used) list[n++] = g_remotes[i].st;

            char buf[sizeof(Header) + MAX_PLAYERS * sizeof(NetPlayer)];
            for (int i = 1; i < MAX_PLAYERS; i++) {
                if (!g_remotes[i].used) continue;
                Header h;
                h.magic = MAGIC; h.type = 2; h.yourId = (uint8_t)i; h.count = (uint8_t)n; h.pad = 0;
                memcpy(buf, &h, sizeof(h));
                memcpy(buf + sizeof(h), list, (size_t)n * sizeof(NetPlayer));
                sendto(g_sock, buf, (int)(sizeof(h) + (size_t)n * sizeof(NetPlayer)), 0,
                       (sockaddr*)&g_remotes[i].addr, sizeof(sockaddr_in));
            }
        }
    } else {
        if (now - g_lastSend >= SEND_EVERY_MS) {
            g_lastSend = now;
            char buf[sizeof(Header) + sizeof(NetPlayer)];
            Header h;
            h.magic = MAGIC; h.type = 1; h.yourId = (uint8_t)g_myId; h.count = 1; h.pad = 0;
            memcpy(buf, &h, sizeof(h));
            memcpy(buf + sizeof(h), &self, sizeof(NetPlayer));
            sendto(g_sock, buf, (int)sizeof(buf), 0, (sockaddr*)&g_server, sizeof(g_server));
        }
    }
}

int GetRemotePlayers(NetPlayer* out, int maxOut) {
    int n = 0;
    if (!g_active) return 0;
    if (g_host) {
        for (int i = 1; i < MAX_PLAYERS && n < maxOut; i++)
            if (g_remotes[i].used) out[n++] = g_remotes[i].st;
    } else {
        for (int i = 0; i < g_otherCount && n < maxOut; i++) out[n++] = g_others[i];
    }
    return n;
}

const char* Status() {
    if (!g_active) {
        snprintf(g_status, sizeof(g_status), "%s", g_err[0] ? g_err : "Multiplayer vypnuty");
    } else if (g_host) {
        snprintf(g_status, sizeof(g_status), "HOST na porte %u | hracov: %d", (unsigned)g_port, CountRemotes() + 1);
    } else if (g_connected && GetTickCount() - g_lastRecv > TIMEOUT_MS) {
        snprintf(g_status, sizeof(g_status), "Server neodpoveda (%s)...", g_target);
    } else if (g_connected) {
        snprintf(g_status, sizeof(g_status), "PRIPOJENY na %s ako hrac #%d", g_target, g_myId);
    } else {
        snprintf(g_status, sizeof(g_status), "Pripajam sa na %s:%u ...", g_target, (unsigned)g_port);
    }
    return g_status;
}

const char* LocalIP() {
    snprintf(g_ip, sizeof(g_ip), "?");
    if (!g_wsa) return g_ip;
    char name[256];
    if (gethostname(name, sizeof(name)) != 0) return g_ip;

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    addrinfo* res = nullptr;
    if (getaddrinfo(name, nullptr, &hints, &res) != 0 || !res) return g_ip;

    for (addrinfo* p = res; p; p = p->ai_next) {
        sockaddr_in* a = (sockaddr_in*)p->ai_addr;
        const char* s = inet_ntoa(a->sin_addr);
        if (s && strncmp(s, "127.", 4) != 0) {
            snprintf(g_ip, sizeof(g_ip), "%s", s);
            break;
        }
    }
    freeaddrinfo(res);
    return g_ip;
}

// ================= LAN zoznam serverov (launcher) =================

void DiscoveryStop() {
    if (g_disc != INVALID_SOCKET) { closesocket(g_disc); g_disc = INVALID_SOCKET; }
    if (g_discWsa) { WSACleanup(); g_discWsa = false; }
}

void DiscoveryClear() {
    memset(g_discList, 0, sizeof(g_discList));
}

bool DiscoveryStart() {
    DiscoveryStop();
    if (!g_discWsa) {
        WSADATA w;
        if (WSAStartup(MAKEWORD(2, 2), &w) != 0) return false;
        g_discWsa = true;
    }
    g_disc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (g_disc == INVALID_SOCKET) return false;

    u_long nonBlocking = 1;
    ioctlsocket(g_disc, FIONBIO, &nonBlocking);
    BOOL yes = TRUE;
    setsockopt(g_disc, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));
    BOOL noReset = FALSE;
    DWORD bytes = 0;
    WSAIoctl(g_disc, SIO_UDP_CONNRESET, &noReset, sizeof(noReset), nullptr, 0, &bytes, nullptr, nullptr);

    sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_ANY);
    a.sin_port = htons(DISC_PORT);
    if (bind(g_disc, (sockaddr*)&a, sizeof(a)) == SOCKET_ERROR) {
        closesocket(g_disc);
        g_disc = INVALID_SOCKET;
        return false;
    }
    DiscoveryClear();
    return true;
}

int DiscoveryPoll(LanServer* out, int maxOut) {
    if (g_disc == INVALID_SOCKET) return 0;
    DWORD now = GetTickCount();

    for (;;) {
        DiscPacket dp;
        sockaddr_in from;
        int fromLen = sizeof(from);
        int n = recvfrom(g_disc, (char*)&dp, sizeof(dp), 0, (sockaddr*)&from, &fromLen);
        if (n == SOCKET_ERROR) {
            int e = WSAGetLastError();
            if (e == WSAECONNRESET || e == WSAEMSGSIZE) continue;
            break;
        }
        if (n != (int)sizeof(dp) || dp.magic != DISC_MAGIC || dp.type != 3) continue;

        const char* ip = inet_ntoa(from.sin_addr);
        if (!ip) continue;
        int idx = -1;
        for (int i = 0; i < 16; i++)
            if (g_discList[i].used && strcmp(g_discList[i].s.ip, ip) == 0) { idx = i; break; }
        if (idx < 0) {
            for (int i = 0; i < 16; i++)
                if (!g_discList[i].used) { idx = i; g_discList[i].used = true; break; }
        }
        if (idx < 0) continue;

        dp.name[sizeof(dp.name) - 1] = 0;
        snprintf(g_discList[idx].s.ip, sizeof(g_discList[idx].s.ip), "%s", ip);
        snprintf(g_discList[idx].s.name, sizeof(g_discList[idx].s.name), "%s", dp.name);
        g_discList[idx].s.players = dp.players;
        g_discList[idx].s.maxPlayers = dp.maxPlayers;
        g_discList[idx].lastSeen = now;
    }

    int count = 0;
    for (int i = 0; i < 16; i++) {
        if (!g_discList[i].used) continue;
        if (now - g_discList[i].lastSeen > DISC_TIMEOUT_MS) { g_discList[i].used = false; continue; }
        if (count < maxOut) out[count++] = g_discList[i].s;
    }
    return count;
}

} // namespace Net