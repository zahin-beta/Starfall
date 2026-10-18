#pragma once
#include <cstdint>
#include <deque>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <atomic>
#include <vector>

namespace net {

enum class Status { Disconnected, Listening, Connecting, Connected, Error };

struct Endpoint {
    uint32_t address = 0; // IPv4 in network byte order
    uint16_t port = 0;    // host byte order
};

class NetPeer {
public:
    NetPeer();
    ~NetPeer();
    NetPeer(const NetPeer&) = delete;
    NetPeer& operator=(const NetPeer&) = delete;

    bool Host(uint16_t port);
    bool Join(const std::string& ip, uint16_t port);
    void Close();

    bool IsHost() const { return isHost; }
    Status GetStatus() const { return status.load(); }
    bool IsConnected() const { return status.load() == Status::Connected; }
    float GetRttMs() const { return rttMs.load(); }
    bool PeerTimedOut() const { return timedOut.load(); }
    std::string GetPeerIp() const;

    void SendReliable(const void* data, int size);
    void SendUnreliable(const void* data, int size);

    bool PollReliable(std::vector<uint8_t>& out);
    bool PollUnreliable(std::vector<uint8_t>& out);

    void Update(float deltaTime);

private:
    void ReceiverLoop();
    void HandleDatagram(const uint8_t* data, int size, const Endpoint& from);
    void SendReliableSeq(uint16_t seq, const std::vector<uint8_t>& payload);
    void BuildAndSend(uint8_t kind, uint16_t seq, const uint8_t* payload, int size);
    void SendRaw(const uint8_t* data, int size, const Endpoint& to) const;
    void HandleAck(uint16_t ack, uint32_t ackBits);

    uintptr_t socketHandle;
    bool isHost = false;
    std::atomic<Status> status{Status::Disconnected};
    Endpoint peer{};
    mutable std::mutex peerMutex;
    std::atomic<bool> peerKnown{false};
    std::atomic<bool> timedOut{false};

    std::thread receiver;
    std::atomic<bool> running{false};
    mutable std::mutex sendMutex;

    std::mutex reliableMutex;
    uint16_t nextSendSeq = 1;
    struct Outgoing { std::vector<uint8_t> payload; float age; };
    std::map<uint16_t, Outgoing> outgoing;
    uint16_t nextExpectedSeq = 1;
    std::map<uint16_t, std::vector<uint8_t>> reorderBuffer;

    uint16_t ackSent = 0;
    uint32_t ackBitsSent = 0;

    std::mutex incomingMutex;
    std::deque<std::vector<uint8_t>> reliableIn;
    std::deque<std::vector<uint8_t>> unreliableIn;

    std::atomic<float> rttMs{0.0f};
    float timeSinceRecv = 0.0f;
};

std::string GetLocalIp();

}
