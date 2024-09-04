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
        std::shared_ptr<GioNet::Server> server = sys.StartServer(GIONET_DEFAULT_PORT, protocol);
        server->Start();
        while (true)
        {
            
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
        std::shared_ptr<GioNet::Client> client = sys.StartClient(serverIpLoc->second.c_str(), GIONET_DEFAULT_PORT, protocol);
        client->SayHello();
        while (true)
        {
            
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

