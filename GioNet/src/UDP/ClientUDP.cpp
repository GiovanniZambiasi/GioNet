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

    std::shared_ptr<Socket> socket = GetSocket();
    if(socket && socket->IsValid())
    {
        GIONET_LOG("Starting UDP client...\n");
        // UDP Client needs to initiate communication before attempting to listen from socket
        Send({"Greetings!"});
        RunListenThread();
    }
    else
    {
        GIONET_LOG("Connection failed...\n");
        Stop();
    }
}

std::optional<GioNet::Buffer> GioNet::ClientUDP::DoReceive()
{
    return GetSocketChecked().ReceiveFrom();
}

void GioNet::ClientUDP::DoSend(const Buffer& buffer)
{
    GetSocketChecked().SendTo(buffer);
}
