#pragma once
#include <map>
#include <queue>
#include <set>
#include <shared_mutex>
#include <thread>

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

        std::shared_ptr<Socket> socket{};

        std::map<Packet::IdType, Packet> sentReliablePackets{};

        std::queue<Packet> readyOutgoingPackets{};
        
        std::shared_mutex outgoingPacketLock{};

        Packet::IdType localPacketIndex{Packet::InvalidId};

        Packet::IdType lastProcessedPacketIndex{0};
        
        std::map<Packet::IdType, Packet> receivedReliablePackets{};

        std::queue<Packet> readyIncomingPackets{};

        std::shared_mutex incomingPacketLock{};

        Packet::IdType remotePacketIndex{};

    public:
        Connection(const NetAddress& address, const std::shared_ptr<Socket>& socket);

        void Schedule(Packet&& packet);

        std::optional<Packet> GetReadyOutgoingPacket();

        void Received(Packet&& packet);

        std::optional<Packet> GetReadyIncomingPacket();

    private:
        Packet::IdType GetIndexForNextPacket();

        void BuildAckHeaderAndSend(Packet packet);
    };
}
