#include "Client.h"

#include "Buffer.h"
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
        std::optional<Buffer> received = DoReceive();

        if (received)
        {
            // YAY! DATA!
            printf("Received data from server: %s\n", received->Data());
        }
        else
        {
            Stop();
        }
    }
}
