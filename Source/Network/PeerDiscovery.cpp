#include "PeerDiscovery.h"

#include <juce_core/juce_core.h>

PeerDiscovery::PeerDiscovery() = default;

PeerDiscovery::~PeerDiscovery() {
    stopAdvertising();
    stopBrowsing();
}

bool PeerDiscovery::startAdvertising(int agentPort, const std::string& name, std::string& error) {
    if (advertising) {
        stopAdvertising();
    }

    try {
        advertiser = std::make_unique<juce::NetworkServiceDiscovery::Advertiser>(
            juce::String(kServiceType),
            juce::String(name),
            kDefaultBroadcastPort,
            agentPort,
            juce::RelativeTime::seconds(1.5));
        advertising = true;
        return true;
    } catch (const std::exception& e) {
        error = e.what();
        return false;
    }
}

void PeerDiscovery::stopAdvertising() {
    advertiser.reset();
    advertising = false;
}

bool PeerDiscovery::startBrowsing(std::string& error) {
    if (browsing) {
        stopBrowsing();
    }

    try {
        browser = std::make_unique<juce::NetworkServiceDiscovery::AvailableServiceList>(
            juce::String(kServiceType),
            kDefaultBroadcastPort);

        browser->onChange = [this]() { onServiceListChanged(); };
        browsing = true;
        return true;
    } catch (const std::exception& e) {
        error = e.what();
        return false;
    }
}

void PeerDiscovery::stopBrowsing() {
    browser.reset();
    browsing = false;
}

std::vector<DiscoveredPeer> PeerDiscovery::getPeers() const {
    std::lock_guard<std::mutex> lock(peersMutex);
    return peers;
}

void PeerDiscovery::onServiceListChanged() {
    if (!browser) return;

    auto services = browser->getServices();
    std::vector<DiscoveredPeer> newPeers;
    newPeers.reserve(services.size());

    for (const auto& svc : services) {
        DiscoveredPeer peer;
        peer.instanceID = svc.instanceID.toStdString();
        peer.description = svc.description.toStdString();
        peer.address = svc.address.toString().toStdString();
        peer.port = svc.port;
        peer.lastSeen = svc.lastSeen;
        newPeers.push_back(peer);
    }

    {
        std::lock_guard<std::mutex> lock(peersMutex);
        peers = std::move(newPeers);
    }

    if (onPeersChanged) {
        onPeersChanged();
    }
}
