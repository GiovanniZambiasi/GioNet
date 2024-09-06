#include "UDP/ClientUDP.h"

#include "Core.h"
#include "Socket.h"

GioNet::ClientUDP::ClientUDP(const NetAddress& address)
    : Client(std::make_shared<Socket>(address, CommunicationProtocols::UDP))
{
}

void GioNet::ClientUDP::Start()
{
    Client::Start();
    // No implicit connection. Implement connection communication via messages
    RunListenThread();
}
