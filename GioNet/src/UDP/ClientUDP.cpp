#include "UDP/ClientUDP.h"

#include "Core.h"
#include "Socket.h"
#include "Buffer.h"

GioNet::ClientUDP::ClientUDP(const NetAddress& address)
    : Client(std::make_shared<Socket>(address, CommunicationProtocols::UDP))
{
}

void GioNet::ClientUDP::Start()
{
    Client::Start();
    printf("Starting UDP client...\n");
    RunListenThread();
}

std::optional<GioNet::Buffer> GioNet::ClientUDP::DoReceive()
{
    return GetSocketChecked().ReceiveFrom();
}

void GioNet::ClientUDP::Send(const Buffer& buffer)
{
    GetSocketChecked().SendTo(buffer);
}
