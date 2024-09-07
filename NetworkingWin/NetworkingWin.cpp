#include <iostream>
#include <unordered_map>

#include "GioNet.h"


std::unordered_map<std::string, std::string> args{};
GioNet::CommunicationProtocols protocol = GioNet::CommunicationProtocols::UDP;

void ParseArgs(int argC, char* argV[]);

int main(int argC, char* argV[])
{
    ParseArgs(argC, argV);
    std::string netMode = args["mode"];
    
    if(netMode == "server")
    {
        auto& sys = GioNet::NetSystem::Get();
        std::shared_ptr<GioNet::Server> server = sys.CreateServer(GIONET_DEFAULT_PORT, protocol);
        server->Start();
        server->BindDataReceived([](const GioNet::Peer& peer, GioNet::Buffer&& buff)
        {
            printf("Data received from peer (%i): %s\n", peer.address.port, buff.Data());
        });
        while(server && server->IsRunning())
        {
            server->Broadcast({"Pong!"});
            std::this_thread::sleep_for(std::chrono::seconds{1});
        }
    }
    else if(netMode == "client")
    {
        auto serverIpLoc = args.find("server_ip");
        if(serverIpLoc == args.end())
        {
            printf("[ERROR] No server ip has been specified\n");
            return 1;
        }
        
        auto& sys = GioNet::NetSystem::Get();
        std::shared_ptr<GioNet::Client> client = sys.CreateClient(serverIpLoc->second.c_str(), GIONET_DEFAULT_PORT, protocol);
        client->Start();
        client->BindDataReceived([](GioNet::Buffer&& buffer)
        {
            printf("Data received from server: %s\n", buffer.Data());
        });
        while(client && client->IsConnected())
        {
            client->Send({"Ping!"});
            std::this_thread::sleep_for(std::chrono::seconds{1});
        }
    }
    else
    {
        printf("[ERROR] No execution mode has been specified\n");
        return 1;
    }

}

void ParseArgs(int argC, char* argV[])
{
    static constexpr char expectedPrefix = '-';  
    static constexpr char nameValSeparationToken = ':';
    
    for (int i = 0; i < argC; ++i)
    {
        std::string arg{argV[i]};

        if(!arg.starts_with(expectedPrefix))
        {
            continue;
        }

        size_t tokenPos = arg.find(nameValSeparationToken);
        std::string key = arg.substr(1, tokenPos - 1);
        std::string val = arg.substr(tokenPos + 1);
        args[key] = val;
    }
}

