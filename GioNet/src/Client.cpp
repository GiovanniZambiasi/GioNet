#include "Client.h"
#include <assert.h>
#include "Socket.h"

GioNet::Client::Client(const std::shared_ptr<Socket>& socket)
    : socket(socket)
{
}

void GioNet::Client::RunListenThread()
{
    listenThread = std::thread{&Client::ListenThreadImpl, this};
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
    assert(socket && socket->IsValid());
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

void GioNet::Client::ListenThreadImpl()
{
    while (socket && socket->IsValid())
    {
        char buffer[GIONET_DEFAULT_BUFFER];
        int bytesReceived = socket->Receive(&buffer[0], sizeof(buffer));

        if (bytesReceived > 0)
        {
            // YAY! DATA!
            printf("Received data from server: %s\n", buffer);
        }
        else if(bytesReceived == SOCKET_ERROR)
        {
            Stop();
        }
    }
}
