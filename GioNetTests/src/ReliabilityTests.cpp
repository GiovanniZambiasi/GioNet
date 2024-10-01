#include <bitset>

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

TEST(ReliabilityTests, incoming_ack)
{
    GioNet::Connection connection{};
    GioNet::Packet packet{GioNet::Packet::Types::Data, GioNet::Packet::Flags::Reliable};
    packet.id = 1;
    connection.Received(GioNet::Packet{packet});
    ASSERT_EQ(connection.GetCurrentAckId(), 1);
    
    packet.id = 4;
    connection.Received(GioNet::Packet{packet});
    ASSERT_EQ(connection.GetCurrentAckId(), 4);

    packet.id = 2;
    connection.Received(GioNet::Packet{packet});
    ASSERT_EQ(connection.GetCurrentAckId(), 4);
}

TEST(ReliabilityTests, incoming_ack_overflow)
{
    GioNet::Connection connection{};
    GioNet::Packet packet{GioNet::Packet::Types::Data, GioNet::Packet::Flags::Reliable};
    packet.id = GioNet::Packet::MaxPossibleId;
    connection.Received(GioNet::Packet{packet});
    ASSERT_EQ(connection.GetCurrentAckId(), GioNet::Packet::MaxPossibleId);

    packet.id = 1;
    connection.Received(GioNet::Packet{packet});
    ASSERT_EQ(connection.GetCurrentAckId(), 1);
}

class AckBitsetTest
{
public:
    void Test(const std::initializer_list<GioNet::Packet::IdType>& ids, GioNet::Packet::IdType expectedAck, GioNet::Packet::AckBitsetType expectedBitset)
    {
        GioNet::Connection connection{};
        ReceivePackets(connection, ids);

        CreateAckHeaderAndCompare(connection, expectedAck, expectedBitset);
    }

    void ReceivePackets(GioNet::Connection& connection, const std::initializer_list<GioNet::Packet::IdType>& ids)
    {
        GioNet::Packet packet{GioNet::Packet::Types::Data, GioNet::Packet::Flags::Reliable};

        for (GioNet::Packet::IdType id : ids)
        {
            packet.id = id;
            connection.Received(GioNet::Packet{packet});
        }
    }

    void CreateAckHeaderAndCompare(GioNet::Connection& connection, GioNet::Packet::IdType expectedAck, GioNet::Packet::AckBitsetType expectedBitset)
    {
        GioNet::Packet packet{GioNet::Packet::Types::Data, GioNet::Packet::Flags::Reliable};
        connection.AddAckHeader(packet);
        
        ASSERT_EQ(packet.ackId, expectedAck);
        static constexpr size_t AckBitsetSize = sizeof(GioNet::Packet::AckBitsetType);
        ASSERT_EQ(packet.ackBitset, expectedBitset) << "Expected ack header: " <<
            std::bitset<AckBitsetSize>{expectedBitset} << ", got ack header: " <<
                std::bitset<AckBitsetSize>{packet.ackBitset};
    }
};

TEST(ReliabilityTests, ack_bitset)
{
    AckBitsetTest bitsetTest{};
    bitsetTest.Test({ 1, 2, 3, 4, 5 }, 5, 0b1111);
    bitsetTest.Test({ 1, 2, 5 }, 5, 0b1100);
    bitsetTest.Test({ GioNet::Packet::MaxPossibleId, 1, 2 }, 2, 0b101);
    bitsetTest.Test({ GioNet::Packet::MaxPossibleId - 1, GioNet::Packet::MaxPossibleId, 1 }, 1, 0b110);
}

TEST(ReliabilityTests, ack_overflow_stress)
{
    GioNet::Connection sender{};
    GioNet::Connection receiver{};

    static constexpr size_t Iterations = GioNet::Packet::MaxPossibleId + 1;

    for (size_t i = 0; i < Iterations; ++i)
    {
        if(i == 65535)
        {
            int s{};
        }
            
        {
            GioNet::Buffer payload{};
            payload.Write(i);
            GioNet::Packet packet{GioNet::Packet::Types::Data, GioNet::Packet::Flags::Reliable, std::move(payload)};
            sender.Schedule(std::move(packet));
        }

        std::optional<GioNet::Packet> outgoingPacket = sender.PopReadyOutgoingPacket();
        ASSERT_TRUE(outgoingPacket);
        GioNet::Packet::IdType expectedId = outgoingPacket->id;

        {
            receiver.Received(std::move(*outgoingPacket));
            auto incomingPacket = receiver.PopReadyIncomingPacket();
            ASSERT_TRUE(incomingPacket);
            ASSERT_EQ(incomingPacket->id, expectedId);
            size_t payload = incomingPacket->payload.Read<size_t>();
            ASSERT_EQ(payload, i);
        }
    }
}

TEST(ReliabilityTests, non_acked_packets_resent)
{
    std::set<GioNet::Packet::IdType> missingPackets = {2, 4};
    GioNet::Connection sender{};
    GioNet::Connection receiver{};
 
    for (int i = 0; i < 5; ++i)
    {
        GioNet::Packet packet{GioNet::Packet::Types::Data, GioNet::Packet::Flags::Reliable};
        sender.Schedule(std::move(packet));
        std::optional<GioNet::Packet> sent = sender.PopReadyOutgoingPacket();
        ASSERT_TRUE(sent);

        if(!missingPackets.contains(sent->id))
        {
            receiver.Received(std::move(*sent));
        }
    }

    std::optional<GioNet::Packet> packetWithAcks{};
    {
        GioNet::Packet packet{GioNet::Packet::Types::Data, GioNet::Packet::Flags::Reliable};
        receiver.Schedule(std::move(packet));

        packetWithAcks = receiver.PopReadyOutgoingPacket();
    }
    ASSERT_TRUE(packetWithAcks);

    {
        sender.Received(std::move(*packetWithAcks));
        packetWithAcks.reset();

        while(std::optional<GioNet::Packet> outgoingPacket = sender.PopReadyOutgoingPacket())
        {
            if(missingPackets.contains(outgoingPacket->id))
            {
                missingPackets.erase(outgoingPacket->id);
            }
        }

        ASSERT_TRUE(missingPackets.empty()) << "Expected all missing packets to be rescheduled";
    }
}