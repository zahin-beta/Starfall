#include "Discovery.h"
#include "Protocol.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <cstring>
#include <algorithm>

namespace net {
namespace {

void EnsureWinsock() {
    static bool started = false;
    if (!started) {
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);
        started = true;
    }
}

constexpr float kBeaconInterval = 1.0f;
constexpr float kHostLifetime = 5.0f;

}

Discovery::~Discovery() {
    Stop();
}

bool Discovery::StartHost(const std::string& name, uint16_t gamePort) {
    Stop();
    EnsureWinsock();
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) return false;
    BOOL broadcast = TRUE;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&broadcast), sizeof(broadcast));
    socketHandle = static_cast<uintptr_t>(sock);
    hosting = true;
    hostName = name;
    hostGamePort = gamePort;
    beaconTimer = kBeaconInterval;
    elapsed = 0.0f;
    return true;
}

bool Discovery::StartClient() {
    Stop();
    EnsureWinsock();
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) return false;
    BOOL reuse = TRUE;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));
    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(kDiscoveryPort);
    if (bind(sock, reinterpret_cast<sockaddr*>(&local), sizeof(local)) == SOCKET_ERROR) {
        closesocket(sock);
        return false;
    }
    u_long nonBlocking = 1;
    ioctlsocket(sock, FIONBIO, &nonBlocking);
    socketHandle = static_cast<uintptr_t>(sock);
    hosting = false;
    elapsed = 0.0f;
    hosts.clear();
    return true;
}

void Discovery::Stop() {
    if (socketHandle != ~static_cast<uintptr_t>(0)) {
        closesocket(static_cast<SOCKET>(socketHandle));
        socketHandle = ~static_cast<uintptr_t>(0);
    }
    hosting = false;
    hosts.clear();
}

void Discovery::SendBeacon() {
    if (!hosting || socketHandle == ~static_cast<uintptr_t>(0)) return;
    BeaconPacket packet{};
    packet.magic = kMagic;
    packet.version = kVersion;
    packet.players = 1;
    packet.gamePort = htons(hostGamePort);
    std::strncpy(packet.name, hostName.c_str(), sizeof(packet.name) - 1);

    sockaddr_in target{};
    target.sin_family = AF_INET;
    target.sin_port = htons(kDiscoveryPort);
    target.sin_addr.s_addr = INADDR_BROADCAST;
    sendto(static_cast<SOCKET>(socketHandle), reinterpret_cast<const char*>(&packet), sizeof(packet), 0,
           reinterpret_cast<sockaddr*>(&target), sizeof(target));

    target.sin_port = htons(kDiscoveryPort);
    target.sin_addr.s_addr = inet_addr("127.0.0.1");
    sendto(static_cast<SOCKET>(socketHandle), reinterpret_cast<const char*>(&packet), sizeof(packet), 0,
           reinterpret_cast<sockaddr*>(&target), sizeof(target));
}

void Discovery::Poll(float deltaTime) {
    if (socketHandle == ~static_cast<uintptr_t>(0)) return;
    SOCKET sock = static_cast<SOCKET>(socketHandle);
    for (int i = 0; i < 8; ++i) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(sock, &readSet);
        timeval timeout{0, 0};
        if (select(0, &readSet, nullptr, nullptr, &timeout) <= 0) break;
        char buffer[kMaxPacket];
        sockaddr_in from{};
        int length = sizeof(from);
        int received = recvfrom(sock, buffer, kMaxPacket, 0, reinterpret_cast<sockaddr*>(&from), &length);
        if (received < static_cast<int>(sizeof(BeaconPacket))) continue;
        BeaconPacket packet{};
        std::memcpy(&packet, buffer, sizeof(packet));
        if (packet.magic != kMagic || packet.version != kVersion) continue;
        if (hosting) continue;
        char ipBuffer[INET_ADDRSTRLEN] = {0};
        in_addr address{};
        address.s_addr = from.sin_addr.s_addr;
        InetNtopA(AF_INET, &address, ipBuffer, INET_ADDRSTRLEN);
        const std::string ip = ipBuffer;
        const uint16_t port = ntohs(packet.gamePort);
        bool updated = false;
        for (auto& host : hosts) {
            if (host.ip == ip && host.port == port) {
                host.lastSeen = 0.0f;
                updated = true;
                break;
            }
        }
        if (!updated) {
            HostInfo info;
            info.name = std::string(packet.name);
            info.ip = ip;
            info.port = port;
            info.lastSeen = 0.0f;
            hosts.push_back(info);
        }
    }
    elapsed += deltaTime;
    for (auto& host : hosts) host.lastSeen += deltaTime;
    hosts.erase(std::remove_if(hosts.begin(), hosts.end(),
                               [](const HostInfo& host) { return host.lastSeen > kHostLifetime; }),
                hosts.end());
}

void Discovery::Update(float deltaTime) {
    if (hosting) {
        beaconTimer += deltaTime;
        if (beaconTimer >= kBeaconInterval) {
            beaconTimer = 0.0f;
            SendBeacon();
        }
    }
    Poll(deltaTime);
}

}
