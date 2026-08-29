#pragma once

#include <juce_events/juce_events.h>

#include <functional>
#include <string>
#include <vector>
#include <mutex>

struct DiscoveredPeer {
    std::string instanceID;
    std::string description;
    std::string address;
    int port = 0;
    juce::Time lastSeen;
};

class PeerDiscovery {
public:
    static constexpr int kDefaultBroadcastPort = 40500;
    static constexpr const char* kServiceType = "_cendance._tcp";

    PeerDiscovery();
    ~PeerDiscovery();

    // Start advertising this cendance instance on the LAN
    bool startAdvertising(int agentPort, const std::string& name, std::string& error);

    // Stop advertising
    void stopAdvertising();

    // Start browsing for peers on the LAN
    bool startBrowsing(std::string& error);

    // Stop browsing
    void stopBrowsing();

    // Get the current list of discovered peers
    std::vector<DiscoveredPeer> getPeers() const;

    // Callback when peer list changes
    std::function<void()> onPeersChanged;

private:
    void onServiceListChanged();

    // JUCE discovery objects
    std::unique_ptr<juce::NetworkServiceDiscovery::Advertiser> advertiser;
    std::unique_ptr<juce::NetworkServiceDiscovery::AvailableServiceList> browser;

    mutable std::mutex peersMutex;
    std::vector<DiscoveredPeer> peers;
    bool advertising = false;
    bool browsing = false;
};
