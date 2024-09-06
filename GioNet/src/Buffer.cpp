#include "Buffer.h"

#include <cassert>

#include "Core.h"

GioNet::Buffer::Buffer(const char* data, int length)
{
    assert(length <= GIONET_BUFFER_MAX);
    payload = std::vector<char>{};
    payload.resize(length, 0);
    char* internalData = payload.data();
    memcpy(internalData, data, length);
}

GioNet::Buffer::Buffer(std::string_view sv)
    : Buffer(sv.data(), static_cast<int>(sv.length()) + 1)
{
}
