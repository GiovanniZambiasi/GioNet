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

    /**
     * Represents a stream of packets, either outgoing or incoming.
     * <br>
     * This is a helper type to encapsulate common information needed by the reliability layer
     */
    struct PacketStream
    {
        std::map<Packet::IdType, Packet> reliablePackets{};

        std::queue<Packet> readyPackets{};
        
        std::shared_mutex lock{};

        Packet::IdType sequenceNumber{Packet::InvalidId};

        std::optional<Packet> PopNextReadyPacket();
    };

    // Enqueues all received packets. If unreliable, processes them right away. If not, buffers them until the proper
    // sequence of packets is received.
    // ..
    // Whenever a packet is sent, puts it into a chronological queue to be sent to the server
    class Connection
    {
        NetAddress address{};

        PacketStream outgoing{};

        PacketStream incoming{};

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
