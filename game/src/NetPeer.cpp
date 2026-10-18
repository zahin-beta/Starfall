#include "NetPeer.h"
#include "Protocol.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <cstring>

namespace net {
namespace {

constexpr float kRetransmitInterval = 0.12f;
constexpr float kConnectionTimeout = 5.0f;
constexpr float kConnectingTimeout = 10.0f;

void EnsureWinsock() {
    static bool started = false;
    if (!started) {
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);
        started = true;
    }
}

}

NetPeer::NetPeer() : socketHandle(static_cast<uintptr_t>(INVALID_SOCKET)) {
    EnsureWinsock();
}

NetPeer::~NetPeer() {
    Close();
}

bool NetPeer::Host(uint16_t port) {
    Close();
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) { status = Status::Error; return false; }
    BOOL reuse = TRUE;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (bind(sock, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
        closesocket(sock);
        status = Status::Error;
        return false;
    }
    socketHandle = static_cast<uintptr_t>(sock);
    isHost = true;
    peerKnown = false;
    timedOut = false;
    timeSinceRecv = 0.0f;
    status = Status::Listening;
    running = true;
    receiver = std::thread(&NetPeer::ReceiverLoop, this);
    return true;
}

bool NetPeer::Join(const std::string& ip, uint16_t port) {
    Close();
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) { status = Status::Error; return false; }
    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = 0;
    if (bind(sock, reinterpret_cast<sockaddr*>(&local), sizeof(local)) == SOCKET_ERROR) {
        closesocket(sock);
        status = Status::Error;
        return false;
    }
    Endpoint target{};
    in_addr parsed{};
    if (InetPtonA(AF_INET, ip.c_str(), &parsed) != 1) {
        closesocket(sock);
        status = Status::Error;
        return false;
    }
    target.address = parsed.s_addr;
    target.port = port;
    {
        std::lock_guard<std::mutex> lock(peerMutex);
        peer = target;
    }
    socketHandle = static_cast<uintptr_t>(sock);
    isHost = false;
    peerKnown = true;
    timedOut = false;
    timeSinceRecv = 0.0f;
    status = Status::Connecting;
    running = true;
    receiver = std::thread(&NetPeer::ReceiverLoop, this);
    return true;
}

void NetPeer::Close() {
    running = false;
    if (receiver.joinable()) receiver.join();
    if (socketHandle != static_cast<uintptr_t>(INVALID_SOCKET)) {
        closesocket(static_cast<SOCKET>(socketHandle));
        socketHandle = static_cast<uintptr_t>(INVALID_SOCKET);
    }
    {
        std::lock_guard<std::mutex> lock(reliableMutex);
        outgoing.clear();
        reorderBuffer.clear();
        nextSendSeq = 1;
        nextExpectedSeq = 1;
        ackSent = 0;
        ackBitsSent = 0;
    }
    {
        std::lock_guard<std::mutex> lock(incomingMutex);
        reliableIn.clear();
        unreliableIn.clear();
    }
    status = Status::Disconnected;
}

std::string NetPeer::GetPeerIp() const {
    std::lock_guard<std::mutex> lock(peerMutex);
    in_addr address{};
    address.s_addr = peer.address;
    char buffer[INET_ADDRSTRLEN] = {0};
    InetNtopA(AF_INET, &address, buffer, INET_ADDRSTRLEN);
    return std::string(buffer);
}

void NetPeer::ReceiverLoop() {
    std::vector<uint8_t> buffer(kMaxPacket);
    while (running.load()) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(static_cast<SOCKET>(socketHandle), &readSet);
        timeval timeout{0, 100000};
        int selected = select(0, &readSet, nullptr, nullptr, &timeout);
        if (!running.load()) break;
        if (selected <= 0) continue;
        sockaddr_in from{};
        int fromLength = sizeof(from);
        int received = recvfrom(static_cast<SOCKET>(socketHandle), reinterpret_cast<char*>(buffer.data()),
                                kMaxPacket, 0, reinterpret_cast<sockaddr*>(&from), &fromLength);
        if (received <= 0) continue;
        Endpoint sender{};
        sender.address = from.sin_addr.s_addr;
        sender.port = ntohs(from.sin_port);
        HandleDatagram(buffer.data(), received, sender);
    }
}

void NetPeer::HandleDatagram(const uint8_t* data, int size, const Endpoint& from) {
    if (size < static_cast<int>(sizeof(PacketHeader))) return;
    bool known = peerKnown.load();
    {
        std::lock_guard<std::mutex> lock(peerMutex);
        if (!peerKnown.load()) {
            peer = from;
            peerKnown = true;
        } else if (peer.address != from.address || peer.port != from.port) {
            return;
        }
    }
    if (!known) {
        status = Status::Connected;
    } else if (status.load() == Status::Connecting) {
        status = Status::Connected;
    }
    timeSinceRecv = 0.0f;

    PacketHeader header{};
    std::memcpy(&header, data, sizeof(header));
    if (header.magic != kMagic || header.version != kVersion) return;
    HandleAck(header.ack, header.ackBits);

    const uint8_t* payload = data + sizeof(PacketHeader);
    const int payloadSize = size - static_cast<int>(sizeof(PacketHeader));

    if (header.kind == static_cast<uint8_t>(PacketKind::Reliable)) {
        if (payloadSize <= 0) return;
        std::vector<uint8_t> message(payload, payload + payloadSize);
        std::lock_guard<std::mutex> lock(reliableMutex);
        if (static_cast<int16_t>(header.seq - ackSent) > 0) {
            uint16_t shift = static_cast<uint16_t>(header.seq - ackSent);
            if (shift >= 32) ackBitsSent = 1u;
            else ackBitsSent = (ackBitsSent << shift) | 1u | (1u << shift);
            ackSent = header.seq;
        } else {
            uint16_t difference = static_cast<uint16_t>(ackSent - header.seq);
            if (difference < 32) ackBitsSent |= (1u << difference);
        }
        if (header.seq == nextExpectedSeq) {
            {
                std::lock_guard<std::mutex> inLock(incomingMutex);
                reliableIn.push_back(message);
            }
            ++nextExpectedSeq;
            auto it = reorderBuffer.find(nextExpectedSeq);
            while (it != reorderBuffer.end()) {
                {
                    std::lock_guard<std::mutex> inLock(incomingMutex);
                    reliableIn.push_back(it->second);
                }
                reorderBuffer.erase(it);
                ++nextExpectedSeq;
                it = reorderBuffer.find(nextExpectedSeq);
            }
        } else if (static_cast<int16_t>(header.seq - nextExpectedSeq) > 0) {
            reorderBuffer[header.seq] = message;
        }
    } else if (header.kind == static_cast<uint8_t>(PacketKind::Unreliable)) {
        if (payloadSize <= 0) return;
        std::lock_guard<std::mutex> lock(incomingMutex);
        unreliableIn.emplace_back(payload, payload + payloadSize);
    }
}

void NetPeer::HandleAck(uint16_t ack, uint32_t ackBits) {
    if (ack == 0) return;
    std::lock_guard<std::mutex> lock(reliableMutex);
    for (auto it = outgoing.begin(); it != outgoing.end();) {
        int16_t difference = static_cast<int16_t>(ack - it->first);
        if (difference >= 0 && difference < 32) {
            bool acknowledged = difference == 0 || ((ackBits >> difference) & 1u) != 0;
            if (acknowledged) {
                if (rttMs.load() == 0.0f) rttMs.store(it->second.age * 1000.0f);
                it = outgoing.erase(it);
                continue;
            }
        }
        ++it;
    }
}

void NetPeer::BuildAndSend(uint8_t kind, uint16_t seq, const uint8_t* payload, int size) {
    std::vector<uint8_t> buffer(sizeof(PacketHeader) + size);
    PacketHeader header{};
    header.magic = kMagic;
    header.version = kVersion;
    header.kind = kind;
    header.seq = seq;
    {
        std::lock_guard<std::mutex> lock(reliableMutex);
        header.ack = ackSent;
        header.ackBits = ackBitsSent;
    }
    std::memcpy(buffer.data(), &header, sizeof(header));
    if (size > 0 && payload != nullptr) {
        std::memcpy(buffer.data() + sizeof(header), payload, size);
    }
    Endpoint target;
    {
        std::lock_guard<std::mutex> lock(peerMutex);
        if (!peerKnown.load()) return;
        target = peer;
    }
    SendRaw(buffer.data(), static_cast<int>(buffer.size()), target);
}

void NetPeer::SendReliableSeq(uint16_t seq, const std::vector<uint8_t>& payload) {
    BuildAndSend(static_cast<uint8_t>(PacketKind::Reliable), seq, payload.data(), static_cast<int>(payload.size()));
}

void NetPeer::SendReliable(const void* data, int size) {
    if (size <= 0) return;
    std::vector<uint8_t> payload(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + size);
    uint16_t seq;
    {
        std::lock_guard<std::mutex> lock(reliableMutex);
        seq = nextSendSeq++;
        Outgoing entry;
        entry.payload = payload;
        entry.age = 0.0f;
        outgoing[seq] = std::move(entry);
    }
    SendReliableSeq(seq, payload);
}

void NetPeer::SendUnreliable(const void* data, int size) {
    BuildAndSend(static_cast<uint8_t>(PacketKind::Unreliable), 0,
                 static_cast<const uint8_t*>(data), size);
}

bool NetPeer::PollReliable(std::vector<uint8_t>& out) {
    std::lock_guard<std::mutex> lock(incomingMutex);
    if (reliableIn.empty()) return false;
    out = std::move(reliableIn.front());
    reliableIn.pop_front();
    return true;
}

bool NetPeer::PollUnreliable(std::vector<uint8_t>& out) {
    std::lock_guard<std::mutex> lock(incomingMutex);
    if (unreliableIn.empty()) return false;
    out = std::move(unreliableIn.front());
    unreliableIn.pop_front();
    return true;
}

void NetPeer::Update(float deltaTime) {
    if (status.load() == Status::Connected) {
        timeSinceRecv += deltaTime;
        if (timeSinceRecv > kConnectionTimeout) {
            timedOut = true;
            status = Status::Error;
        }
    } else if (status.load() == Status::Connecting) {
        timeSinceRecv += deltaTime;
        if (timeSinceRecv > kConnectingTimeout) {
            timedOut = true;
            status = Status::Error;
        }
    }

    std::vector<std::pair<uint16_t, std::vector<uint8_t>>> resend;
    {
        std::lock_guard<std::mutex> lock(reliableMutex);
        for (auto& entry : outgoing) {
            entry.second.age += deltaTime;
            if (entry.second.age >= kRetransmitInterval) {
                resend.emplace_back(entry.first, entry.second.payload);
            }
        }
    }
    for (auto& item : resend) {
        SendReliableSeq(item.first, item.second);
    }
}

void NetPeer::SendRaw(const uint8_t* data, int size, const Endpoint& to) const {
    if (socketHandle == static_cast<uintptr_t>(INVALID_SOCKET)) return;
    sockaddr_in target{};
    target.sin_family = AF_INET;
    target.sin_addr.s_addr = to.address;
    target.sin_port = htons(to.port);
    std::lock_guard<std::mutex> lock(sendMutex);
    sendto(static_cast<SOCKET>(socketHandle), reinterpret_cast<const char*>(data), size, 0,
           reinterpret_cast<sockaddr*>(&target), sizeof(target));
}

std::string GetLocalIp() {
    EnsureWinsock();
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) return "0.0.0.0";
    sockaddr_in target{};
    target.sin_family = AF_INET;
    target.sin_port = htons(53);
    InetPtonA(AF_INET, "8.8.8.8", &target.sin_addr);
    std::string result = "127.0.0.1";
    if (connect(sock, reinterpret_cast<sockaddr*>(&target), sizeof(target)) == 0) {
        sockaddr_in local{};
        int length = sizeof(local);
        if (getsockname(sock, reinterpret_cast<sockaddr*>(&local), &length) == 0) {
            char buffer[INET_ADDRSTRLEN] = {0};
            InetNtopA(AF_INET, &local.sin_addr, buffer, INET_ADDRSTRLEN);
            result = buffer;
        }
    }
    closesocket(sock);
    return result;
}

}
