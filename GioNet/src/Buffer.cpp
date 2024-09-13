#include "Buffer.h"
#include <cassert>
#include "Core.h"

GioNet::Buffer::Buffer(const char* data, int length)
{
    assert(length <= GIONET_BUFFER_MAX);
    payload = std::vector<char>{};
    CopyBytesIntoPayload(data, length);
}

GioNet::Buffer::Buffer(std::string_view view)
{
    Write(view);
}

void GioNet::Buffer::CopyBytesIntoPayload(const void* src, size_t size)
{
    if(size == 0)
        return;
    
    size_t beforeResize = payload.size();
    payload.resize(payload.size() + size, 0);
    char* dataHead = payload.data() + beforeResize;
    memcpy(dataHead, src, size);
}

template<>
void GioNet::Buffer::Write(const int& val)
{
    WriteBytes(val);
}

template<>
void GioNet::Buffer::Write(const char& val)
{
    WriteBytes(val);
}

template<>
void GioNet::Buffer::Write(const std::string& val)
{
    int length = static_cast<int>(val.length());
    Write(length);
    // Not copying termination char because it will be added implicitly by std::string on deserialize
    CopyBytesIntoPayload(val.data(), val.length()); 
}

template<>
std::string GioNet::Buffer::ReadBytesAndConstruct()
{
    int length = ReadBytesAndConstruct<int>();
    std::string result{};

    if(length > 0)
    {
        result.resize(length, 0);
        memcpy(result.data(), payload.data(), length);
    }
    
    return result;
}

template<>
void GioNet::Buffer::Write(const std::string_view& val)
{
    int length = static_cast<int>(val.length());
    Write(length);
    // Not copying termination char because it will be added implicitly by std::string on deserialize
    CopyBytesIntoPayload(val.data(), val.length()); 
}
