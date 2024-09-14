#include "GioNet.h"
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
        TData after = buffer.ReadBytesAndConstruct<TData>();
        ASSERT_EQ(val, after);
        ASSERT_TRUE(buffer.IsEmpty());
    }

    void ReadTest(GioNet::Buffer& buffer, const TData& expectedVal)
    {
        TData after = buffer.ReadBytesAndConstruct<TData>();
        ASSERT_EQ(expectedVal, after);
    }
};

TEST(BufferSerializationTests, int)
{
    DataSerializationTest<int> test;
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

TEST(BufferSerializationTests, buffer)
{
    GioNet::Buffer data{"This is a c str!"};
    GioNet::Buffer b{};
    b.Write(data);
    GioNet::Buffer deserialized{b.ReadBytesAndConstruct<GioNet::Buffer>()};
    ASSERT_EQ(deserialized, data);
}