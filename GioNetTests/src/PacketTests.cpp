#include <gtest/gtest.h>
#include "GioNet.h"

class PacketHeaderTest
{
public:
    void Test(uint16_t id, GioNet::Packet::Type type)
    {
        GioNet::Packet packet{id, type};
        ASSERT_EQ(id, packet.GetId());
        ASSERT_EQ(type, packet.GetType());
    }
};

TEST(Packet, header_encoding)
{
    PacketHeaderTest t{};
    t.Test(255, GioNet::Packet::Type::Ack);
    t.Test(GioNet::Packet::GetMaximumViableId(), GioNet::Packet::Type::Connect);
    t.Test(0, GioNet::Packet::Type::Data);
}