#include "GioNet.h"
#include "gtest/gtest.h"

class ConnectionTests : public testing::Test
{
public:
    std::shared_ptr<GioNet::Server> server{};

    std::shared_ptr<GioNet::Client> client{};

    ConnectionTests()
    {
        auto& NetSys = GioNet::NetSystem::Get();
        server = NetSys.CreateServer(GIONET_DEFAULT_PORT);
        server->Start();

        client = NetSys.CreateClient("localhost", GIONET_DEFAULT_PORT);
        client->Start();
    }

    void WaitForConnection()
    {
        auto start = std::chrono::system_clock::now();
        while (!client->IsConnected())
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
};

TEST_F(ConnectionTests, udp_connection)
{
    WaitForConnection();
}
