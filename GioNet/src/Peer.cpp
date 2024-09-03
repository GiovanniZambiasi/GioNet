#include "Peer.h"

#include <format>

std::string GioNet::Peer::ToString() const
{
    return std::format("(SOCKET:{} IP:{}:{})", windowsSocket, ip, port);
}

bool GioNet::Peer::IsValid() const
{
    return windowsSocket != INVALID_SOCKET;
}

bool GioNet::Peer::operator==(const Peer& other) const
{
    return windowsSocket == other.windowsSocket;
}
