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

void GioNet::Client::BindDataReceived(DataReceivedDelegate&& delegate)
{
    dataReceived = std::move(delegate);
}

void GioNet::Client::InvokeDataReceived(Buffer&& buffer)
{
    if(dataReceived)
        dataReceived(std::move(buffer));
}

void GioNet::Client::ListenThreadImpl()
{
    while (socket && socket->IsValid() && !listenThread.get_stop_token().stop_requested())
    {
        std::optional<Buffer> received = DoReceive();

        if (received)
        {
            InvokeDataReceived(std::move(*received));
            received.reset();
        }
        else
        {
            Stop();
        }
    }
}
