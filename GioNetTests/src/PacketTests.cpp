#include <gtest/gtest.h>
#include "GioNet.h"

class PacketHeaderTest
{
public:
    void Test(GioNet::Packet::Types type, std::initializer_list<GioNet::Packet::Flags> flags = {})
    {
        GioNet::Packet packet{type, flags};
        ASSERT_EQ(type, packet.GetType());
        for (GioNet::Packet::Flags flag : flags)
        {
            ASSERT_TRUE(packet.HasFlag(flag));
        }
    }
};

TEST(Packet, header_encoding)
{
    PacketHeaderTest t{};
    t.Test(GioNet::Packet::Types::Ack, { });
    t.Test(GioNet::Packet::Types::Connect, {GioNet::Packet::Flags::Fragmented, GioNet::Packet::Flags::Reliable});
    t.Test(GioNet::Packet::Types::Data, {GioNet::Packet::Flags::Reliable});
}
