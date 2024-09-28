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
     * .
     * This is a helper type to encapsulate common information needed by the reliability layer
     */
    struct PacketStream
    {
        std::map<Packet::IdType, Packet> reliablePackets{};

        std::queue<Packet> readyPackets{};
        
        mutable std::shared_mutex lock{};

        Packet::IdType sequenceNumber{Packet::InvalidId};

        std::optional<Packet> PopNextReadyPacket();
    };

    // Enqueues all received packets. If unreliable, processes them right away. If not, buffers them until the proper
    // sequence of packets is received.
    // .
    // Whenever a packet is sent, puts it into a chronological queue to be sent to the server
    class Connection
    {
        NetAddress address{};

        PacketStream outgoing{};

        PacketStream incoming{};

        std::atomic<Packet::IdType> currentAckId{Packet::InvalidId};

    public:
        Connection(const NetAddress& address);

        Connection() = default;
        
        void Schedule(Packet&& packet);

        std::optional<Packet> PopReadyOutgoingPacket();

        void SetOutgoingSequenceNumber(Packet::IdType sequenceNumber);

        void Received(Packet&& packet);

        std::optional<Packet> PopReadyIncomingPacket();

        void SetIncomingSequenceNumber(Packet::IdType sequenceNumber);
        
        Packet::IdType GetLastProcessedIncomingPacketId() const { return incoming.sequenceNumber; }

        std::string ToString() const;

        const NetAddress& GetAddress() const { return address; }
        
        /**
         * Returns the id of the latest packet received.
         * .
         * Note that packet ids can overflow, so the latest id may eventually wrap back to a smaller number (such as
         * going from 'Packet::MaxPosssibleId' back to 1)
         */
        Packet::IdType GetCurrentAckId() const { return currentAckId.load(); }

        bool HasReceivedPacket(Packet::IdType id) const; 

        void AddAckHeader(Packet& packet);
        
    private:
        void UpdateAckId(Packet::IdType newId);
        
        Packet::IdType GetIndexForNextPacket();

        void EnqueueProcessablePackets();
    };
}
