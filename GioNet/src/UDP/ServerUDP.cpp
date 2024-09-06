#include "UDP/ServerUDP.h"


GioNet::ServerUDP::ServerUDP(unsigned short port)
    : Server(std::make_shared<Socket>(NetAddress{"", port}, CommunicationProtocols::UDP))
{
}

GioNet::ServerUDP::~ServerUDP()
{
    listenThread.detach();
}

void GioNet::ServerUDP::Start()
{
    Server::Start();
    printf("Starting UDP server...\n");
    listenThread = std::thread{&ServerUDP::RunListenThread, this};
}

void GioNet::ServerUDP::RunListenThread()
{
    std::shared_ptr<Socket> socket = GetSocket();
    while (socket && socket->IsValid())
    {
        char buffer[GIONET_DEFAULT_BUFFER];
        NetAddress source{};
        int bytesReceived = socket->Receive(buffer, sizeof(buffer), &source);

        if(bytesReceived > 0)
        {
            printf("Received data: %s\n", buffer);
            
            const char* greeting = "Greetings from server!";
            socket->Send(greeting, 1 + strlen(greeting), source);
            // Should probably add connection to source
        }
        else
        {
            Stop();
        }
    }
}
