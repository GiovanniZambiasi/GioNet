#include "GioNet.h"
#include "gtest/gtest.h"

class ConnectionTests : public testing::Test
{
public:
    std::shared_ptr<GioNet::Server> server{};

    std::vector<std::shared_ptr<GioNet::Client>> clients{};

    bool AllClientsConnected() const
    {
        return std::ranges::all_of(clients, [](const std::shared_ptr<GioNet::Client>& client)
        {
            return client->IsConnected();
        }) && server->GetConnectedPeerCout() == clients.size();
    }

    void StartupServerAndClients(int clientCount)
    {
        auto& NetSys = GioNet::NetSystem::Get();
        server = NetSys.CreateServer(GIONET_DEFAULT_PORT);
        server->Start();
        
        for (int i = 0; i < clientCount; ++i)
        {
            std::shared_ptr<GioNet::Client> client = NetSys.CreateClient("localhost", GIONET_DEFAULT_PORT);
            clients.push_back(client);
            client->Start();
        }
        
        auto start = std::chrono::system_clock::now();
        while (!AllClientsConnected())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{16});
            auto now = std::chrono::system_clock::now();
            auto elapsed{std::chrono::duration_cast<std::chrono::seconds>(start - now).count()};

            if (elapsed > 10.f)
            {
                ADD_FAILURE("Connection timed out");
            }
        }
    }

    void ShutdownServerAndClients()
    {
        server.reset();
        clients.clear();
    }
};

TEST_F(ConnectionTests, udp_connection)
{
    StartupServerAndClients(1);
    ShutdownServerAndClients();
    StartupServerAndClients(3);
}
