#include <gtest/gtest.h>
#include "GioNet.h"

class PacketHeaderTest
{
public:
    void Test(GioNet::Packet::Types type, GioNet::Packet::Flags flags = {})
    {
        GioNet::Packet packet{type, flags};
        ASSERT_EQ(type, packet.GetType());
        ASSERT_EQ(packet.GetFlags(), flags);
    }
};

TEST(Packet, header_encoding)
{
    PacketHeaderTest t{};
    t.Test(GioNet::Packet::Types::Connect, GioNet::Packet::Flags::None);
    t.Test(GioNet::Packet::Types::Connect, GioNet::Packet::Flags::Reliable);
    t.Test(GioNet::Packet::Types::Data, GioNet::Packet::Flags::Reliable);
}
