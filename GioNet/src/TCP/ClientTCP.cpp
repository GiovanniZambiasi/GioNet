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
    printf("Starting TCP client...\n");

    Socket& socket = GetSocketChecked();
    if(socket.Connect())
    {
        printf("Successfully connected to server!\n");
        RunListenThread();
        socket.Send(Buffer{"Ping!"});
    }
    else
    {
        printf("Connection failed.\n");
    }
}

std::optional<GioNet::Buffer> GioNet::ClientTCP::DoReceive()
{
    return GetSocketChecked().Receive();
}
