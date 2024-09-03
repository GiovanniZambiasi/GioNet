#include "Client.h"

#include <assert.h>
#include "Socket.h"

GioNet::Client::Client(const std::shared_ptr<Socket>& socket)
    : socket(socket)
{
}

GioNet::Client::~Client()
{
    listenThread.detach();
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

void GioNet::Client::ReceiveLoop()
{
    int received;
    do
    {
        char buffer[GIONET_DEFAULT_BUFFER];
        received = socket->Receive(&buffer[0], sizeof(buffer));
        printf("Received data from server: '%s'\n", &buffer[0]);
    }
    while (received != SOCKET_ERROR);
}
