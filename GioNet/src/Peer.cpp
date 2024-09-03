#include "Peer.h"

#include <format>

std::string GioNet::Peer::ToString() const
{
    return std::format("(SOCKET:{} IP:{}:{})", windowsSocket, ip, port);
}
