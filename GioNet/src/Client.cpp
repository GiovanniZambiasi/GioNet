#include "Client.h"

#include "Buffer.h"
#include "Socket.h"

GioNet::Client::Client(const std::shared_ptr<Socket>& socket)
    : socket(socket)
{
}

void GioNet::Client::RunListenThread()
{
    listenThread = std::jthread{&Client::ListenThreadImpl, this};
}

GioNet::Client::~Client()
{
    Stop();
}

void GioNet::Client::Start()
{
}

void GioNet::Client::Stop()
{
    if(socket->IsValid())
    {
        socket->Close();
    }

    listenThread.request_stop();
}

bool GioNet::Client::IsConnected() const
{
    return socket->IsValid();
}

void GioNet::Client::ListenThreadImpl()
{
    while (socket && socket->IsValid() && !listenThread.get_stop_token().stop_requested())
    {
        std::optional<Buffer> received = DoReceive();

        if (received)
        {
            // YAY! DATA!
            GIONET_LOG("Received data from server: %s\n", received->Data());
        }
        else
        {
            Stop();
        }
    }
}
