#pragma once
#include <map>
#include <queue>
#include <set>
#include <shared_mutex>

#include "Core.h"
#include "Packet.h"

namespace GioNet
{
    class Socket;

    // Enqueues all received packets. If unreliable, processes them right away. If not, buffers them until the proper
    // sequence of packets is received.
    // ..
    // Whenever a packet is sent, puts it into a chronological queue to be sent to the server
    class Connection
    {
        NetAddress address{};

        std::map<Packet::IdType, Packet> sentReliablePackets{};

        std::queue<Packet> readyOutgoingPackets{};
        
        std::shared_mutex outgoingPacketLock{};

        Packet::IdType localPacketIndex{Packet::InvalidId};

        Packet::IdType lastProcessedPacketIndex{Packet::InvalidId};
        
        std::map<Packet::IdType, Packet> receivedReliablePackets{};

        std::queue<Packet> readyIncomingPackets{};

        std::shared_mutex incomingPacketLock{};

        Packet::IdType remotePacketIndex{Packet::InvalidId};

    public:
        Connection(const NetAddress& address);

        Connection() = default;
        
        void Schedule(Packet&& packet);

        std::optional<Packet> GetReadyOutgoingPacket();

        void Received(Packet&& packet);

        std::optional<Packet> GetReadyIncomingPacket();

        std::string ToString() const;

        const NetAddress& GetAddress() const { return address; }

    private:
        Packet::IdType GetIndexForNextPacket();
    };
}
