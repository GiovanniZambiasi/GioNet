#include "GioNet.h"
#include "gtest/gtest.h"

TEST(Buffer, specialized_ctor)
{
    int64_t data{};
    GioNet::Buffer b{&data, sizeof(data)};
    ASSERT_EQ(b.Length(), sizeof(data));
}

TEST(Buffer, copy)
{
    GioNet::Buffer source{"Wow, so much data!"};
    GioNet::Buffer destination{};
    destination.Copy(source);
    ASSERT_EQ(destination, source);
}