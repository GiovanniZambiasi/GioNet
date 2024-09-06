#include "Client.h"

#include <assert.h>
#include "Socket.h"

GioNet::Client::Client(const NetAddress& address, CommunicationProtocols protocol)
{
    socket = std::make_shared<Socket>(address, protocol);
}

GioNet::Client::~Client()
{
    Stop();
}

void GioNet::Client::SayHello()
{
    const char* buffer = "Hello from client!";
    socket->Send(buffer, strlen(buffer) + 1);
}

void GioNet::Client::Start()
{
    switch (socket->GetProtocol())
    {
    case CommunicationProtocols::TCP:
        if(socket->Connect())
        {
            printf("Successfully connected to server\n");
            listenThread = std::thread{&Client::ReceiveLoop, this};
        }
        else
        {
            printf("Connection failed!\n");
        }
        break;
    case CommunicationProtocols::UDP:
        printf("Client started receiving datagrams..\n");
        listenThread = std::thread{&Client::ReceiveLoop, this};
        break;
    default:
        printf("[ERROR]: Unimplemented protocol\n");
        break;
    }
}

void GioNet::Client::Stop()
{
    if(socket->IsValid())
    {
        socket->Close();
    }

    listenThread.detach();
}

bool GioNet::Client::IsConnected() const
{
    return socket->IsValid();
}

void GioNet::Client::ReceiveLoop()
{
    int received;
    do
    {
        char buffer[GIONET_DEFAULT_BUFFER];
        received = socket->Receive(&buffer[0], sizeof(buffer));
        if(received > 0)
        {
            printf("Received data from server: '%s'\n", &buffer[0]);
        }
        else if(!socket->IsValid())
        {
            break;
        }
    }
    while (received != SOCKET_ERROR);

    printf("Ending receive loop because connection was lost\n");
}
