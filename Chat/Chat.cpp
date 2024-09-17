#include <format>
#include <fstream>
#include <iostream>
#include <conio.h>
#include "GioNet.h"

constexpr int MaxMessageLength{100};

std::unordered_map<std::string, std::string> args{};

class ChatHost
{
    std::string prompt{};
    std::shared_mutex promptLock{};
    
protected:
    std::string username{};

    void PrintMessageHeader(std::string_view name)
    {
        std::cout << "[" << name << "]: ";
    }
    
    void PrintMessage(std::string_view view)
    {
        std::cout << "\r" << view;
        
        int whitespaceCount = MaxMessageLength - static_cast<int>(view.length());
        for (int i = 0; i < whitespaceCount; ++i)
        {
            std::cout << " ";
        }

        std::cout << "\n" << std::flush;
        PrintMessageHeader(username);
        std::cout << GetPrompt();
    }

    std::string ReadInput()
    {
        ClearPrompt();

        bool finished{false};
        do
        {
            char c = _getch();
            finished = (c == '\r' || c == '\n');

            if (c == '\b' && GetPrompt().empty())
            {
                continue;
            }
            
            std::cout << c << std::flush;

            if(!finished)
            {
                AppendToPrompt(c);
            }
            
        } while(!finished);

        std::cout << "\n";
        
        return GetPrompt();
    }

    std::string Format(std::string_view message) const
    {
        return std::format("[{}]: {}", username, message);
    }

    std::string GetPrompt()
    {
        std::shared_lock _{promptLock};
        return prompt;
    }

    void ClearPrompt()
    {
        std::unique_lock _{promptLock};
        prompt.clear();
    }

    void AppendToPrompt(char c)
    {
        std::unique_lock _{promptLock};

        if(c == '\b' && prompt.size() > 0)
        {
            prompt.erase(prompt.end() - 1);
            std::cout << " \b";
        }
        else
            prompt += c;
    }
};

class ChatServer : public ChatHost
{
    std::shared_ptr<GioNet::Server> sv{};

public:
    ChatServer()
    {
        sv = GioNet::NetSystem::Get().CreateServer(GIONET_DEFAULT_PORT);
        username = "SERVER";
    }

    void Run()
    {
        sv->BindDataReceived([this](const GioNet::Peer& peer, GioNet::Buffer&& buff)
        {
            DataReceived(peer, std::move(buff));
        });
        sv->BindPeerConnected([this](const GioNet::Peer& peer)
        {
            PeerConnected(peer);
        });
        sv->BindPeerDisconnected([this](const GioNet::Peer& peer)
        {
            PeerDisconnected(peer);
        });
        sv->Start();
        
        while (sv && sv->IsRunning())
        {
            PrintMessageHeader(username);
            std::string input{ReadInput()};
            Send(input);
        }
    }

    void DataReceived(const GioNet::Peer& peer, GioNet::Buffer&& buff)
    {
        std::string message = buff.Read<std::string>();
        PrintMessage(message);
        sv->Broadcast({message}, { peer });
    }

    void PeerConnected(const GioNet::Peer& peer)
    {
        Send("New peer connected...");
    }

    void PeerDisconnected(const GioNet::Peer& peer)
    {
        Send("Peer connected...");
    }

    void Send(std::string_view message)
    {
        if(message.length() > MaxMessageLength)
        {
            printf("[ERROR]: Max message length exceeded.\n");
            return;
        }
        
        std::string formattedMessage = Format(message);
        sv->Broadcast({formattedMessage});
    }
};

class ChatClient : public ChatHost
{
    std::shared_ptr<GioNet::Client> client{};
    
public:
    ChatClient()
    {
        std::cout << "What is your name? ";
        username = ReadInput();
    }

    void Run()
    {
        std::cout << "What is the server ip? ";
        std::string serverIp{ReadInput()};
        client = GioNet::NetSystem::Get().CreateClient(serverIp.c_str(), GIONET_DEFAULT_PORT);
        client->BindDataReceived([this](GioNet::Buffer&& buff)
        {
            DataReceived(std::move(buff));
        });
        client->Start();
        
        if(client->IsConnected())
        {
            Loop();    
        }
        else
        {
            std::cout << "Could not connect to server\n";
        }
    }

    void Loop()
    {
        while (client && client->IsConnected())
        {
            PrintMessageHeader(username);
            std::string input{ReadInput()};
            client->Send({Format(input)});
        }
    }

    void DataReceived(GioNet::Buffer&& data)
    {
        std::string message = data.Read<std::string>();
        PrintMessage(message);
    }
};

void ParseArgs(int argC, char* argV[]);

void StartClient()
{
    std::cout << "Welcome to Gio chat!\n";
    ChatClient c{};
    c.Run();
}

void StartServer()
{
    ChatServer s{};
    s.Run();
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

int main(int argc, char* argv[])
{
    ParseArgs(argc, argv);
    std::string netMode = args["mode"];

    if(netMode == "client")
    {
        StartClient();
    }
    else
    {
        StartServer();
    }
}
