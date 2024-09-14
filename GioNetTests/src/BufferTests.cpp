#include "GioNet.h"
#include "gtest/gtest.h"

TEST(BufferConstruction, specialized_ctor)
{
    int64_t data{};
    GioNet::Buffer b{&data, sizeof(data)};
    ASSERT_EQ(b.Length(), sizeof(data));
}