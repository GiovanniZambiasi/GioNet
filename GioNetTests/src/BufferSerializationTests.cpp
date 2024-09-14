#include "GioNet.h"
#include "Packet.h"
#include "gtest/gtest.h"

template<typename TData>
class DataSerializationTest
{
public:
    void ConstructAndTest(const TData val)
    {
        GioNet::Buffer b{};
        WriteReadTest(b, val);
    }

    void WriteReadTest(GioNet::Buffer& buffer, const TData val)
    {
        TData before = val;
        buffer.Write(before);
        TData after = buffer.Read<TData>();
        ASSERT_EQ(val, after);
        ASSERT_TRUE(buffer.IsEmpty());
    }

    void ReadTest(GioNet::Buffer& buffer, const TData& expectedVal)
    {
        TData after = buffer.Read<TData>();
        ASSERT_EQ(expectedVal, after);
    }
};

TEST(BufferSerializationTests, int)
{
    DataSerializationTest<int32_t> test;
    test.ConstructAndTest(42);
    test.ConstructAndTest(-1000);
    test.ConstructAndTest(0);
    test.ConstructAndTest(~0);
}

TEST(BufferSerializationTests, char)
{
    DataSerializationTest<char> test;
    test.ConstructAndTest('a');
    test.ConstructAndTest('\n');
    test.ConstructAndTest(0);
    test.ConstructAndTest(~0);
}

TEST(BufferSerializationTests, std_string)
{
    DataSerializationTest<std::string> test;
    test.ConstructAndTest("Foo");
    test.ConstructAndTest("");
    test.ConstructAndTest("Hello, world!\n");
}

TEST(BufferSerializationTests, ctor_string_view)
{
    DataSerializationTest<std::string> test{};
    GioNet::Buffer b{"This is a c str!"};
    test.ReadTest(b, "This is a c str!");

    std::string str{"This is an std::string!"};
    b = GioNet::Buffer{str};
    test.ReadTest(b, "This is an std::string!");
}

TEST(BufferSerializationTests, packet)
{
    DataSerializationTest<GioNet::Packet> test{};
    test.ConstructAndTest(GioNet::Packet{12, GioNet::Packet::Type::Ping, {}});
    test.ConstructAndTest(GioNet::Packet{static_cast<uint8_t>(~0), GioNet::Packet::Type::Data, {"Payload!"}});
    test.ConstructAndTest(GioNet::Packet{0, GioNet::Packet::Type::Ack, {"Other payload!"}});
}

TEST(BufferSerializationTests, buffer)
{
    GioNet::Buffer data{"This is a c str!"};
    GioNet::Buffer b{};
    b.Write(data);
    GioNet::Buffer deserialized{b.Read<GioNet::Buffer>()};
    ASSERT_EQ(deserialized, data);
}

TEST(BufferSerializationTests, subsequent_writes_same_type)
{
    GioNet::Buffer data{};
    data.Write(10);
    data.Write(999);
    data.Write(0);

    ASSERT_EQ(data.Read<int>(), 10);
    ASSERT_EQ(data.Read<int>(), 999);
    ASSERT_EQ(data.Read<int>(), 0);
}

TEST(BufferSerializationTests, subsequent_writes_different_types)
{
    GioNet::Buffer data{};
    data.Write(10);
    std::string s{"Hello, world!"};
    data.Write(s);
    data.Write('t');

    ASSERT_EQ(data.Read<int>(), 10);
    ASSERT_EQ(data.Read<std::string>(), s);
    ASSERT_EQ(data.Read<char>(), 't');
}