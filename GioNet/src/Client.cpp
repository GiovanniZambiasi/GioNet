#include "Client.h"

#include <assert.h>
#include "Socket.h"

GioNet::Client::Client(const NetAddress& address)
{
    socket = std::make_shared<Socket>(address, CommunicationProtocols::TCP);
}

GioNet::Client::~Client()
{
    Disconnect();
}

void GioNet::Client::SayHello()
{
    const char* buffer = "Hello from client!";
    socket->Send(buffer, strlen(buffer) + 1);
}

void GioNet::Client::Connect()
{
    if(socket->Connect())
    {
        printf("Successfully connected to server\n");
        listenThread = std::thread{&Client::ReceiveLoop, this};
    }
    else
    {
        printf("Connection failed!\n");
    }
}

void GioNet::Client::Disconnect()
{
    if(socket->IsValid())
    {
        socket->Close();
    }

    listenThread.detach();
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
