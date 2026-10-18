#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace net {

struct HostInfo {
    std::string name;
    std::string ip;
    uint16_t port = 0;
    float lastSeen = 0.0f;
};

class Discovery {
public:
    ~Discovery();
    bool StartHost(const std::string& name, uint16_t gamePort);
    bool StartClient();
    void Stop();
    void Update(float deltaTime);
    const std::vector<HostInfo>& GetHosts() const { return hosts; }
    void ClearHosts() { hosts.clear(); }

private:
    void SendBeacon();
    void Poll(float deltaTime);

    uintptr_t socketHandle = ~static_cast<uintptr_t>(0);
    std::string hostName;
    uint16_t hostGamePort = 0;
    float beaconTimer = 0.0f;
    float elapsed = 0.0f;
    bool hosting = false;
    std::vector<HostInfo> hosts;
};

}
