#include <iostream>
#include <unordered_map>

#include "GioNet.h"


std::unordered_map<std::string, std::string> args{};
GioNet::CommunicationProtocols protocol = GioNet::CommunicationProtocols::TCP;

constexpr int BenchmarkCount{1000};

void ParseArgs(int argC, char* argV[]);

void RunServer()
{
    auto& sys = GioNet::NetSystem::Get();
    std::shared_ptr<GioNet::Server> server = sys.CreateServer(GIONET_DEFAULT_PORT, protocol);
    server->Start();

    std::chrono::time_point<std::chrono::system_clock> start{};
    int count = 0;
    
    server->BindDataReceived([&count, &start](const GioNet::Peer& peer, GioNet::Buffer&& buff)
    {
        //printf("Data received from peer (%i): %s\n", peer.address.port, buff.Data());
        if(count == 0)
        {
            start = std::chrono::system_clock::now();
        }
        
        ++count;
    });
    while(server && server->IsRunning() && count < BenchmarkCount)
    {
        // server->Broadcast({"Pong!"});
        std::this_thread::sleep_for(std::chrono::seconds{1});
    }

    std::chrono::time_point end{std::chrono::system_clock::now()};
    
    std::chrono::seconds duration{ std::chrono::duration_cast<std::chrono::seconds>(end - start) };
    printf("Server finished receiving %i messages in %lld seconds\n", count, duration.count());
}

bool RunClient()
{
    auto serverIpLoc = args.find("server_ip");
    if(serverIpLoc == args.end())
    {
        printf("[ERROR] No server ip has been specified\n");
        return false;
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
    }
    return true;
}

int main(int argC, char* argV[])
{
    ParseArgs(argC, argV);
    std::string netMode = args["mode"];
    
    if(netMode == "server")
    {
        RunServer();
    }
    else if(netMode == "client")
    {
        if(!RunClient())
            return 1;
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

