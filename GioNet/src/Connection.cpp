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
    std::scoped_lock _{outgoing.lock};
    
    if(packet.HasFlags(Packet::Flags::Reliable))
    {
        Packet::IdType index = GetIndexForNextPacket();
        
        if(outgoing.reliablePackets.contains(index))
        {
            // TODO - Put packets in an aux queue until indices are available again? 
            GIONET_LOG("[ERROR]: Packet already exists with id %i", index);
            return;
        }
        
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
        // TODO - Add ack header
    }
        
    return packet;
}

void GioNet::Connection::Received(Packet&& packet)
{
    std::scoped_lock _{incoming.lock};
    
    // TODO - Check incoming acks, remove unnecessary outgoing packets
    
    if(packet.HasFlags(Packet::Flags::Reliable))
    {
        if(incoming.reliablePackets.contains(packet.id))
            return;

        auto packetId = packet.id;
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

void GioNet::Connection::EnqueueProcessablePackets()
{
    auto nextExpectedPacketId = incoming.sequenceNumber;

    while(true)
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
