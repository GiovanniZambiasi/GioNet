#include "Connection.h"

#include <chrono>

#include "Socket.h"

std::optional<GioNet::Packet> GioNet::PacketStream::PopNextReadyPacket()
{
    std::scoped_lock _{lock};
    
    if(readyPackets.empty())
        return {};

    Packet packet = std::move(readyPackets.front());
    readyPackets.pop();
    return {std::move(packet)};
}

GioNet::Connection::Connection(const NetAddress& address)
    : address(address)
{
}

void GioNet::Connection::Schedule(Packet&& packet)
{
    std::unique_lock _{outgoing.lock};
    
    if(packet.HasFlags(Packet::Flags::Reliable))
    {
        Packet::IdType index = GetIndexForNextPacket();
        packet.id = index;
        outgoing.reliablePackets[index] = packet;
    }
    
    outgoing.readyPackets.push(std::move(packet));
}

std::optional<GioNet::Packet> GioNet::Connection::PopReadyOutgoingPacket()
{
    std::optional<Packet> packet = outgoing.PopNextReadyPacket();

    if(packet)
    {
        AddAckHeader(*packet);
    }
        
    return packet;
}

void GioNet::Connection::Received(Packet&& packet)
{
    std::unique_lock _{incoming.lock};
    
    // TODO - Check incoming acks, remove unnecessary outgoing packets
    
    if(packet.HasFlags(Packet::Flags::Reliable))
    {
        assert(packet.id != Packet::InvalidId);
        auto packetId = packet.id;
        UpdateAckId(packetId);
        incoming.reliablePackets[packetId] = std::move(packet);
        EnqueueProcessablePackets();
    }
    else
    {
        incoming.readyPackets.push(std::move(packet));
    }
}

std::optional<GioNet::Packet> GioNet::Connection::PopReadyIncomingPacket()
{
    return incoming.PopNextReadyPacket();
}

void GioNet::Connection::SetIncomingSequenceNumber(Packet::IdType sequenceNumber)
{
    incoming.sequenceNumber = sequenceNumber;
}

std::string GioNet::Connection::ToString() const
{
    return address.ToString();
}

bool GioNet::Connection::HasReceivedPacket(Packet::IdType id) const
{
    std::shared_lock _{incoming.lock};
    return incoming.reliablePackets.contains(id);
}

void GioNet::Connection::SetOutgoingSequenceNumber(Packet::IdType sequenceNumber)
{
    outgoing.sequenceNumber = sequenceNumber;
}

GioNet::Packet::IdType GioNet::Connection::GetIndexForNextPacket()
{
    if(outgoing.sequenceNumber == Packet::MaxPossibleId)
    {
        // Skip invalid id (0)
        outgoing.sequenceNumber++;
    }
    
    return ++outgoing.sequenceNumber;
}

void GioNet::Connection::AddAckHeader(Packet& packet)
{
    static constexpr size_t ackCount = sizeof(Packet::ackBitset);
    
    packet.ackId = GetCurrentAckId();
    packet.ackBitset = 0;

    for (Packet::AckBitsetType i = 0; i < static_cast<Packet::AckBitsetType>(ackCount); ++i)
    {
        Packet::IdType offset = static_cast<Packet::IdType>(i) + 1;
        Packet::IdType id = packet.ackId - offset;

        if(HasReceivedPacket(id))
        {
            packet.ackBitset |= 1 << i;
        }
    }
}

/*
 * Because packet ids can overflow, simply checking if the new id is greater won't work. This method also takes into
 * account the numerical distance between the previous ack id, and the new packet id. If their distance is too great,
 * we consider the smaller "new id" to actually be the latest packet received. 
*/
void GioNet::Connection::UpdateAckId(Packet::IdType newId)
{
    static constexpr Packet::IdType HalfId = Packet::MaxPossibleId / 2;

    Packet::IdType currentId = currentAckId.load();

    if(currentId == Packet::InvalidId)
    {
        currentAckId.store(newId);
        return;
    }

    Packet::IdType distance;
    bool newIdIsGreater;

    if(currentId > newId)
    {
        distance = currentId - newId;
        newIdIsGreater = false;
    }
    else
    {
        distance = newId - currentId;
        newIdIsGreater = true;
    }
    
    if((distance < HalfId && newIdIsGreater) ||
        (distance > HalfId && !newIdIsGreater))
    {
        currentAckId.store(newId);
    }
}

void GioNet::Connection::EnqueueProcessablePackets()
{
    Packet::IdType latestReceivedPacketId = GetCurrentAckId();
    auto nextExpectedPacketId = incoming.sequenceNumber;

    while(nextExpectedPacketId != latestReceivedPacketId)
    {
        nextExpectedPacketId++;

        if(nextExpectedPacketId == Packet::InvalidId)
            ++nextExpectedPacketId;

        auto nextPacket = incoming.reliablePackets.find(nextExpectedPacketId);

        if(nextPacket == incoming.reliablePackets.end())
            break;

        incoming.sequenceNumber = nextExpectedPacketId;
        incoming.readyPackets.push(nextPacket->second);
    }
}
