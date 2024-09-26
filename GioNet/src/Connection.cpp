#include "Connection.h"

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

std::optional<GioNet::Packet> GioNet::Connection::GetReadyOutgoingPacket()
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
    // TODO - Check for acks, remove unnecessary outgoing packets
    
    if(packet.HasFlags(Packet::Flags::Reliable))
    {
        if(incoming.reliablePackets.contains(packet.id))
            return;

        incoming.readyPackets.push(packet);
        incoming.reliablePackets[packet.id] = std::move(packet);
    }
    else
    {
        incoming.readyPackets.push(std::move(packet));
    }
}

std::optional<GioNet::Packet> GioNet::Connection::GetReadyIncomingPacket()
{
    return incoming.PopNextReadyPacket();
}

std::string GioNet::Connection::ToString() const
{
    return address.ToString();
}

GioNet::Packet::IdType GioNet::Connection::GetIndexForNextPacket()
{
    return ++outgoing.sequenceNumber;
}