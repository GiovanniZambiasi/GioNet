#include "Client.h"

#include "Buffer.h"
#include "Socket.h"

GioNet::Client::Client(const NetAddress& serverAddress)
    : serverAddress(serverAddress),
      socket(std::make_shared<Socket>(serverAddress, CommunicationProtocols::UDP))
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

void GioNet::Client::Stop()
{
    if(socket->IsValid())
    {
        socket->Close();
    }

    listenThread.request_stop();
}

void GioNet::Client::Send(const Buffer& buffer)
{
    if(!IsConnected())
    {
        GIONET_LOG("[ERROR]: Tried to send buffer, but client is not connected");
        return;
    }

    GetSocketChecked().SendTo(buffer);
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
        NetAddress addr{};
        std::optional<Buffer> received = socket->ReceiveFrom(&addr);

        if(addr != serverAddress)
            continue;
        
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
