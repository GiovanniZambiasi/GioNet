#include "TCP/ClientTCP.h"

#include "Buffer.h"
#include "Core.h"
#include "Socket.h"

GioNet::ClientTCP::ClientTCP(const NetAddress& address)
    : Client(std::make_shared<Socket>(address, CommunicationProtocols::TCP))
{
}

void GioNet::ClientTCP::Start()
{
    Client::Start();

    GIONET_LOG("Starting TCP client...\n");
    
    std::shared_ptr<Socket> socket = GetSocket();
    if(socket && socket->IsValid() && socket->Connect())
    {
        GIONET_LOG("Successfully connected to server!\n");
        RunListenThread();
    }
    else
    {
        GIONET_LOG("Connection failed.\n");
        Stop();
    }
}

std::optional<GioNet::Buffer> GioNet::ClientTCP::DoReceive()
{
    return GetSocketChecked().Receive();
}

void GioNet::ClientTCP::Send(const Buffer& buffer)
{
    GetSocketChecked().Send(buffer);
}
