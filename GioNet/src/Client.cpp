#include "Client.h"

#include "Buffer.h"
#include "Message.h"
#include "Socket.h"

GioNet::Client::Client(const NetAddress& serverAddress)
    : serverAddress(serverAddress),
      socket(std::make_shared<Socket>(serverAddress, CommunicationProtocols::UDP)),
      connectionToServer(std::make_shared<Connection>(serverAddress))
{
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
        socket->SendTo({Packet{Packet::Types::Ping}});
        sendThread = std::jthread{&Client::SendThread, this};
        listenThread = std::jthread{&Client::ListenThread, this};
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
    sendThread.request_stop();
}

void GioNet::Client::Send(Message&& message)
{
    if(!IsConnected())
    {
        GIONET_LOG("[ERROR]: Tried to send buffer, but client is not connected");
        return;
    }

    connectionToServer->Schedule(
        {
            Packet::Types::Data,
            message.reliable ? Packet::Flags::Reliable : Packet::Flags::None,
            std::move(message.data)
        });
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

void GioNet::Client::ListenThread()
{
    std::shared_ptr socketCopy{socket};
    while (socketCopy && socketCopy->IsValid() && !listenThread.get_stop_token().stop_requested())
    {
        NetAddress addr{};
        std::optional<Buffer> received = socketCopy->ReceiveFrom(&addr);

        if(addr != serverAddress)
            continue;
        
        if (received)
        {
            Packet packet{received->Read<Packet>()};
            received.reset();
            connectionToServer->Received(std::move(packet));

            ProcessIncomingPacket();
        }
        else
        {
            Stop();
        }
    }
}

void GioNet::Client::ProcessIncomingPacket()
{
    while (std::optional<Packet> incomingPacket = connectionToServer->PopReadyIncomingPacket())
    {
        InvokeDataReceived(std::move(incomingPacket->payload));
    }
}

void GioNet::Client::SendThread()
{
    std::shared_ptr socketCopy{socket};
    while (socketCopy && socketCopy->IsValid() && !listenThread.get_stop_token().stop_requested())
    {
        while (std::optional<Packet> outgoingPacket = connectionToServer->PopReadyOutgoingPacket())
        {
            Buffer b{};
            b.Write(outgoingPacket.value());
            socketCopy->SendTo(b);
        }
    }
}
