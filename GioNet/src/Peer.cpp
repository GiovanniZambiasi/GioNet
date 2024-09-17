#include "Peer.h"

#include "Socket.h"

std::string GioNet::Peer::ToString() const
{
    return connection ? connection->ToString() : address.ToString();
}

std::size_t std::hash<GioNet::Peer>::operator()(const GioNet::Peer& peer) const noexcept
{
    return std::hash<GioNet::NetAddress>()(peer.address);
}