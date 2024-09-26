#include "Connection.h"

#include "Socket.h"

GioNet::Connection::Connection(const NetAddress& address)
    : address(address)
{
}

void GioNet::Connection::Schedule(Packet&& packet)
{
    std::scoped_lock _{outgoingPacketLock};
    
    if(packet.HasFlags(Packet::Flags::Reliable))
    {
        Packet::IdType index = GetIndexForNextPacket();
        
        if(sentReliablePackets.contains(index))
        {
            // TODO - Put packets in an aux queue until indices are available again? 
            GIONET_LOG("[ERROR]: Packet already exists with id %i", index);
            return;
        }
        
        packet.id = index;
        sentReliablePackets[index] = packet;
        readyOutgoingPackets.push(std::move(packet));
    }
    else
    {
        readyOutgoingPackets.push(std::move(packet));
    }
}

std::optional<GioNet::Packet> GioNet::Connection::GetReadyOutgoingPacket()
{
    std::scoped_lock _{outgoingPacketLock};
    
    if(readyOutgoingPackets.empty())
        return {};

    Packet packet = std::move(readyOutgoingPackets.front());
    readyOutgoingPackets.pop();
    // TODO - Add ack header
    return {std::move(packet)};
}

void GioNet::Connection::Received(Packet&& packet)
{
    std::scoped_lock _{incomingPacketLock};
    // TODO - Check for acks, remove unnecessary outgoing packets
    
    if(packet.HasFlags(Packet::Flags::Reliable))
    {
        if(receivedReliablePackets.contains(packet.id))
            return;

        readyIncomingPackets.push(packet);
        receivedReliablePackets[packet.id] = std::move(packet);
    }
    else
    {
        readyIncomingPackets.push(std::move(packet));
    }
}

std::optional<GioNet::Packet> GioNet::Connection::GetReadyIncomingPacket()
{
    std::scoped_lock _{incomingPacketLock};
    
    if(readyIncomingPackets.empty())
        return {};

    Packet packet = std::move(readyIncomingPackets.front());
    readyIncomingPackets.pop();
    return {std::move(packet)};
}

std::string GioNet::Connection::ToString() const
{
    return address.ToString();
}

GioNet::Packet::IdType GioNet::Connection::GetIndexForNextPacket()
{
    return ++localPacketIndex;
}