#include "gtest/gtest.h"
#include "GioNet.h"

TEST(ReliabilityTests, outgoing_sequencing)
{
    GioNet::Connection connection{};

    static constexpr int N = 10;

    for (int i = 0; i < N; ++i)
    {
        connection.Schedule({GioNet::Packet::Types::Data, GioNet::Packet::Flags::Reliable});
    }

    GioNet::Packet::IdType previousId{GioNet::Packet::InvalidId};
    
    for (int i = 0; i < N; ++i)
    {
        std::optional<GioNet::Packet> packet = connection.PopReadyOutgoingPacket();
        ASSERT_TRUE(packet);
        ASSERT_NE(packet->id, GioNet::Packet::InvalidId);

        if(previousId != GioNet::Packet::InvalidId)
        {
            ASSERT_GT(packet->id, previousId);
        }

        previousId = packet->id;
    }
}

TEST(ReliabilityTests, outgoing_sequencing_overflow)
{
    GioNet::Connection connection{};
    connection.SetOutgoingSequenceNumber(GioNet::Packet::MaxPossibleId);
    connection.Schedule({GioNet::Packet::Types::Data, GioNet::Packet::Flags::Reliable});
    std::optional<GioNet::Packet> packet = connection.PopReadyOutgoingPacket();
    ASSERT_TRUE(packet);
    ASSERT_NE(packet->id, GioNet::Packet::InvalidId);
}

TEST(ReliabilityTests, incoming_sequencing)
{
    GioNet::Connection connection{};
    
    GioNet::Packet packet{GioNet::Packet::Types::Data, GioNet::Packet::Flags::Reliable};
    packet.id = 3;
    connection.Received(GioNet::Packet{packet});
    packet.id = 1;
    connection.Received(GioNet::Packet{packet});
    packet.id = 2;
    connection.Received(GioNet::Packet{packet});

    GioNet::Packet::IdType expectedId = 1;
    std::optional<GioNet::Packet> processedPacket;
    
    processedPacket = connection.PopReadyIncomingPacket();
    ASSERT_TRUE(processedPacket);
    ASSERT_EQ(processedPacket->id, expectedId);

    processedPacket = connection.PopReadyIncomingPacket();
    ASSERT_TRUE(processedPacket);
    ASSERT_EQ(processedPacket->id, ++expectedId);
    
    processedPacket = connection.PopReadyIncomingPacket();
    ASSERT_TRUE(processedPacket);
    ASSERT_EQ(processedPacket->id, ++expectedId);
}

TEST(ReliabilityTests, incoming_sequencing_multiple_copies_same_id)
{
    GioNet::Connection connection{};
    
    GioNet::Packet packet{GioNet::Packet::Types::Data, GioNet::Packet::Flags::Reliable};
    packet.id = 1;
    connection.Received(GioNet::Packet{packet});
    connection.Received(GioNet::Packet{packet});
    connection.Received(GioNet::Packet{packet});

    GioNet::Packet::IdType expectedId = 1;
    std::optional<GioNet::Packet> processedPacket = connection.PopReadyIncomingPacket();
    ASSERT_TRUE(processedPacket);
    ASSERT_EQ(processedPacket->id, expectedId);

    processedPacket = connection.PopReadyIncomingPacket();
    ASSERT_FALSE(processedPacket);
}

/*
 * Tests that when the incoming packet stream overflows packet id, packet 1 comes after max packet id
 */
TEST(ReliabilityTests, incoming_sequencing_overflow)
{
    GioNet::Connection connection{};
    // Fake having received MaxPossibleIds - 1 packets (so connection should expect next packet id to be MaxPossibleId)
    connection.SetIncomingSequenceNumber(GioNet::Packet::MaxPossibleId - 1);
    
    GioNet::Packet packet{GioNet::Packet::Types::Data, GioNet::Packet::Flags::Reliable};
    packet.id = GioNet::Packet::MaxPossibleId;
    connection.Received(GioNet::Packet{packet});
    packet.id = 1;
    connection.Received(GioNet::Packet{packet});

    std::optional<GioNet::Packet> processedPacket = connection.PopReadyIncomingPacket();
    ASSERT_TRUE(processedPacket && processedPacket->id == GioNet::Packet::MaxPossibleId);
    
    processedPacket = connection.PopReadyIncomingPacket();
    ASSERT_TRUE(processedPacket && processedPacket->id == 1);
}